CONTIKI_PROJECT = low-power-trials
TARGET=stm32nucleo-spirit1
BOARD=ids01a4
all:
	make after
	arm-none-eabi-size $(CONTIKI_PROJECT).stm32nucleo-spirit1
	arm-none-eabi-objcopy -O binary $(CONTIKI_PROJECT).stm32nucleo-spirit1 $(CONTIKI_PROJECT).bin

after: $(CONTIKI_PROJECT) 
CONTIKI = ../..
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

include $(CONTIKI)/Makefile.include
