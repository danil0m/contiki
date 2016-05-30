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
 *      Node server.
 * \author
 *      Danilo Martino <danilo.martino17@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"
#include "rest-engine.h"
#include "rtc.h"
#include "cfs.h"
#include "low-power.h"
#include "rpl.h"
#include "net/ip/uip-debug.h"
#include "etimer.h"
#include "contiki.h"
#include "er-coap.h"
#include "er-coap-transactions.h"
#include <stdlib.h>
#include "dev/button-sensor.h"
#include "net/ip/tcpip.h"
#include "process.h"
#include "net/ipv6/uip-ds6-route.h"
#include "dev/leds.h"


#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF("[%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF("[%02x:%02x:%02x:%02x:%02x:%02x]", (lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3], (lladdr)->addr[4], (lladdr)->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#endif

/*
 * Resources to be activated need to be imported through the extern keyword.
 * The build system automatically compiles the resources in the corresponding sub-directory.
 */
extern resource_t
  res_hello,
  res_low_power,
  res_alarms,
  res_sensors,
  res_time,
  res_date,
  res_rssi,
  res_lqi,
  res_radio,
  res_sendpacket,
  res_separate,
  res_sync;

struct etimer event_timer;

static void printok(void* data, void* response){
	printf("ok\r\n");

}


int i=0;
extern struct process shutdown_process;
PROCESS(er_example_server, "Erbium Example Server");
AUTOSTART_PROCESSES(&er_example_server, &shutdown_process);

PROCESS_THREAD(er_example_server, ev, data)
{
  PROCESS_BEGIN();

  int read_file=0,fp;
  Alarm_Typedef_t first_alarm,last_alarm;

  //dis_output(NULL);
  /*when wake-up from standby, select next alarm*/
  fp=cfs_open("file", CFS_READ);
  if(fp!=-1){
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
  	  cfs_close(fp);
  }

  PROCESS_PAUSE();

  PRINTF("Starting Erbium Example Server\n");
  leds_on(LEDS_RED);
#ifdef RF_CHANNEL
  PRINTF("RF channel: %u\n", RF_CHANNEL);
#endif
#ifdef IEEE802154_PANID
  PRINTF("PAN ID: 0x%04X\n", IEEE802154_PANID);
#endif

  PRINTF("uIP buffer: %u\n", UIP_BUFSIZE);
  PRINTF("LL header: %u\n", UIP_LLH_LEN);
  PRINTF("IP+UDP header: %u\n", UIP_IPUDPH_LEN);
  PRINTF("REST max chunk: %u\n", REST_MAX_CHUNK_SIZE);

  /* Initialize the REST engine. */
  rest_init_engine();


  /*
   * Bind the resources to their Uri-Path.
   * WARNING: Activating twice only means alternate path, not two instances!
   * All static variables are the same for each URI path.
   */
  rest_activate_resource(&res_hello, "test/hello");
  rest_activate_resource(&res_low_power, "node/sdn");
  rest_activate_resource(&res_alarms, "node/alarms");
  rest_activate_resource(&res_sensors, "node/sensors");
  rest_activate_resource(&res_time, "node/time");
  rest_activate_resource(&res_date, "node/date");
  rest_activate_resource(&res_rssi, "node/rssi");
  rest_activate_resource(&res_lqi, "node/lqi");
  rest_activate_resource(&res_radio, "node/radio");
  rest_activate_resource(&res_sendpacket, "node/sendpacket");
  rest_activate_resource(&res_separate, "node/separate");
  rest_activate_resource(&res_sync, "node/sync");


  /* Define application-specific events here. */
  while(1) {

	  if(rpl_get_instance(RPL_DEFAULT_INSTANCE)!=NULL){
	        	if(rpl_get_instance(RPL_DEFAULT_INSTANCE)->current_dag!=NULL){
	        		printf("grounded\r\n");
	        		coap_transaction_t* trans;
	        		  coap_packet_t packet[1];
	        		  uip_ipaddr_t addr;
	        		  printf("root: ");
	        		  uip_debug_ipaddr_print(&(rpl_get_instance(RPL_DEFAULT_INSTANCE)->current_dag->dag_id));
	        		  printf("\r\n");
	        		  uip_ip6addr_u8(&addr, 0xaa,0xaa,0x0,0x0,0x0,0x0,0x0,0x0, 0x0, 0x00,0x0,0x0, 0x0,0x0, 0x0,0x1);
	        		  uip_debug_ipaddr_print(&addr);
	        		  printf("\r\n");
	        		  coap_init_message(packet, COAP_TYPE_CON, COAP_PUT, coap_get_mid());
	        		  coap_set_header_uri_path(packet, "sensors/");
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
	        		  trans= coap_new_transaction(packet->mid, &addr, UIP_HTONS(5683) );
	        		  trans->callback=printok;
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
/*
	        		  Time_Typedef_t time1;
	        		  	    time1=RTC_GetTime();
	        		  	    printf("time init: %d:%d:%d.%d\r\n", time1.hour,time1.minute,time1.second,time1.millisecond);
	        		  	    for(i=0;i<1000000;++i){
	        		  			if(!(i%1000)){
	        		  			PROCESS_CURRENT()->needspoll=1;
	        		  			PROCESS_WAIT_EVENT();
	        		  			}
	        		  	    }
	        		  	    Time_Typedef_t time2;
	        		  	    time2=RTC_GetTime();
	        		  	    printf("time end: %d:%d:%d.%d\r\n", time2.hour,time2.minute,time2.second,time2.millisecond);*/

	        	}
	        	else{
	        	}
	    }else{
	    	process_poll(PROCESS_CURRENT());
	        }
    PROCESS_WAIT_EVENT();
    if(ev == sensors_event && data == &button_sensor) {

      /* Also call the separate response example handler. */
      res_separate.resume();
    }


  }                             /* while (1) */

  PROCESS_END();
}
