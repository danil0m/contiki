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
#include "er-coap-constants.h"

#include "stm32l1xx.h"
#include "stm32l1xx_hal_rcc.h"
#include "stm32l1xx_hal_cortex.h"
#include "stm32l1xx_hal.h"

static void res_post_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

PROCESS(reset_process, "Reset process");

/*
 * A handler function named [resource name]_handler must be implemented for each RESOURCE.
 * A buffer for the response payload is provided through the buffer pointer. Simple resources can ignore
 * preferred_size and offset, but must respect the REST_MAX_CHUNK_SIZE limit for the buffer.
 * If a smaller block size is requested for CoAP, the REST framework automatically splits the data.
 */
SEPARATE_RESOURCE(res_reset,
         "title=\"Reset Node\";rt=\"Text\"",
         NULL,
		 res_post_handler,
         NULL,
         NULL,
		 NULL);


static void
res_post_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
	coap_packet_t ack[1];
	coap_packet_t *const coap_req = (coap_packet_t *)request;


	 /* sending ACK */
	 coap_init_message(ack, COAP_TYPE_ACK, CONTENT_2_05, coap_req->mid);
	 /* serializing into IPBUF: Only overwrites header parts that are already parsed into the request struct */
	 coap_send_message(&UIP_IP_BUF->srcipaddr, UIP_UDP_BUF->srcport,
	                        (uip_appdata), coap_serialize_message(ack,
	                                                              uip_appdata));

	process_poll(&reset_process);

}

PROCESS_THREAD(reset_process, ev, data){
	  PROCESS_BEGIN();
	  while(1){
		  PROCESS_YIELD();
	  	  if(ev==PROCESS_EVENT_POLL){
	  		  HAL_NVIC_SystemReset();
	  	  }
	  }
	  PROCESS_END();
}

