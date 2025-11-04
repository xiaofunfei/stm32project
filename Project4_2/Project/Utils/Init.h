#ifndef __INIT_H__
#define __INIT_H__
#include "Config.h"  

// 项目初始化文件

extern QueueHandle_t queueBlueTooth; // 字符串传输队列
extern QueueHandle_t queueWIFI; // 字符串传输队列
extern QueueHandle_t tipsQueue;
extern QueueHandle_t sensorQueue1;
extern QueueHandle_t sensorQueue2;

void Init(void);

#endif
