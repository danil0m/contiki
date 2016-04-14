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

static void res_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_delete_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

/*
 * A handler function named [resource name]_handler must be implemented for each RESOURCE.
 * A buffer for the response payload is provided through the buffer pointer. Simple resources can ignore
 * preferred_size and offset, but must respect the REST_MAX_CHUNK_SIZE limit for the buffer.
 * If a smaller block size is requested for CoAP, the REST framework automatically splits the data.
 */
RESOURCE(res_alarm1,
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
	int n_alarm;
	int read_chars=0, deadbeef;
	int fp;
	int i;
	Alarm_Typedef_t alarm;
	 fp=cfs_open("alarms", CFS_READ);
	 if(REST.get_query_variable(request, "alarm", &var) && fp!=-1){
		 vect[0]=var[0];
		 vect[1]=var[1];
		 /*because atoi needs const char* */
		 number=vect;
		 n_alarm=atoi(number);
		 printf("alarm selected: %d\r\n", n_alarm);
		 for(i=0;i<n_alarm;i++){
			 cfs_read(fp, &deadbeef, 4);
			 if(deadbeef==0xDEADBEEF){
				 read_chars=0;
				 printf("deadbeef\r\n");
				 break;
			 }
			 cfs_seek(fp,read_chars, CFS_SEEK_SET );
			 cfs_read(fp,&alarm, sizeof(Alarm_Typedef_t));
			 read_chars+=sizeof(Alarm_Typedef_t);
		 }
		 cfs_close(fp);
		 if(read_chars!=0){
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
			 int i;
			 for(i=0;i<8;i++){
				 buffer[i]=time_message[i];
			 }
			 buffer[i]='\0';
			 REST.set_header_content_type(response, REST.type.TEXT_PLAIN); /* text/plain is the default, hence this option could be omitted. */
			 REST.set_header_etag(response, (uint8_t *)length, 1);
			 REST.set_response_payload(response, buffer, length);
		 }
		 else{
			 REST.set_header_content_type(response, REST.type.TEXT_PLAIN); /* text/plain is the default, hence this option could be omitted. */
			 			 REST.set_header_etag(response, (uint8_t *)length, 1);
			 			 REST.set_response_payload(response, "alarm not found", 15);
		 }
	 }
	 else{
	 			 REST.set_header_content_type(response, REST.type.TEXT_PLAIN); /* text/plain is the default, hence this option could be omitted. */
	 			 			 REST.set_header_etag(response, (uint8_t *)length, 1);
	 			 			 REST.set_response_payload(response, "error", 5);
	 		 }

	 if(fp!=-1){
		 cfs_close(fp);
	 }

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
	int read_chars=0;
	int deadbeef;
	 fp=cfs_open("file", CFS_READ & CFS_WRITE & CFS_APPEND);


	 if(REST.get_query_variable(request, "alarm", &var) && fp!=-1){
		 /*checks whrere ends arrays of alarms*/
		 	 cfs_seek(fp,0, CFS_SEEK_SET );
		 	 cfs_read(fp, &deadbeef, 4);
		 	 cfs_seek(fp,read_chars, CFS_SEEK_SET );
		 	 while(deadbeef!=0xDEADBEEF){
		 		 cfs_read(fp, &prev_alarm, sizeof(Alarm_Typedef_t));
		 		 read_chars+=sizeof(Alarm_Typedef_t);
		 		 cfs_read(fp, &deadbeef, 4);
		 		 cfs_seek(fp,read_chars, CFS_SEEK_SET );
		 	 }



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
	 								  alarm.hour=atoi(hour);
	 								  number[0]=var[3];
	 								  number[1]=var[4];
	 								  minute=number;
	 								  alarm.minute=atoi(minute);
	 								  number[0]=var[6];
	 								  number[1]=var[7];
	 								  second=number;
	 								  alarm.second=atoi(second);
	 								  printf("alarm  sent: %d:%d:%d\r\n", alarm.hour,alarm.minute, alarm.second);
	 									if(read_chars==0 || Compare_Alarms(alarm, prev_alarm)){
	 										  printf("alarm written\r\n");
	 										  cfs_write(fp, &alarm, sizeof(Alarm_Typedef_t));
	 										  deadbeef=0xDEADBEEF;
	 										  cfs_write(fp, &deadbeef, 4);
	 									}
	 								  }

	 							  }
	 						  }
	 					  }
	 				  }

	 		  }

	  else {
		  /* bad response*/
	  }

	 if(fp!=-1){
		 cfs_close(fp);
	 }

	  }
static void res_delete_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){
	int fp, deadbeef;
	deadbeef=0xDEADBEEF;
	fp=cfs_open("alarms", CFS_WRITE);
	if(fp!=-1){
		cfs_write(fp,&deadbeef, 4);
		cfs_close(fp);
	}
}
