#include "main.h"


int main(void) {
    Init();
	
//	  startTone();
//		OLED_ShowString(1,1, "zs");
//		printf1("init \r\n");
//		while(1){
//		}
	
		printf1("main \r\n");
    xTaskCreate(mainTask, "mainTask", configMINIMAL_STACK_SIZE * 4, NULL, 3, NULL); 
    xTaskCreate(tipsTask, "tipsTask", configMINIMAL_STACK_SIZE*2, NULL, 3, NULL); 
    vTaskStartScheduler();
}




