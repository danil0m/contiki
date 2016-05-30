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

extern coap_status_t erbium_status_code;

static void res_post_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

PROCESS(shutdown_process, "Shutdown process");

/*
 * A handler function named [resource name]_handler must be implemented for each RESOURCE.
 * A buffer for the response payload is provided through the buffer pointer. Simple resources can ignore
 * preferred_size and offset, but must respect the REST_MAX_CHUNK_SIZE limit for the buffer.
 * If a smaller block size is requested for CoAP, the REST framework automatically splits the data.
 */
SEPARATE_RESOURCE(res_low_power,
         "title=\"Enter Standby Mode\";rt=\"Text\"",
         NULL,
		 res_post_handler,
         NULL,
         NULL,
		 NULL);


static void
res_post_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
	coap_transaction_t* trans;
		        		  coap_packet_t packet[1];
		        		  uip_ipaddr_t addr;
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
		        		  coap_send_transaction(trans);
		        		  printf("node shutdown\r\n");
		        		    erbium_status_code = MANUAL_RESPONSE;
		        		  process_poll(&shutdown_process);

}

PROCESS_THREAD(shutdown_process, ev, data){
	  PROCESS_BEGIN();
	  PROCESS_YIELD();
	  MCU_Enter_StandbyMode();

	  PROCESS_END();
}

