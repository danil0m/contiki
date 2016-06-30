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
#include "low-power.h"
#include "er-coap-separate.h"
#include "er-coap-transactions.h"
#include "contiki.h"
#include "cfs.h"
#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else /* DEBUG */
#define PRINTF(...)
#endif /* DEBUG */
#define BORDER_ROUTER
extern coap_status_t erbium_status_code;
coap_packet_t coap_req;
uip_ipaddr_t source;
int port;
/*counter of incoming request*/
int counter=0;

res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_post_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void sysreset(void *data, void *response){
	  HAL_NVIC_SystemReset();
}
PROCESS(shutdown_process, "Shutdown process");

/*
 * A handler function named [resource name]_handler must be implemented for each RESOURCE.
 * A buffer for the response payload is provided through the buffer pointer. Simple resources can ignore
 * preferred_size and offset, but must respect the REST_MAX_CHUNK_SIZE limit for the buffer.
 * If a smaller block size is requested for CoAP, the REST framework automatically splits the data.
 */
SEPARATE_RESOURCE(res_low_power,
         "title=\"Enter Standby Mode\";rt=\"Text\"",
		 res_get_handler,
		 res_post_handler,
         NULL,
         NULL,
		 NULL);
res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){

	int length=5, fp, result, val;
	char message[5]="error";
	fp=cfs_open("file", CFS_READ);
		result=cfs_read(fp, &val, sizeof(int));
		cfs_close(fp);
		if(result!=-1){
			length=1;
			message[0]='0'+val;
		}
	    memcpy(buffer, message, length);
	 REST.set_header_content_type(response, REST.type.TEXT_PLAIN); /* text/plain is the default, hence this option could be omitted. */
	  REST.set_header_etag(response, (uint8_t *)&length, 1);
	  REST.set_response_payload(response, buffer, length);

}

static void
res_post_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{

	coap_transaction_t* trans;
    coap_packet_t packet[1];
	uip_ipaddr_t addr;
	/*saving request for ack*/
    int fp, value, success;
	int i;
	coap_req= *(coap_packet_t *)request;
	for(i=0;i<sizeof(uip_ipaddr_t);i++){
		source.u8[i]=UIP_IP_BUF->srcipaddr.u8[i];
	}
	port=UIP_UDP_BUF->srcport;

	fp=cfs_open("file", CFS_READ);
	if(fp!=-1){
		success=cfs_read(fp,&value, sizeof(int));
		PRINTF("status: %d, value read: %d\r\n",success, value);
		if(value==0){
		        		  //setting link-local broadcast address
		        		  memset(&addr,0, sizeof(addr));
		        		  addr.u8[0]=0xff;
		        		  addr.u8[1]=0x02;
		        		  addr.u8[15]=0x1a;
		        		  coap_init_message(packet, COAP_TYPE_CON, COAP_POST, coap_get_mid());
		        		  coap_set_header_uri_path(packet, "node/sdn");
		        		  uint8_t token[4];
		        		  uint16_t rand_numb;
		        		  rand_numb=rand();
		        		  token[0]=rand_numb/256;
		        		  token[1]=rand_numb%256;
		        		  rand_numb=rand();
		        		  token[2]=rand_numb/256;
		        		  token[3]=rand_numb%256;

		        		  coap_set_token(packet, token, 4);
		        		  trans= coap_new_transaction(packet->mid, &addr, UIP_HTONS(5683) );

		        		  //Warning: No check for serialization error.
		        		  trans->packet_len = coap_serialize_message(packet, trans->packet);
		        		  trans->callback=sysreset;
		        		  coap_send_transaction(trans);
		        		  PRINTF("node shutdown\r\n");
		        		  //not send ack
		        		  erbium_status_code = MANUAL_RESPONSE;
		}
	}

	cfs_close(fp);
}

PROCESS_THREAD(shutdown_process, ev, data){
	  PROCESS_BEGIN();


#ifdef BORDER_ROUTER_OLD

	  int init_fp,init_val=0, result;
	 		 init_fp=cfs_open("file", CFS_READ);
	 		 if(init_fp==-1){
 		 		 PRINTF("init file\r\n");
	 		 cfs_write(init_fp, &init_val, sizeof(int));
	 		 }else {
	 	 		result=cfs_read(init_fp, &init_val, sizeof(int));
	 			PRINTF("init value read %d, result %d\r\n", init_val,result);
	 	 		/*se c'era un file diverso da quello del border router*/
	 	 		if((init_val!=0 && init_val!=-1)|| result==-1){
	 	 			init_val=0;
	 		 		 cfs_write(init_fp, &init_val, sizeof(int));
	 		 		 PRINTF("init file\r\n");
	 	 		}
	 		 }
	 		cfs_close(init_fp);
#endif /*BORDER_ROUTER_OLD*/


	 		while(1){
	 			PROCESS_YIELD();
#ifndef BORDER_ROUTER_OLD
	 			MCU_Enter_StandbyMode();
#else



	 			int fp, value=0;
	 			int seek_res;

	 			fp=cfs_open("file", CFS_READ);
	if(fp!=-1){
		cfs_read(fp, &value, sizeof(int));
		seek_res=cfs_seek(fp, 0, CFS_SEEK_SET);
		PRINTF("seek: %d\r\n",seek_res);
		PRINTF("value read %d\r\n", value);
		if(value==0){
			value=1;
			cfs_write(fp, &value, sizeof(int));

			  HAL_NVIC_SystemReset();

		}else {
			value=0;
			cfs_write(fp, &value, sizeof(int));
		}
	}
	cfs_close(fp);
#endif/*BORDER ROUTER_OLD*/


	 }
	  PROCESS_END();
}

