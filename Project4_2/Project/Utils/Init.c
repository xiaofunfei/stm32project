#include "Init.h"

void Init(void) {
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    Delay_Init();
    PC_Init();
    WIFI_Init();
    BlueTooth_Init();
    Sensor_Init();
    OLED_Init();
    Light_Init();
    WarningTone_Init();
		Reset_Init();
		SPI1_Init();

    queueBlueTooth = xQueueCreate(100, 1);
		queueWIFI = xQueueCreate(100, 1);
		tipsQueue = xQueueCreate(10, 4);
		sensorQueue1 = xQueueCreate(10, 2);
		sensorQueue2 = xQueueCreate(10, 2);
	
		Delay_ms(5000);// 等待,等待各模块上电,上电之后需要一定时间进入稳定状态
	
		BlueTooth_NVIC();
		WIFI_NVIC();
		
		startTone(); // 启动声音提示
		openLight(GPIO_Pin_5);// 复位启动/开始工作提示灯
		Delay_ms(1000);
		closeLight(GPIO_Pin_5);
}
