#include "WriteW25Task.h"


void WriteW25Task(void * arg) {

    int w25AddressEnd = 0;
		getEndADDRESS(&w25AddressEnd);
		saveSensorToW25(w25AddressEnd);
}
