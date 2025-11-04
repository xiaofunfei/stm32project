#include "TipsTask.h"


void tipsTask(void * arg) {
	// 绿灯-> 复位灯
	// 黄灯-> 等待灯
	// 红灯-> 异常发生灯
	
	
    int greenFlag = -1;
	  int yellowFlag = -1;
	  int redFlag = -1;
		int toneFlag = -1;

    while (1) {
        int tips = -1;
        xQueueReceive(tipsQueue, &tips,  1000);
			
				if(tips == NETWORK_CONNECTING){
					 yellowFlag = NETWORK_CONNECTING;
				}else if(tips == NETWORK_CONNECTING_END){
					yellowFlag = NETWORK_CONNECTING_END;
				}else if(tips == WARNING_RESET){
					toneFlag = MY_TRUE;
					greenFlag = MY_TRUE;
				}
			
				
				if(yellowFlag == NETWORK_CONNECTING){
						changeLight(GPIO_Pin_7);
				}else if(yellowFlag == NETWORK_CONNECTING_END){
						closeLight(GPIO_Pin_7);
				}
				
				
				if(greenFlag == MY_TRUE){
					openLight(GPIO_Pin_5);
				}

				
				if(toneFlag == MY_TRUE){
					openWarningTone(GPIO_Pin_15);
				}
    }
}
