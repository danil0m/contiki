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
 *      Example resource
 * \author
 *      Matthias Kovatsch <kovatsch@inf.ethz.ch>
 */

#include <stdlib.h>
#include <string.h>
#include "rest-engine.h"
#include "rtc.h"
#include "cfs.h"
#define N_ALARMS 3

static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

/*
 * A handler function named [resource name]_handler must be implemented for each RESOURCE.
 * A buffer for the response payload is provided through the buffer pointer. Simple resources can ignore
 * preferred_size and offset, but must respect the REST_MAX_CHUNK_SIZE limit for the buffer.
 * If a smaller block size is requested for CoAP, the REST framework automatically splits the data.
 */


RESOURCE(res_set_time,
         "title=\"Time: ?var=00:00:00\";rt=\"Text\"",
         res_get_handler,
         NULL,
		 res_put_handler,
         NULL);

static void
res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  char time_message[8]="00:00:00";
  Time_Typedef_t curr_time;
  int length=8;
  curr_time=RTC_GetTime();
  if(curr_time.hour>9){
	  time_message[0]='0'+curr_time.hour/10;
  }
  time_message[1]='0'+curr_time.hour%10;
  if(curr_time.minute>9){
	  time_message[3]= '0'+curr_time.minute/10;
  }
  time_message[4]= '0'+curr_time.minute%10;
  if(curr_time.second>9){
	  time_message[6]='0'+curr_time.second/10;
  }
  time_message[7]='0'+curr_time.second%10;
  int i;
  for(i=0;i<8;i++){
	  buffer[i]=time_message[i];
  }
  buffer[i]='\0';
  REST.set_header_content_type(response, REST.type.TEXT_PLAIN); /* text/plain is the default, hence this option could be omitted. */
  REST.set_header_etag(response, (uint8_t *)length, 1);
  REST.set_response_payload(response, buffer, length);
}

static void res_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){

	Alarm_Typedef_t alarm_set;
	Time_Typedef_t time;
	Alarm_Typedef_t alarms[N_ALARMS];
	char number[2];
	const char* hour;
	const char* minute;
	const char* second;
	int i;
	int fp;
	  static const char* var=NULL;
	  int query_var;
	  if(REST.get_query_variable(request, "var", &var)){
		  printf("var: %s\r\n", var);
		  /* checks format*/

			  if((var[0]>='0' && var[0]<='1' && var[1]>='0' && var[1]<='9') || (var[0]=='2' && var[1]>='0' && var[1]<='3')){
				  if(var[2]==':'){
					  if(var[3]>='0' && var[3]<='5' && var[4]>='0' && var[4]<='9'){
						  if(var[5]==':'){
							  if(var[6]>='0' && var[6]<='5' && var[7]>='0' && var[7]<='9'){
								  number[0]=var[0];
								  number[1]=var[1];
								  hour=number;
								  time.hour=atoi(hour);
								  number[0]=var[3];
								  number[1]=var[4];
								  minute=number;
								  time.minute=atoi(minute);
								  number[0]=var[6];
								  number[1]=var[7];
								  second=number;
								  time.second=atoi(second);
								  printf("time  sent: %d:%d:%d\r\n", time.hour,time.minute,time.second);
								  RTC_TimeRegulate(time.hour,time.minute,time.second, 0);
								  time=RTC_GetTime();
								  printf("time: %d:%d:%d\r\n", time.hour,time.minute,time.second);
							  }
						  }
					  }
				  }

		  }
	  }
	  fp=cfs_open("file", CFS_READ);
	  for(i=0;i<N_ALARMS;i++){
	  cfs_read(fp, &alarms[i], sizeof(Alarm_Typedef_t));
	  }
	  cfs_close(fp);
	  i=0;
	  while( i!=N_ALARMS && !Compare_Alarm(alarms[i])){
		  i++;
	  }

	  printf("i: %d\r\n", i);
	  if(i==N_ALARMS){
		  i=0;
	  }
	  /* per salvare nel registro di backup il prossimo allarme
	  BKUPWrite(0,i);
	   */
	  Set_Alarm(alarms[i].hour,alarms[i].minute,alarms[i].second);

	  alarm_set=GetAlarm();
	  printf("alarm set: %d:%d:%d\r\n", alarm_set.hour, alarm_set.minute, alarm_set.second);

}

