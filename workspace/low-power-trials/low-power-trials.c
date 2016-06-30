/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         A very simple Contiki application showing how Contiki programs look
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "stm32l1xx_hal.h"
#include "stm32l1xx_nucleo.h"
#include "rtc.h"
#include "cfs.h"
#include "low-power.h"
#include "dev/button-sensor.h"
#include <stdio.h> /* For printf() */
#include <stdlib.h> /*for malloc and free*/
/*---------------------------------------------------------------------------*/
Alarm_Typedef_t array_in[3];
Alarm_Typedef_t array_out[3];
extern process_event_t sensors_event;
PROCESS(low_power_trials, "Low power trials");
AUTOSTART_PROCESSES(&low_power_trials);
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(low_power_trials, ev, data)
{
  PROCESS_BEGIN();

  int i,status;
  int *intheap[10];
  volatile Time_Typedef_t  time2;
  SENSORS_ACTIVATE(button_sensor);
  //printf("Hello, world\r\n");
  Alarm_Typedef_t vector[3];
  vector[0].hour=8;
  vector[0].minute=11;
  vector[0].second=10;

  vector[1].hour=15;
    vector[1].minute=33;
    vector[1].second=20;

    vector[2].hour=21;
      vector[2].minute=45;
      vector[2].second=00;

      /*HAL_Delay(5000);*/

//      printf("Standby flag before: %d\r\n", __HAL_PWR_GET_FLAG(PWR_FLAG_SB));

      if(__HAL_PWR_GET_FLAG(PWR_FLAG_SB)){
      	   __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
      }

   //  printf("Standby flag after: %d\r\n", __HAL_PWR_GET_FLAG(PWR_FLAG_SB));


  while(1){
   int fp;
   /*fp=cfs_open("file", CFS_WRITE);
   status=cfs_write(fp, &vector[0], sizeof(Alarm_Typedef_t));
   printf("Status: %d\r\n",status);
   status=cfs_write(fp, &vector[1], sizeof(Alarm_Typedef_t));
   printf("Status: %d\r\n",status);
   status=cfs_write(fp, &vector[2], sizeof(Alarm_Typedef_t));
   printf("Status: %d\r\n",status);
   cfs_close(fp);*/
   fp=cfs_open("file", CFS_READ);
   status=cfs_read(fp,&array_out[0],sizeof(Alarm_Typedef_t));
   //printf("Status: %d\r\n",status);
   status=cfs_read(fp,&array_out[1],sizeof(Alarm_Typedef_t));
   //printf("Status: %d\r\n",status);
   status=cfs_read(fp,&array_out[2],sizeof(Alarm_Typedef_t));
   //printf("Status: %d\r\n",status);
   status=cfs_seek(fp, 3, CFS_SEEK_END);
   //printf("Seek status: %d\r\n",status);
   for(i=0; i<3;i++){
	 //  printf("alarm %d %d:%d:%d\r\n", i, array_out[i].hour, array_out[i].minute,array_out[i].second);
   }

   time2=RTC_GetTime();
   printf("time: %d:%d:%d.%d\r\n", time2.hour,time2.minute,time2.second,time2.millisecond);
   printf("Status: %d\r\n", RTC_TimeRegulate(15,30,00, 999));
   time2=RTC_GetTime();
   printf("new time: %d:%d:%d.%d\r\n", time2.hour,time2.minute,time2.second,time2.millisecond);
   //printf("switching off\r\n");
   Set_Alarm(vector[1].hour,vector[1].minute, vector[1].second);
   Alarm_Typedef_t alarm= GetAlarm();
   printf("alarm set %d:%d:%d\r\n", alarm.hour,alarm.minute,alarm.second);
   /*Set_WakeupTimer(10000);*/
   intheap[0]=(int*)malloc(sizeof(int));
   //printf("address: %x\r\n", intheap[0]);
   if(intheap[0]!=0){
	   *intheap[0]=10;
	 //  printf("intheap: %d\r\n", *intheap[0]);
	   intheap[1]=(int*)malloc(sizeof(int));
	   //printf("address: %x\r\n", intheap[1]);
	   free(intheap[0]);
	   intheap[0]=(int*)malloc(sizeof(int));
	   //printf("address: %x\r\n", intheap[0]);




   }

 MCU_Enter_StandbyMode();
  }
  while(1){
	  PROCESS_WAIT_EVENT_UNTIL(ev==sensors_event && data==&button_sensor);
  }

  PROCESS_END();
}

void printHello(){

	printf("hello\r\n");
}


/*---------------------------------------------------------------------------*/
