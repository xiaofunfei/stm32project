#include "TipsTask.h"


void watchNetTask(void * arg) {

    while (1) {
			int netStatus1 = getLinkedStatus();
			statusCheck(netStatus1, "getLinkedStatus");
			
			int netStatus2 = tcpCommand();
			statusCheck(netStatus2, "getLinkedStatus");
			
			if(netStatus1 == NETWORK_CONNECTED && netStatus2 == MY_TRUE){
				// 网络状态变好, 复位重启
				statusCheck(NORMAL_RESET,  "reset");
			}
			
			vTaskDelay(5*600000); // 每间隔5分钟, 重新测试一下网络连接
    }
}
