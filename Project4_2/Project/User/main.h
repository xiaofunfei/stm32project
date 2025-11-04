#ifndef __MAIN_H__
#define __MAIN_H__
#include "Config.h"  

QueueHandle_t queueBlueTooth; // 字符串传输队列
QueueHandle_t queueWIFI; // 字符串传输队列
QueueHandle_t tipsQueue; // 通知程序运行状态的队列
QueueHandle_t sensorQueue1; // 传感器数据传递
QueueHandle_t sensorQueue2; // 传感器数据传递

#endif
