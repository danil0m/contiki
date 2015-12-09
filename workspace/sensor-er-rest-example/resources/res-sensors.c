#include "contiki.h"

#ifdef COMPILE_SENSORS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rest-engine.h"
#include "dev/temperature-sensor.h"
#include "dev/humidity-sensor.h"
#include "dev/acceleration-sensor.h"


static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

RESOURCE(res_sensors,
         "title=\"Sensors data pull\"",
         res_get_handler,
         NULL,
         NULL,
         NULL);

static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){


	int16_t temp_value = temperature_sensor.value(0);
	int16_t x_value = acceleration_sensor.value(X_AXIS);
	int16_t y_value = acceleration_sensor.value(Y_AXIS);
	int16_t z_value = acceleration_sensor.value(Z_AXIS);
	int16_t hum_value = humidity_sensor.value(0);

	char temp[64];
	sprintf(temp, "{\"temp\":\"%d.%d\",\"hum\":\"%d.%d\", \"a_x\":\"%d\",\"a_y\":\"%d\",\"a_z\":\"%d\"}", \
			temp_value/10, temp_value%10,hum_value/10, hum_value%10,x_value, y_value, z_value);
	printf("%s\r\n",temp);
	printf("%d\r\n", REST_MAX_CHUNK_SIZE);
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

#endif
