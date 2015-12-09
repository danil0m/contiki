CONTIKI_PROJECT = hello_world
TARGET=stm32nucleo-spirit1
BOARD=ids01a4
all:
	ls
	make after
	arm-none-eabi-objcopy -O binary $(CONTIKI_PROJECT).stm32nucleo-spirit1 $(CONTIKI_PROJECT).bin

after: $(CONTIKI_PROJECT) 
CONTIKI = ../..
include $(CONTIKI)/Makefile.include
