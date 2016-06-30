#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rest-engine.h"
#include "er-coap-separate.h"
#include "er-coap-transactions.h"
#include "rtc.h"
#include "cfs.h"

/* A structure to store the information required for the separate handler */
typedef struct application_separate_store {

  /* Provided by Erbium to store generic request information such as remote address and token. */
  coap_separate_t request_metadata;

  /* Add fields for addition information to be stored for finalizing, e.g.: */
  char buffer[32];
} application_separate_store_t;
float measure=0;
int  n_measure;
int measure_ready=0;
struct etimer resume_timer;
struct timer ready_timer;
PROCESS(measure_process, "Measure Process");


static application_separate_store_t separate_store;
static int separate_active=0;

static void res_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_resume_handler();
SEPARATE_RESOURCE(res_sensors,
         "title=\"Sensors data pull\"",
         NULL,
         NULL,
		 res_put_handler,
         NULL,
		 res_resume_handler);


PROCESS_THREAD(measure_process, ev, data){
	  PROCESS_BEGIN();

	  float temp_measure;
	  etimer_set(&resume_timer, CLOCK_SECOND/10);
	  timer_set(&ready_timer, 10*CLOCK_SECOND);
	  while(1){
		  PROCESS_YIELD();
		  if(ev==PROCESS_EVENT_TIMER){
		  	  temp_measure=measure*n_measure+0.1;
		  	  n_measure++;
		  	  measure=temp_measure/n_measure;
		  	  etimer_set(&resume_timer, CLOCK_SECOND/10);
		  	  if(timer_expired(&ready_timer)){
		  		  measure_ready=1;
		  		  res_sensors.resume();

		  	  }
		  }
	  }



	  PROCESS_END();
}


static void res_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){



	char temp[64];
	const char* json=NULL;
	/*if not active separate response*/
	if(measure_ready){
	sprintf(temp, "{\"tilt\":\"%.3f\"}",measure);
	printf("%s",temp);
	int length;
	json=temp;
	length=strlen(json);
    memcpy(buffer, temp, length);
    REST.set_header_content_type(response, REST.type.APPLICATION_JSON); /* text/plain is the default, hence this option could be omitted. */
    REST.set_header_etag(response, (uint8_t *)&length, 1);
    REST.set_response_payload(response, buffer, length);
	}else{
	    if(separate_active==1){
	        coap_separate_reject();
	    }else{
		coap_separate_accept(request, &separate_store.request_metadata);
		separate_active=1;
	    }

	}
	Alarm_Typedef_t first_alarm, last_alarm;
	Time_Typedef_t time;
	char number[2];
	char subseconds[3];
	const char* hour;
	const char* minute;
	const char* second;
	const char* subsec;
	int fp;
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
								  subseconds[0]=var[9];
								  subseconds[1]=var[10];
								  subseconds[2]=var[11];
								  subsec=subseconds;
								  time.millisecond=atoi(subsec);
								  printf("time  sent: %d:%d:%d.%d\r\n", time.hour,time.minute,time.second, time.millisecond);
								  RTC_TimeRegulate(time.hour,time.minute,time.second, time.millisecond);
								  time=RTC_GetTime();
								  printf("time: %d:%d:%d\r\n", time.hour,time.minute,time.second);
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

static void res_resume_handler(){

	if(separate_active) {
	    coap_transaction_t *transaction = NULL;
	    printf("sending message %d to ", separate_store.request_metadata.mid);
	    int i;
	    for(i=0;i<sizeof(uip_ipaddr_t);i++){
	    	printf("%x",separate_store.request_metadata.addr.u8[i]);
	    	if(i%4==0){
	    		printf(":");
	    	}
	    }
	    printf("\r\n");
	    if((transaction = coap_new_transaction(separate_store.request_metadata.mid, &separate_store.request_metadata.addr, separate_store.request_metadata.port))) {
	      coap_packet_t response[1]; /* This way the packet can be treated as pointer as usual. */
	      /* Restore the request information for the response. */
	      coap_separate_resume(response, &separate_store.request_metadata, REST.status.OK);

	  	sprintf(separate_store.buffer, "{\"tilt\":\"%.3f\"}",measure);
	      coap_set_payload(response, separate_store.buffer, strlen(separate_store.buffer));
	      /*
	       * Be aware to respect the Block2 option, which is also stored in the coap_separate_t.
	       * As it is a critical option, this example resource pretends to handle it for compliance.
	       */
	      coap_set_header_block2(response, separate_store.request_metadata.block2_num, 0, separate_store.request_metadata.block2_size);
	      /* Warning: No check for serialization error. */
	      transaction->packet_len = coap_serialize_message(response, transaction->packet);
	      coap_send_transaction(transaction);
	      /* The engine will clear the transaction (right after send for NON, after acked for CON). */
	      /* FIXME there could me more! */
	      separate_active = 0;
	    } else {
	      /*
	       * Set timer for retry, send error message, ...
	       * The example simply waits for another button press.
	       */
	    }
	  } /* if (separate_active) */

}

