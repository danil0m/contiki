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

static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_post_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

/*
 * A handler function named [resource name]_handler must be implemented for each RESOURCE.
 * A buffer for the response payload is provided through the buffer pointer. Simple resources can ignore
 * preferred_size and offset, but must respect the REST_MAX_CHUNK_SIZE limit for the buffer.
 * If a smaller block size is requested for CoAP, the REST framework automatically splits the data.
 */


RESOURCE(res_time,
         "title=\"Set time: ?var=0 if request only time, var=1 to get also date for GET method | var=00:00:00 for PUT\";rt=\"Text\"",
         res_get_handler,
         NULL,
		 res_put_handler,
         NULL);

static void
res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  char time_message[]="00:00:00 00/00/00";
  const char* var=NULL;
  Time_Typedef_t curr_time;
  Date_Typedef_t curr_date;
  int length;
  if(REST.get_query_variable(request, "var", &var)){
  	  if(var[0]=='0'){
  		  length=8;
  	  }else if (var[0]=='1'){
  		length=17;
  	  }
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

  	  if(var[0]=='1'){
  		  curr_date=RTC_GetDate();
  		if(curr_date.Date>9){
  		  		  time_message[9]='0'+curr_date.Date/10;
  		  	  }
  		  	  time_message[10]='0'+curr_date.Date%10;
  		  	  if(curr_date.Month>9){
  		  		  time_message[12]= '0'+curr_date.Month/10;
  		  	  }
  		  	  time_message[13]= '0'+curr_date.Month%10;
  		  	  if(curr_date.Year>9){
  		  		  time_message[15]='0'+curr_date.Year/10;
  		  	  }
  		  	  time_message[16]='0'+curr_date.Year%10;
  	  }
  }
  int i;
  for(i=0;i<length;i++){
	  buffer[i]=time_message[i];
  }
  buffer[i]='\0';
  REST.set_header_content_type(response, REST.type.TEXT_PLAIN); /* text/plain is the default, hence this option could be omitted. */
  REST.set_header_etag(response, (uint8_t *)length, 1);
  REST.set_response_payload(response, buffer, length);
}

/*set time and sets next alarm*/
static void res_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){

	Alarm_Typedef_t first_alarm, last_alarm;
	Time_Typedef_t time;
	char number[2];
	const char* hour;
	const char* minute;
	const char* second;
	int fp;
	int deadbeef=0;
	int read_file=0;
	static const char* var=NULL;
	fp=cfs_open("file", CFS_READ);
	if(REST.get_query_variable(request, "var", &var) && fp!=-1){
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
								  RTC_TimeRegulate(time.hour,time.minute,time.second);
								  time=RTC_GetTime();
								  printf("time: %d:%d:%d\r\n", time.hour,time.minute,time.second);
							  }
						  }
					  }
				  }

		  }
			  /*seek until first alarm greater than time*/
			  do{
			  	  			 if(cfs_read(fp,&last_alarm, sizeof(Alarm_Typedef_t))==-1){
			  	  				/*if not 0 =-1 else 0*/
			  	  				read_file=-1*read_file;
			  	  				 break;
			  	  			 }
			  	  			 if(read_file==0){
			  	  				first_alarm=last_alarm;
			  	  				read_file=1;
			  	  			 }

			  	  		 }while(!Compare_Alarm(last_alarm));

			  	  		 if(read_file==-1){
			  		  /*not found an alarm greater than time set the first*/
			  	  			 Set_Alarm(first_alarm.hour,first_alarm.minute,first_alarm.second);
			  	  			 last_alarm=GetAlarm();
			  	  			 printf("alarm set: %d:%d:%d\r\n", last_alarm.hour, last_alarm.minute, last_alarm.second);

			  	  		 }
			  	  		 else if(read_file==1){
			  	  				 Set_Alarm(last_alarm.hour,last_alarm.minute,last_alarm.second);
			  	  				 last_alarm=GetAlarm();
			  	  				 printf("alarm set: %d:%d:%d\r\n", last_alarm.hour, last_alarm.minute, last_alarm.second);
			  	  			}
			  	  		else {
			  	  			 printf("no alarm found\r\n");
			  	  		}

	  }
	  if(fp!=-1){

	  	  	  	  cfs_close(fp);
	  }
	  else{
		  /*error*/
	  }
}

/*adds to the time the alarm selected in the query variable and sets next alarm*/
static void res_post_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){

	Alarm_Typedef_t first_alarm, last_alarm;
	Time_Typedef_t time;
	const char* alarm_string=NULL;
	char number[2];
	int n_alarm;
	int i;
	int fp;
	int deadbeef=0;
	int read_chars=0;
	static const char* var=NULL;

	  int query_var;
	  fp=cfs_open("file", CFS_READ);
	  if(REST.get_query_variable(request, "var", &var) && fp!=-1){
		  printf("var: %s\r\n", var);
	  	  number[0]=var[0];
	  	  number[1]=var[1];
	  	  alarm_string=number;
		  n_alarm=atoi(alarm_string);

		  for(i=0;i<n_alarm;i++){
			  cfs_read(fp, &deadbeef, 4);
		 	  if(deadbeef==0xDEADBEEF){
		 		  break;
		 	  }
		 	  cfs_seek(fp,read_chars, CFS_SEEK_SET );
		 	  cfs_read(fp,&last_alarm, sizeof(Alarm_Typedef_t));
		 	  if(read_chars==0){
		 	  	first_alarm=last_alarm;
		 	  	 }
		 	   read_chars+=sizeof(Alarm_Typedef_t);
		 	   }

		  if(deadbeef==0XDEADBEEF){
			  printf("alarm not found\r\n");
		  }else{
			  	  time=RTC_GetTime();
		 		 RTC_TimeRegulate((time.hour+last_alarm.hour+ (time.minute+last_alarm.minute)/60)%24, (time.minute+last_alarm.minute+(time.second+last_alarm.second)/60)%60, (time.second+last_alarm.second)%60);
			  	  printf("time: %d:%d:%d\r\n", time.hour,time.minute,time.second);
			  	printf("alarm: %d:%d:%d\r\n", last_alarm.hour,last_alarm.minute,last_alarm.second);
			  /*read next alarm*/
			  	 /*seek until first alarm greater than time*/
			  	cfs_seek(fp,0, CFS_SEEK_SET);
			  	read_chars=0;
			  	deadbeef=0;
			  	do{
			  		cfs_read(fp, &deadbeef, 4);
			  		if(deadbeef==0xDEADBEEF){
			  			break;
			  		 }
			  		 cfs_seek(fp,read_chars, CFS_SEEK_SET );
			  		 cfs_read(fp,&last_alarm, sizeof(Alarm_Typedef_t));
			  		 if(read_chars==0){
			  				first_alarm=last_alarm;
			  		 }
			  		 read_chars+=sizeof(Alarm_Typedef_t);
			  		 }while(!Compare_Alarm(last_alarm));
			  		 cfs_close(fp);

			  		if(read_chars!=0){
			  		 /*not found an alarm greater than time, set the first*/
			  		 if(deadbeef==0xDEADBEEF){
			  			Set_Alarm(first_alarm.hour,first_alarm.minute,first_alarm.second);
			  			}
			  			else{
			  				Set_Alarm(last_alarm.hour,last_alarm.minute,last_alarm.second);
			  			 }
			  			last_alarm=GetAlarm();
			  			printf("alarm set: %d:%d:%d\r\n", last_alarm.hour, last_alarm.minute, last_alarm.second);
			  			 }
			  			 else {
			  				 printf("no alarm found\r\n");
			  				 }

		  }
	}
	else{
		 	  		  /*var or file error*/
	  }

	if(fp!=-1){
		cfs_close(fp);
	 }

}
