#ifndef __W25_H__
#define __W25_H__
#include "Config.h"  

// 工具类
void SPI1_Init(void);
uint8_t W25Q64_Read_Byte(uint32_t ReadAddr);
void W25Q64_Erase_Sector(uint32_t SectorAddr);

void getEndADDRESS(int * w25AddressEnd);

void saveSensorToW25(int  w25AddressEnd);

#endif
