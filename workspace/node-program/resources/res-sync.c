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
#include "lib/list.h"
#include "uip.h"
#include "uip-ds6-route.h"
#include "net/ip/uip-debug.h"
#include "process.h"

#include "er-coap.h"
#include "er-coap-transactions.h"

struct elem_address {
	struct elem_address *next;
	uip_ipaddr_t address;
};

static struct elem_address *address_list=NULL;
uint8_t freeze=0;
uip_ipaddr_t sender;



void choose_neigbors(int event,
	       uip_ipaddr_t *route,
	       uip_ipaddr_t *nexthop,
	       int num_routes);

static void neighbor_callback();

static void printlist(){
	struct elem_address *elem;
	elem=address_list;
	printf("{\r\n");
	while(elem!=NULL){
		uip_debug_ipaddr_print(&elem->address);
		printf("\r\n");
		elem=elem->next;
	}
	printf("}\r\n");
}

PROCESS(sync_neighbor_process, "Synchronize neigbor process");

static int compare_address(uip_ipaddr_t *addr1,
	       uip_ipaddr_t *addr2){
	int i;
	for(i=8;i<16;i++){
		if(addr1->u8[i]!=addr2->u8[i]){
			printf("i:%d ,%x!=%x\r\n",i,addr1->u8[i],addr2->u8[i]);
			return 0;
		}
	}
	return 1;
}

void choose_neigbors(int event,
	       uip_ipaddr_t *route,
	       uip_ipaddr_t *nexthop,
	       int num_routes){
	struct elem_address *elem;
	int i;

	if(compare_address(route, nexthop) && !freeze){
			if(event==UIP_DS6_NOTIFICATION_ROUTE_ADD){
				elem=(struct elem_address*)malloc(sizeof(struct elem_address));
				for(i=0;i<sizeof(uip_ipaddr_t);i++){
					elem->address.u8[i]=route->u8[i];
				}
				list_add(&address_list, elem);
				//printlist();
			}else if(event==UIP_DS6_NOTIFICATION_ROUTE_RM){
				elem=address_list;
				while(elem!=NULL){
					elem=list_item_next(elem);
					if(compare_address(route,&elem->address)){
						free(elem);
						elem=NULL;
					}
				}
			}
	}
}

static void neighbor_callback(void *data, void *response){
	printf("callback\r\n");
}

struct uip_ds6_notification notification_struct;

PROCESS_THREAD(sync_neighbor_process, ev, data){
	  PROCESS_BEGIN();
	  struct elem_address *elem;
	  coap_transaction_t* trans;
	  coap_packet_t packet[1];
	  int i;

	  printf("process sync neighbor started\r\n");
	  list_init(&address_list);
	  uip_ds6_notification_add(&notification_struct, choose_neigbors);
	  while(1){
		  PROCESS_YIELD();
		  freeze=1;
		  elem=address_list;
	  	  while(elem!=NULL){
	  		  coap_init_message(packet, COAP_TYPE_CON, COAP_PUT, coap_get_mid());
	  		  coap_set_header_uri_path(packet, "node/sync");
	  		  uint8_t token[4];
	  		  uint16_t rand_numb;
	  		  rand_numb=rand();
	  		  printf("rand1: %x\r\n", rand_numb);
	  		  token[0]=rand_numb/256;
	  		  token[1]=rand_numb%256;
	  		  rand_numb=rand();
	  		  printf("rand2: %x\r\n", rand_numb);
	  		  token[2]=rand_numb/256;
	  		  token[3]=rand_numb%256;
	  		  printf("token: ");
	  		  for(i=0;i<4;i++){
	  			  printf("%x", token[i]);
	  		  }
	  		  printf("\r\n");
	  		  coap_set_token(packet, token, 4);
	  		  trans= coap_new_transaction(packet->mid, &elem->address, UIP_HTONS(5683) );
	  		  trans->callback=neighbor_callback;
	  		  //Warning: No check for serialization error.
	  		  trans->packet_len = coap_serialize_message(packet, trans->packet);
	  		  printf("packet token: ");
	  		  for(i=0;i<packet->token_len;i++){
	  			  printf("%x", packet->token[i]);
	  		  }
	  		  printf("\r\n");
	  		  printf("packet: ");
	  		  for(i=0;i<trans->packet_len; i++){
	  			  if(trans->packet[i]<0x41){
	  				  printf("0x%x", trans->packet[i]);
		  	   	   }else{
		  		   printf("%c", trans->packet[i]);
		  	   	 }
  		  	  }
   		  	  printf("\r\n");
  		  	  coap_send_transaction(trans);



	  	  }
	  }


	  PROCESS_END();

}


static void res_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
/*
 * A handler function named [resource name]_handler must be implemented for each RESOURCE.
 * A buffer for the response payload is provided through the buffer pointer. Simple resources can ignore
 * preferred_size and offset, but must respect the REST_MAX_CHUNK_SIZE limit for the buffer.
 * If a smaller block size is requested for CoAP, the REST framework automatically splits the data.
 */


RESOURCE(res_sync,
         "title=\Sync network;rt=\"Text\"",
         NULL,
         NULL,
		 res_put_handler,
		 NULL);
/*set time and sets next alarm*/
static void res_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){

	Alarm_Typedef_t first_alarm, last_alarm;
	Time_Typedef_t time;
	char number[2];
	char subseconds[3];
	const char* hour;
	const char* minute;
	const char* second;
	const char* subsec;
	int fp;
	int i;
	int read_file=0;
	static const char* var=	NULL;
	if(REST.get_request_payload(request, &var)){
		  printf("var: %s\r\n", var);
		  /* checks format*/

			  if((var[0]>='0' && var[0]<='1' && var[1]>='0' && var[1]<='9') || (var[0]=='2' && var[1]>='0' && var[1]<='3')){
				  if(var[2]==':'){
					  if(var[3]>='0' && var[3]<='5' && var[4]>='0' && var[4]<='9'){
						  if(var[5]==':'){
							  if(var[6]>='0' && var[6]<='5' && var[7]>='0' && var[7]<='9'){
								  if(var[8]=='.'){
									  if(var[9]>='0' && var[9]<='9' && var[10]>='0' && var[10]<='9'&& var[11]>='0' && var[11]<='9'){
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
								  /*subseconds[0]=var[9];
								  subseconds[1]=var[10];
								  subseconds[2]=var[11];
								  subsec=subseconds;
								  time.millisecond=atoi(subsec);*/
								  printf("time  sent: %d:%d:%d.%d\r\n", time.hour,time.minute,time.second, 0);
								  RTC_TimeRegulate(time.hour,time.minute,time.second, time.millisecond);
								  time=RTC_GetTime();
								  printf("time: %d:%d:%d\r\n", time.hour,time.minute,time.second);
								  process_poll(&sync_neighbor_process);
									  }
									  }
								  }
						  }
					  }
				  }

		  }
	}
	fp=cfs_open("file", CFS_READ);
	 if(fp!=-1){
			do{
				  /*seek until first alarm greater than time*/
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
			  	  	  	  cfs_close(fp);

	  }
}

