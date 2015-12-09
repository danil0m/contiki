#include <stdio.h>
#include <string.h>
#include "rest-engine.h"
#include "er-coap.h"
#include "clock.h"

static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);


RESOURCE(res_timer,
         "title=\"My trial resource: ?len=0, message=\"Hello world\"..\";rt=\"Text\"",
         res_get_handler,
         NULL,
		 NULL,
		 NULL);

static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){

	char buff[64];
	uint32_t clock_ticks;
	clock_ticks =clock_time();
	sprintf(buff, "%d", clock_ticks);
	int length;
	char car=buff[0];
	for(length=0;car!='\0';length++){
		car=buff[length];
	}
	length--;
    memcpy(buffer, buff, length);
	REST.set_header_content_type(response, REST.type.TEXT_PLAIN); /* text/plain is the default, hence this option could be omitted. */
	  REST.set_header_etag(response, (uint8_t *)&length, 1);
	  REST.set_response_payload(response, buffer, length);


}


