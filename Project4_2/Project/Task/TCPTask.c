#include "TCPTask.h"


void tcpTask(void * arg) {
		// 检查W25Q64 上传阿里云
		int w25AddressEnd = 0;
		getEndADDRESS(&w25AddressEnd);
		printf1("tcpTask: endAddress: %d \r\n", w25AddressEnd);
	
		int netStatus = cipCommand();
		statusCheck(netStatus, "getTCPStatus");
	
    netStatus = beginSendCommand();
		statusCheck(netStatus, "getTCPStatus");
	
    while (1) {
			// 接收新的传感器数据上传阿里云
			char temp[50] = {0};
			uint16_t ad1 = 0,  ad2 = 0;
			BaseType_t ret1 = xQueueReceive(sensorQueue1, &ad1,  1000);
			BaseType_t ret2 = xQueueReceive(sensorQueue2, &ad2,  1000);
			if(ret1 != pdFAIL && ret2 != pdFAIL){
				sprintf(temp, "=%s:%u,%s:%u=","1", ad1, "2", ad2);
				// 5+4*2+1*2 = 15
				
				// 上传至阿里云
				char http[180] = {0};
				sprintf(http, "POST /x HTTP/1.1\r\n\r\n%s", temp);
				messageUpload(http);
				
				memset(http, 0, sizeof(http));
				TickType_t beginTicks = xTaskGetTickCount(); // 获得当前时钟节拍
				// 回收阿里云的响应报文
				 while (1) {
						char ch = 0;
						BaseType_t ret = xQueueReceive(queueWIFI, &ch,  1000);
						if (ret == pdPASS) { // 读到AT指令返回信息, 存储
								http[strlen(http)] = ch;
							  printf1("tcpTask: 39: %s \r\n", http);
						}
						
						if (endsWithStr(http, "OK")) {
								break;
						} 

						if (5000 + beginTicks < xTaskGetTickCount()) {
							// 服务端数据响应超时: 退出数据收发模式, 关闭透传, 关闭tcp连接,  重启
							exitSendCommand();
							closeCipCommand();
							closeTCPCommand();

							statusCheck(NETWORK_TIMEOUT, "closeConnectStatus");	
						}
				}
			}
			printf1("tcpTask: temp: %s \r\n", temp);
			
			// 判断w25q64中是否有数据
			if(w25AddressEnd > 0 ){
				// w25q64中缓存有数据,  读数据
				int  erase_Sector_Address = 0;
				
				// 读取数据: 从后向前读-> 先上传后面的, 再上传前面的
				// 每次读取10条数据, 准备上传阿里云
				char buf[160] = {0}; 
				int times_tag = 0;
				
				int i = w25AddressEnd-1;
				for (int j=0; i > w25AddressEnd-150; i--, j++) {
					if(i%4096 == 0){
						erase_Sector_Address = i;
					}
					buf[j] = W25Q64_Read_Byte(i);
					if(buf[j] == '='){
						times_tag++;
					}
					if(times_tag == 20){
						 break;// 读10组
					}
				}
				char buf2[150] = {0};
				for(int k=0; k<strlen(buf); k++){
					buf2[k] = buf[strlen(buf)-k-1];
				}
				
				// 上传阿里云
				printf1("tcpTask: buf: %s \r\n", buf2);
				
				char http[180] = {0};
				sprintf(http, "POST /y HTTP/1.1\r\n\r\n%s", buf2);
				printf1("tcpTask: http: %s \r\n", http);
				messageUpload(http);
				
				memset(http, 0, sizeof(http));
				TickType_t beginTicks = xTaskGetTickCount(); // 获得当前时钟节拍
				// 回收阿里云的响应报文
				 while (1) {
						char ch = 0;
						BaseType_t ret = xQueueReceive(queueWIFI, &ch,  1000);
						if (ret == pdPASS) { // 读到AT指令返回信息, 存储
								http[strlen(http)] = ch;
							  printf1("tcpTask: 39: %s \r\n", http);
						}
						
						if (endsWithStr(http, "OK")) {
								break;
						} 

						if (5000 + beginTicks < xTaskGetTickCount()) {
							// 服务端数据响应超时: 退出数据收发模式, 关闭透传, 关闭tcp连接,  重启
							exitSendCommand();
							closeCipCommand();
							closeTCPCommand();

							statusCheck(NETWORK_TIMEOUT, "closeConnectStatus");	
						}
				}
				

				w25AddressEnd = i;
				printf1("tcpTask: firstAddress: %d \r\n", w25AddressEnd);

				if( erase_Sector_Address != 0 ||  w25AddressEnd == 0){
					// 擦除: 这个删除扇区
					W25Q64_Erase_Sector(erase_Sector_Address);
					printf1("tcpTask: W25Q64_Erase_Sector :%d \r\n", erase_Sector_Address);
				} 
			}
    }
}
