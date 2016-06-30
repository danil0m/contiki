/*
 * Copyright (c) 2013, Institute for Pervasive Computing, ETH Zurich
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
 */

/**
 * \file
 *      Alarm Resource
 * \author
 *      Danilo Martino <danilo.martino17@gmail.com>
 */

#include <stdlib.h>
#include <string.h>
#include "rest-engine.h"
#include "rtc.h"
#include "cfs.h"

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

static void res_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_delete_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

/*
 * This resource is implemented as a linked list. With put method it is put in the eeprom the value of the alarm
 * if the alarm sent is correct and it is greater than the previous. Get method gets the value of the alarm that
 * is in the position given by the value in the param. Delete method deletes all the list.
 */
RESOURCE(res_alarms_test,
         "title=\"Set Alarms?alarm=00:00:00\";rt=\"Text\"",
         res_get_handler,
		 NULL,
         res_put_handler,
		 res_delete_handler);

static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){

	char time_message[8]="00:00:00";
	const char* var=NULL;
	const char* number;
	char vect[2];
	int length=8;
	int fp;
	int i;
	int read_chars=0,seek_status;
	Alarm_Typedef_t alarm;
		 	 alarm=GetAlarm();
			 if(alarm.hour>9){
				 time_message[0]='0'+alarm.hour/10;
			 }
			 time_message[1]='0'+alarm.hour%10;
			 if(alarm.minute>9){
				 time_message[3]= '0'+alarm.minute/10;
			 }
			 time_message[4]= '0'+alarm.minute%10;
			 if(alarm.second>9){
				 time_message[6]='0'+alarm.second/10;
			 }
			 time_message[7]='0'+alarm.second%10;
			 for(i=0;i<8;i++){
				 buffer[i]=time_message[i];
			 }
			 buffer[i]='\0';
			 REST.set_header_content_type(response, REST.type.TEXT_PLAIN); /* text/plain is the default, hence this option could be omitted. */
			 REST.set_header_etag(response, (uint8_t *)length, 1);
			 REST.set_response_payload(response, buffer, length);
}


/*put in memory the given alarm*/
static void res_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
	Alarm_Typedef_t alarm, prev_alarm;
	const char *var;
	const char *hour=NULL;
	const char *minute=NULL;
	const char *second=NULL;
	char number[2];
	int fp;
	int seek_status=0;

	 if(REST.get_request_payload(request, &var)){

		 /*checks whrere ends arrays of alarms*/
		 PRINTF("payload: %s\r\n", var);
	 		  /* checks format*/

	 			  if((var[0]>='0' && var[0]<='1' && var[1]>='0' && var[1]<='9') || (var[0]=='2' && var[1]>='0' && var[1]<='3')){
	 				  if(var[2]==':'){
	 					  if(var[3]>='0' && var[3]<='5' && var[4]>='0' && var[4]<='9'){
	 						  if(var[5]==':'){
	 							  if(var[6]>='0' && var[6]<='5' && var[7]>='0' && var[7]<='9'){
	 								  number[0]=var[0];
	 								  number[1]=var[1];
	 								  hour=number;
	 								  alarm.hour=atoi(hour);
	 								  number[0]=var[3];
	 								  number[1]=var[4];
	 								  minute=number;
	 								  alarm.minute=atoi(minute);
	 								  number[0]=var[6];
	 								  number[1]=var[7];
	 								  second=number;
	 								  alarm.second=atoi(second);
	 								  PRINTF("alarm  sent: %d:%d:%d\r\n", alarm.hour,alarm.minute, alarm.second);
	 								  	  Set_Alarm(alarm.hour,alarm.minute,alarm.second);

	 						}


	 							  }
	 						  }
	 					  }
	 				  }

	 		  }

	  else {
		  /* bad response*/
	  }
	  }
static void res_delete_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){
	cfs_remove("alarms");
}
