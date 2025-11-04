#ifndef __CONFIG_H__
#define __CONFIG_H__
#include "stm32f10x.h" 

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h" 
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "Delay.h"
#include "PC.h"
#include "WIFI.h"
#include "BlueTooth.h"
#include "Flash.h"
#include "Init.h"
#include "Sensor.h"
#include "OLED.h"
#include "Light.h"
#include "WarningTone.h"
#include "Reset.h"
#include "Utils.h"
#include "Dog.h"
#include "W25.h"
#include "TipsTask.h"
#include "MainTask.h"
#include "WatchNetTask.h"
#include "WriteW25Task.h"
#include "TCPTask.h"

#define  MAX_AT_WAIT_TIMES_COUNT		30000 // AT指令最大的等待时钟节拍
#define  TRY_AT_COMMAND_AGAIN_TIMES	30000 // AT指令失败后, 再次进行重试的时钟节拍间隔
#define  SENSOR_TIMES	10000 // 多久获取一次传感器数据

#define  FLASH_ADDRESS 							0x0800F000  // Flash存储账号密码地址
#define  FLASH_STRING_BUF_LEN 			40 //  账号密码地址大小
#define  NET_CONNECT_TIMES 					3	 //  网络重连次数

#define  W25_END_ADDRESS 							0x800000  // W25Q64的地址

#define  AT 												"AT\r\n"
#define  AT_CWJAP_DEF 							"AT+CWJAP_DEF?\r\n"
#define  AT_SWJAP  									"AT+CWJAP_DEF=\"%s\",\"%s\"\r\n"
#define  AT_CIPSTART  							"AT+CIPSTART=\"TCP\",\"47.115.220.165\",9000\r\n"
#define  AT_CIPMODE1  							"AT+CIPMODE=1\r\n"
#define  AT_CIPSEND  								"AT+CIPSEND\r\n"
#define  AT_EXIT  									"+++"
#define  AT_CIPSEND  								"AT+CIPSEND\r\n"
#define  AT_CIPMODE0  							"AT+CIPMODE=0\r\n"
#define  AT_CIPCLOSE  							"AT+CIPCLOSE\r\n"

typedef enum {
		NETWORK_TIMEOUT,     			// 0 - 网络操作超时
    NETWORK_DISCONNECTED,     // 0 - 未连接
	  NETWORK_CONNECTED,        // 2 - 已连接
    NETWORK_CONNECTING,       // 1 - 网络操作进行中
		NETWORK_CONNECTING_END,      // 1 - 网络操作结束
	
		WARNING_RESET, 						//  - 异常复位
		NORMAL_RESET, 						//  - 普通复位
	
	  MY_TRUE,
	  MY_FALSE,
	
    NETWORK_DISCONNECTING,    // 3 - 断开中
    NETWORK_ERROR,            // 4 - 出现错误
    NETWORK_LIMITED,          // 6 - 受限连接（如仅本地访问）
    NETWORK_UNKNOWN           // 7 - 未知状态
} Status;




#endif
