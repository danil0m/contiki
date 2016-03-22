#include "contiki.h"
#include "dev/eeprom.h"

#include "stm32l1xx.h"
#include "stm32l1xx_hal_rcc.h"
#include "stm32l1xx_hal_cortex.h"
#include "stm32l1xx_hal.h"
#include "st-lib.h"

unsigned char eeprom[EEPROM_SIZE] __attribute__((section(".eeprom")));


void eeprom_write(eeprom_addr_t addr, unsigned char *buf, int size){

	int i;
	HAL_FLASHEx_DATAEEPROM_Unlock();
	  for(i=0;i< size;i++){
		  if(addr+i<EEPROM_SIZE){
			  HAL_FLASHEx_DATAEEPROM_Program(TYPEPROGRAMDATA_FASTBYTE, eeprom+addr+i, *((uint8_t*)(buf+i)));

		  }else
			  break;
	  }
	  /*HAL_FLASHEx_DATAEEPROM_Program(TYPEPROGRAMDATA_FASTWORD, eeprom+addr+i, 0xDEADBEEF);*/


	  HAL_FLASHEx_DATAEEPROM_Lock();


}

void eeprom_read(eeprom_addr_t addr, unsigned char *buf, int size){

	int i;
	for(i=0;i<size;i++){
		if(addr+i<EEPROM_SIZE)
			buf[i]= eeprom[addr+i];
		else
			break;
	}


}


void eeprom_init(void){


}
