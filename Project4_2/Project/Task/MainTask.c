#include "MainTask.h"


void testTCP(){
	printf1("testTCP \r\n");
	// 测试是否退出了数据收发模式
	atCommandCheck();
	
	printf1("testTCP  tcpCommand \r\n");
	// 测试是能建立TCP连接
	int netStatus = tcpCommand();
	statusCheck(netStatus, "getTCPStatus");
	
	if(netStatus == MY_TRUE){
		printf1("testTCP  ok \r\n");
		// TCP连接正常
		// 让tcpTask检查W25Q64 上传阿里云
		xTaskCreate(tcpTask, "tcpTask", configMINIMAL_STACK_SIZE*8, NULL, 3, NULL); 
		// 获取当前数据, 交给tcpTask上传aliyun
		
		while(1){
			
        // 读取传感器数据
       	uint16_t AD1 = AD_GetValue(ADC_Channel_1);		
				uint16_t AD4 = AD_GetValue(ADC_Channel_4);		
				xQueueSend(sensorQueue1, &AD1, 1000);
				xQueueSend(sensorQueue2, &AD4, 1000);
			
				vTaskDelay(SENSOR_TIMES);
		}
	
	}else{
		printf1("testTCP error \r\n");
		// TCP连接失败->启动一个监控网络和TCP的任务; 
		xTaskCreate(watchNetTask, "watchNetTask", configMINIMAL_STACK_SIZE*2, NULL, 3, NULL); 
		
		// 获取数据存储W25Q64: 先获取当前W25Q64中的数据已经存储到哪里了
		int w25AddressEnd = 0;
		getEndADDRESS(&w25AddressEnd);
		
		saveSensorToW25(w25AddressEnd);
	}
}

void mainTask(void * arg) {
	
	// 判断网络连接状态
	int netStatus = getLinkedStatus();
	statusCheck(netStatus, "getLinkedStatus");
	if(netStatus == NETWORK_CONNECTED){
		printf1("MainTask: 15: %s \r\n", "wifiok");
		// 已连上网络
		testTCP();
		
	}else if(netStatus == NETWORK_DISCONNECTED){
		// 没有连上网
		printf1("MainTask: 15: %s \r\n", "wifi_error");
		
		// 没有连上网, 开启任务额外任务存储W25Q64
		xTaskCreate(WriteW25Task, "WriteW25Task", configMINIMAL_STACK_SIZE*2, NULL, 3, NULL); 
		
		
		// 获取Flash是否存储有账号密码
		int flashStatus = getFlashStatus();
		
		char buffer[FLASH_STRING_BUF_LEN] = {0};
		if (flashStatus != 0) { 
				// Flash无账号密码, 获取蓝牙
        readBlueTooth(buffer);
		}else{
				// Falsh有账号密码, 读取 
        readFlash(buffer);
		}
		printf1("MainTask: 32: %s \r\n", buffer);
		
		int times = NET_CONNECT_TIMES;
		while(times--){ // 联网重试
			printf1("MainTask: 36: %d \r\n", times);
			// 重新联网						
			netStatus =	wifiCommand(buffer);
			if(netStatus == MY_TRUE){
				break;
			}
			
			vTaskDelay(TRY_AT_COMMAND_AGAIN_TIMES);// AT指令失败后, 再次进行重试的时钟节拍间隔
		}
		
		printf1("MainTask: 43: %d \r\n", netStatus);
		if (flashStatus != 0 && netStatus == MY_TRUE) { 
			// 蓝牙:联网成功 -> 记录Flash, 复位
			// 记录Flash
			setFlash(buffer);
			messageToApp("{\"status\":0, \"wifi_name\": \"net\"}");
			
			statusCheck(NORMAL_RESET,  "reset");
		}else if(netStatus == MY_TRUE){
			// 读取Flash联网成功
			statusCheck(NORMAL_RESET,  "reset");
		}else if(netStatus == MY_FALSE ){
		  // 联网失败 -> 警告提示, 复位
			statusCheck(NETWORK_TIMEOUT,  "reset");
		}
	}
}
