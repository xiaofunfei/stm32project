#include "WIFI.h"

void WIFI_Init(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    // 对应USART2->Tx, 即数据发送引脚
    GPIO_InitTypeDef initType;
    initType.GPIO_Mode = GPIO_Mode_AF_PP;
    initType.GPIO_Pin = GPIO_Pin_2;
    initType.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &initType);

    // 对应USART2->Rx, 即数据接收引脚
    initType.GPIO_Mode = GPIO_Mode_IPU;
    initType.GPIO_Pin = GPIO_Pin_3;
    initType.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &initType);

    // USART配置
    USART_InitTypeDef initUSART;
    initUSART.USART_BaudRate = 115200;
    initUSART.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    initUSART.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    initUSART.USART_Parity = USART_Parity_No;
    initUSART.USART_StopBits = USART_StopBits_1;
    initUSART.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART2, &initUSART);


    // 让串口开始工作运行
    USART_Cmd(USART2, ENABLE);
}

void WIFI_NVIC(void) {
    // 配置开启USART接收数据的中断
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

    // 配置中断和优先级
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 12;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_InitStructure);

}

void wifiSendByte(uint8_t byte) {
    USART_SendData(USART2, byte);

    while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
}

void wifiSendAtCommand(char *strs) {
    for (uint8_t i = 0; strs[i] != '\0'; i++) {
        wifiSendByte(strs[i]);
    }
}

void messageUpload(char *strs) {

    for (uint8_t i = 0; strs[i] != '\0'; i++) {
        wifiSendByte(strs[i]);
    }
}

void USART2_IRQHandler(void) {
    if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET) {
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
        char data = USART_ReceiveData(USART2);

        // 接收ESP01S数据回传, 存入消息队列
        BaseType_t stat = pdTRUE;
        xQueueSendFromISR(queueWIFI, &data, &stat);
    }
}

int atCommandCheck(void) {	
	 xQueueReset(queueWIFI);

    TickType_t beginTicks = xTaskGetTickCount(); // 获得当前时钟节拍
    wifiSendAtCommand("AT\r\n"); // 发送命令
    
    char buf[50] = {0};
    int size = 0;

    while (1) {
        char ch = 0;
        BaseType_t ret = xQueueReceive(queueWIFI, &ch,  1000);
        if (ret == pdPASS) { // 读到AT指令返回信息, 存储
            buf[size] = ch;
            size++;
        }else{
					if (endsWithStr(buf, "OK") || endsWithStr(buf, "OK\r\n")) {
							return 0;
					} else{
						vTaskDelay(1000);
						wifiSendAtCommand("+++");
						vTaskDelay(1000);
						break;
					}
				}
    }
		return 0;
}

// 获取此刻ESP01S的网络连接状态: 0有链接,-1无连接
int getLinkedStatus(void) {

    TickType_t beginTicks = xTaskGetTickCount(); // 获得当前时钟节拍
    wifiSendAtCommand(AT_CWJAP_DEF); // 发送命令
		statusCheck(NETWORK_CONNECTING, "closeConnectStatus");
	
    char buf[150] = {0};
    int size = 0;

    while (1) {
        char ch = 0;
        BaseType_t ret = xQueueReceive(queueWIFI, &ch,  1000);

        if (ret == pdPASS) { // 读到AT指令返回信息, 存储
            buf[size] = ch;
            size++;
        }

        // 情况1
        //AT+CWJAP_DEF?
        //No AP

        //OK

        // 情况2
        //AT+CWJAP_DEF?
        //+CWJAP_DEF:"snow","ea:02:96:74:89:99",1,-36,0

        //OK

        if (hasStr(buf, "No AP")) {
						// 关闭灯提示
						
						statusCheck(NETWORK_CONNECTING_END, "closeConnectStatus");
            return NETWORK_DISCONNECTED;
        } else if (hasStr(buf, "+CWJAP_DEF:\"")) {
					// 关闭灯提示
						statusCheck(NETWORK_CONNECTING_END, "closeConnectStatus");
            return NETWORK_CONNECTED;
        }

        if (MAX_AT_WAIT_TIMES_COUNT + beginTicks < xTaskGetTickCount()) {
						// 关闭灯提示
						statusCheck(NETWORK_CONNECTING_END, "closeConnectStatus");
            return NETWORK_TIMEOUT;// 等待返回超时
        }
    }
}


int wifiCommand(char * buffer) {

    char bufName[20] = {0};
    char bufPassword[20] = {0};
    int index = 0;

		// !账号=密码!
    // 切割出用户名和密码
    // 1~=
    for (int i = index + 1; i < FLASH_STRING_BUF_LEN; i++) {
        if (buffer[i] == '=') {
            index = i;
            break;
        }
        bufName[i - 1] = buffer[i];
    }

    // =~!
    for (int i = index + 1; i < FLASH_STRING_BUF_LEN; i++) {
        if (buffer[i] == '!') {
            break;
        }
        bufPassword[i - 1 - index] = buffer[i];
    }

    char at[50] = {0};
    sprintf(at, AT_SWJAP, bufName, bufPassword);
		
		 // 清空消息队列
    xQueueReset(queueWIFI);
		printf1("WIFI: 189: %s \r\n", at);
    TickType_t beginTicks = xTaskGetTickCount(); // 获得当前时钟节拍
    wifiSendAtCommand(at); // 发送命令
		statusCheck(NETWORK_CONNECTING, "closeConnectStatus");
    
    char buf[150] = {0};
    int size = 0;

    while (1) {
        char ch = 0;
        BaseType_t ret = xQueueReceive(queueWIFI, &ch,  1000);
        if (ret == pdPASS) { // 读到AT指令返回信息, 存储
            buf[size] = ch;
            size++;
					printf1("WIFI: 203: %s \r\n", buf);
        }
				
				if (endsWithStr(buf, "OK")) {
						statusCheck(NETWORK_CONNECTING_END, "closeConnectStatus");
            return MY_TRUE;
        } else if (endsWithStr(buf, "FAIL") || endsWithStr(buf, "ERROR") || endsWithStr(buf, "busy p")) {
            statusCheck(NETWORK_CONNECTING_END, "closeConnectStatus");
            return MY_FALSE;
        } 

        if (MAX_AT_WAIT_TIMES_COUNT + beginTicks < xTaskGetTickCount()) {
            statusCheck(NETWORK_CONNECTING_END, "closeConnectStatus");
            return MY_FALSE;// 等待返回超时
        }
		}
}

int tcpCommand(void) {
		printf1("WIFI: 222: tcpCommand \r\n");
	  xQueueReset(queueWIFI);
		
    TickType_t beginTicks = xTaskGetTickCount(); // 获得当前时钟节拍
    wifiSendAtCommand(AT_CIPSTART); // 发送命令
		statusCheck(NETWORK_CONNECTING, "closeConnectStatus");
    
    char buf[150] = {0};
    int size = 0;

    while (1) {
        char ch = 0;
        BaseType_t ret = xQueueReceive(queueWIFI, &ch,  1000);
        if (ret == pdPASS) { // 读到AT指令返回信息, 存储
            buf[size] = ch;
            size++;
					printf1("WIFI: 238: %s \r\n", buf);
        }
				
				if (endsWithStr(buf, "OK")) {
						statusCheck(NETWORK_CONNECTING_END, "closeConnectStatus");
            return MY_TRUE;
        } else if (endsWithStr(buf, "FAIL") || endsWithStr(buf, "ERROR") || endsWithStr(buf, "busy p")) {
            statusCheck(NETWORK_CONNECTING_END, "closeConnectStatus");
            return MY_FALSE;
        } 

        if (MAX_AT_WAIT_TIMES_COUNT + beginTicks < xTaskGetTickCount()) {
            statusCheck(NETWORK_CONNECTING_END, "closeConnectStatus");
            return NETWORK_TIMEOUT;// 等待返回超时
        }
		}
}


int cipCommand(void) {

		printf1("WIFI: 259: cipCommand \r\n");
	  xQueueReset(queueWIFI);
		
    TickType_t beginTicks = xTaskGetTickCount(); // 获得当前时钟节拍
    wifiSendAtCommand(AT_CIPMODE1); // 发送命令
		statusCheck(NETWORK_CONNECTING, "closeConnectStatus");
    
    char buf[150] = {0};
    int size = 0;

    while (1) {
        char ch = 0;
        BaseType_t ret = xQueueReceive(queueWIFI, &ch,  1000);
        if (ret == pdPASS) { // 读到AT指令返回信息, 存储
            buf[size] = ch;
            size++;
					printf1("WIFI: 275: %s \r\n", buf);
        }
				
				if (endsWithStr(buf, "OK")) {
						statusCheck(NETWORK_CONNECTING_END, "closeConnectStatus");
            return MY_TRUE;
        } else if (endsWithStr(buf, "FAIL") || endsWithStr(buf, "ERROR") || endsWithStr(buf, "busy p")) {
            statusCheck(NETWORK_CONNECTING_END, "closeConnectStatus");
            return MY_FALSE;
        } 

        if (MAX_AT_WAIT_TIMES_COUNT + beginTicks < xTaskGetTickCount()) {
            statusCheck(NETWORK_CONNECTING_END, "closeConnectStatus");
            return NETWORK_TIMEOUT;// 等待返回超时
        }
		}	
}


int beginSendCommand(void) {

		printf1("WIFI: 296: beginSendCommand \r\n");
	  xQueueReset(queueWIFI);
		
    TickType_t beginTicks = xTaskGetTickCount(); // 获得当前时钟节拍
    wifiSendAtCommand(AT_CIPSEND); // 发送命令
		statusCheck(NETWORK_CONNECTING, "closeConnectStatus");
    
    char buf[150] = {0};
    int size = 0;

    while (1) {
        char ch = 0;
        BaseType_t ret = xQueueReceive(queueWIFI, &ch,  1000);
        if (ret == pdPASS) { // 读到AT指令返回信息, 存储
            buf[size] = ch;
            size++;
					printf1("WIFI: 312: %s \r\n", buf);
        }
				
				if (endsWithStr(buf, "OK")) {
						statusCheck(NETWORK_CONNECTING_END, "closeConnectStatus");
            return MY_TRUE;
        } else if (endsWithStr(buf, "FAIL") || endsWithStr(buf, "ERROR") || endsWithStr(buf, "busy p")) {
            statusCheck(NETWORK_CONNECTING_END, "closeConnectStatus");
            return MY_FALSE;
        } 

        if (MAX_AT_WAIT_TIMES_COUNT + beginTicks < xTaskGetTickCount()) {
            statusCheck(NETWORK_CONNECTING_END, "closeConnectStatus");
            return NETWORK_TIMEOUT;// 等待返回超时
        }
		}	
}

int exitSendCommand(void) {
    vTaskDelay(1000);
    wifiSendAtCommand("+++");
    vTaskDelay(1000);
    return 0;
}

int closeCipCommand(void) {
	
	
		printf1("WIFI: 340: beginSendCommand \r\n");
	  xQueueReset(queueWIFI);
		
    TickType_t beginTicks = xTaskGetTickCount(); // 获得当前时钟节拍
    wifiSendAtCommand(AT_CIPMODE0); // 发送命令
		statusCheck(NETWORK_CONNECTING, "closeConnectStatus");
    
    char buf[150] = {0};
    int size = 0;

    while (1) {
        char ch = 0;
        BaseType_t ret = xQueueReceive(queueWIFI, &ch,  1000);
        if (ret == pdPASS) { // 读到AT指令返回信息, 存储
            buf[size] = ch;
            size++;
					printf1("WIFI: 356: %s \r\n", buf);
        }
				
				if (endsWithStr(buf, "OK")) {
						statusCheck(NETWORK_CONNECTING_END, "closeConnectStatus");
            return MY_TRUE;
        } else if (endsWithStr(buf, "FAIL") || endsWithStr(buf, "ERROR") || endsWithStr(buf, "busy p")) {
            statusCheck(NETWORK_CONNECTING_END, "closeConnectStatus");
            return MY_FALSE;
        } 

        if (MAX_AT_WAIT_TIMES_COUNT + beginTicks < xTaskGetTickCount()) {
            statusCheck(NETWORK_CONNECTING_END, "closeConnectStatus");
            return NETWORK_TIMEOUT;// 等待返回超时
        }
		}	
}

int closeTCPCommand(void) {

		printf1("WIFI: 376: beginSendCommand \r\n");
	  xQueueReset(queueWIFI);
		
    TickType_t beginTicks = xTaskGetTickCount(); // 获得当前时钟节拍
    wifiSendAtCommand(AT_CIPCLOSE); // 发送命令
		statusCheck(NETWORK_CONNECTING, "closeConnectStatus");
    
    char buf[150] = {0};
    int size = 0;

    while (1) {
        char ch = 0;
        BaseType_t ret = xQueueReceive(queueWIFI, &ch,  1000);
        if (ret == pdPASS) { // 读到AT指令返回信息, 存储
            buf[size] = ch;
            size++;
					printf1("WIFI: 392: %s \r\n", buf);
        }
				
				if (endsWithStr(buf, "OK")) {
						statusCheck(NETWORK_CONNECTING_END, "closeConnectStatus");
            return MY_TRUE;
        } else if (endsWithStr(buf, "FAIL") || endsWithStr(buf, "ERROR") || endsWithStr(buf, "busy p")) {
            statusCheck(NETWORK_CONNECTING_END, "closeConnectStatus");
            return MY_FALSE;
        } 

        if (MAX_AT_WAIT_TIMES_COUNT + beginTicks < xTaskGetTickCount()) {
            statusCheck(NETWORK_CONNECTING_END, "closeConnectStatus");
            return NETWORK_TIMEOUT;// 等待返回超时
        }
		}	
}



