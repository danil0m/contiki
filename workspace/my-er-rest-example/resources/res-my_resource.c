#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "rest-engine.h"

static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_post_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_delete_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
PROCESS(blink_light, "Blink light process");
/*
 * A handler function named [resource name]_handler must be implemented for each RESOURCE.
 * A buffer for the response payload is provided through the buffer pointer. Simple resources can ignore
 * preferred_size and offset, but must respect the REST_MAX_CHUNK_SIZE limit for the buffer.
 * If a smaller block size is requested for CoAP, the REST framework automatically splits the data.
 */
RESOURCE(res_my_resource,
         "title=\"My trial resource: ?len=0, message=\"Hello world\"..\";rt=\"Text\"",
         res_get_handler,
         res_post_handler,
		 res_put_handler,
		 res_delete_handler);
char* message = "Hello World!";

static void
res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  const char *len = NULL;
  int length = 12, mes_length; /*|<-------->| */
  /* Some data that has the length up to REST_MAX_CHUNK_SIZE. For more, see the chunk resource. */

  /* The query string can be retrieved by rest_get_query() or parsed for its key-value pairs. */
  if(REST.get_query_variable(request, "len", &len)) {
    length = atoi(len);
    if(length < 0) {
      length = 0;
    }
    if(length > REST_MAX_CHUNK_SIZE) {
      length = REST_MAX_CHUNK_SIZE;
    }

    for(mes_length=0; message[mes_length]!='\0'; mes_length++ );
    if(length>=mes_length){
    	length=mes_length;
    }
    memcpy(buffer, message, length);
  } else {
    memcpy(buffer, message, length);
  } REST.set_header_content_type(response, REST.type.TEXT_PLAIN); /* text/plain is the default, hence this option could be omitted. */
  REST.set_header_etag(response, (uint8_t *)&length, 1);
  REST.set_response_payload(response, buffer, length);
  printf("%s\r\n", message);
}
static void res_post_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){
	process_start(&blink_light , NULL);

}

static void res_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){

	REST.get_query_variable(request, "message", &message);
	printf("%s\r\n", message);

}


PROCESS_THREAD(blink_light, ev, data){
	PROCESS_BEGIN();
	int i;
	 static struct etimer et;
		for(i=0;i<10;i++){
			 etimer_set(&et, CLOCK_SECOND);
			 PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
			 leds_toggle(LEDS_RED);
			 etimer_set(&et, CLOCK_SECOND);
			 PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
			 leds_toggle(LEDS_RED);

		}
	PROCESS_END();

}

static void res_delete_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){
	memcpy(message,"\0",1);


}

