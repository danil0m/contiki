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
static void res_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

/*
 * A handler function named [resource name]_handler must be implemented for each RESOURCE.
 * A buffer for the response payload is provided through the buffer pointer. Simple resources can ignore
 * preferred_size and offset, but must respect the REST_MAX_CHUNK_SIZE limit for the buffer.
 * If a smaller block size is requested for CoAP, the REST framework automatically splits the data.
 */


RESOURCE(res_date,
         "title=\"Date: ?var=00/00\";rt=\"Text\"",
         res_get_handler,
         NULL,
		 res_put_handler,
         NULL);

static void
res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  char date_message[8]="00/00/00";
  Date_Typedef_t curr_date;
  int length=8;
  curr_date=RTC_GetDate();
  if(curr_date.Date>9){
	  date_message[0]='0'+curr_date.Date/10;
  }
  date_message[1]='0'+curr_date.Date%10;
  if(curr_date.Month>9){
	  date_message[3]= '0'+curr_date.Month/10;
  }
  date_message[4]= '0'+curr_date.Month%10;
  if(curr_date.Year>9){
	  date_message[6]='0'+curr_date.Year/10;
  }
  date_message[7]='0'+curr_date.Year%10;
  int i;
  for(i=0;i<8;i++){
	  buffer[i]=date_message[i];
  }
  buffer[i]='\0';
  REST.set_header_content_type(response, REST.type.TEXT_PLAIN); /* text/plain is the default, hence this option could be omitted. */
  REST.set_header_etag(response, (uint8_t *)length, 1);
  REST.set_response_payload(response, buffer, length);
}

/*set time and sets next alarm*/
static void res_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){

	Date_Typedef_t date;
	char number[2];
	const char* day;
	const char* month;
	const char* year;;
	static const char* var=NULL;
	if(REST.get_query_variable(request, "var", &var)){
		  printf("var: %s\r\n", var);
		  /* checks format*/

			  if((var[0]>='0' && var[0]<='9' && var[1]>='0' && var[1]<='9') && (var[3]>='0' && var[3]<='9' && var[4]>='0'&&var[4]<='9')&& (var[6]>='0' && var[6]<='9' && var[7]>='0'&&var[7]<='9')){
								  number[0]=var[0];
								  number[1]=var[1];
								  day=number;
								  date.Date=atoi(day);
								  number[0]=var[3];
								  number[1]=var[4];
								  month=number;
								  date.Month=atoi(month);
								  number[0]=var[6];
								  number[1]=var[7];
								  year=number;
								  date.Year=atoi(year);
								  date.WeekDay=1;
								  printf("date  sent: %d/%d/%d\r\n", date.Date,date.Month,date.Year);
								  if(date.Date>32){
								  /*error*/
								  }else if(date.Date==31){
									  	  if(date.Month==2 || date.Month==4||date.Month==6|| date.Month==9|| date.Month==11){
										  /*error*/
									  	  }
									  	  else{
									  		  printf("date set1\r\n");
									  		  RTC_SetDate(date);
									  	  	  }
									  }else if(date.Date==30 && date.Month==2){
									  	/*error*/
									  		}else if( date.Date==29 && date.Month==2 && (date.Year%4)){
											  /*error*/
									  			}else{
									  				printf("date set2\r\n");
											  printf("status: %d\r\n",RTC_SetDate(date));
									  			}
				 }
			  date=RTC_GetDate();
			  printf("curr date  : %d/%d/%d\r\n", date.Date,date.Month,date.Year);


		}

}
