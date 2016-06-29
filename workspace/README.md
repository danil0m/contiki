Workspace STM32Nucleo-Spirit1
============

In this directory there are some example for stm32nucleo-spirit1 platform, including rpl-border-router, er-rest-example, sensor-er-rest-example

It needs STM32NUCLEO-L152RE (MCU) board with the X-NUCLEO-IDS01Ax 
(sub-1GHz RF communication) (and X-NUCLEO-IKS01A1 for sensor-er-rest-example) expansion board(s).

To initialize stm32nucleo platform, you need to execute the following commands:

		git checkout stm32nucleo-spirit1
		git submodule init
		git submodule update
		


To build an example on the terminal, enter the directory and type: 

	make 
	
To clean one directory project type, being in that directory:

	make clean

To build in eclipse, right-click on the project directory -> Make Targets -> Create. Than create  "all" and "clean" (for cleaning directory, optional) targets.
Then right-click on the same project directory -> Make Targets -> Build (Shift+F9), select all and than click on Build. 

In the Makefile TARGET and BOARD (for the radio) and SENSORBOARD in the case of sensor example are already specified. Check BOARD variable in the Makefile. It depends on the X-NUCLEO-IDS01Ax expansion board for sub GHz radio connectivity you have.
