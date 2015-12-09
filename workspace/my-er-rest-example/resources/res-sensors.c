#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rest-engine.h"
#include <time.h>

static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

RESOURCE(res_sensors,
         "title=\"Sensors data pull\"",
         res_get_handler,
         NULL,
         NULL,
         NULL);

static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){


	int r = 23;
	char temp[64];
	sprintf(temp, "{\"temperature\" : \"%d\" }", r);
	printf("%s",temp);
	int length;
	char car;
	car=temp[0];
	for(length=0; car!='\0'; length++){
		car=temp[length];
	}
	length--;
    memcpy(buffer, temp, length);
    REST.set_header_content_type(response, REST.type.APPLICATION_JSON); /* text/plain is the default, hence this option could be omitted. */
    REST.set_header_etag(response, (uint8_t *)&length, 1);
    REST.set_response_payload(response, buffer, length);

}
