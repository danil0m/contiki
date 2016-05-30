#include "rtc.h"



extern st_lib_rtc_handle_typedef st_lib_rtc_handle;

static void Error_Handler(void);

/**
  * @brief  Configure the RTC peripheral by selecting the clock source.
  * @param  None
  * @retval None
  */

void RTC_Config(void)
{
  /*##-1- Configure the RTC peripheral #######################################*/
	st_lib_rtc_handle.Instance = RTC;

  /* Configure RTC prescaler and RTC data registers */
  /* RTC configured as follow:
  - Hour Format    = Format 24
  - Asynch Prediv  = Value according to source clock
  - Synch Prediv   = Value according to source clock
  - OutPut         = Output Disable
  - OutPutPolarity = High Polarity
  - OutPutType     = Open Drain */
	st_lib_rtc_handle.Init.HourFormat = RTC_HOURFORMAT_24;
	st_lib_rtc_handle.Init.AsynchPrediv = RTC_ASYNCH_PREDIV;
	st_lib_rtc_handle.Init.SynchPrediv = RTC_SYNCH_PREDIV;
	st_lib_rtc_handle.Init.OutPut = RTC_OUTPUT_DISABLE;
	st_lib_rtc_handle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
	st_lib_rtc_handle.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;

  if(HAL_RTC_Init(&st_lib_rtc_handle) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }
}

Date_Typedef_t RTC_GetDate(){
	RTC_DateTypeDef date;
	RTC_TimeTypeDef time;
	HAL_RTC_GetTime(&st_lib_rtc_handle, &time, FORMAT_BIN);
	HAL_RTC_GetDate(&st_lib_rtc_handle, &date, FORMAT_BIN);
	return date;

}
HAL_StatusTypeDef RTC_SetDate(Date_Typedef_t date){
	return HAL_RTC_SetDate(&st_lib_rtc_handle, &date, FORMAT_BIN);
}
HAL_StatusTypeDef RTC_TimeRegulate(uint8_t hh, uint8_t mm, uint8_t ss, uint32_t milliseconds){
    RTC_TimeTypeDef stimestructure;

	stimestructure.Hours = hh;
	    stimestructure.Minutes = mm;
	    stimestructure.Seconds = ss;
	    stimestructure.SubSeconds=(1000-milliseconds)*(RTC_SYNCH_PREDIV+1)/1000;
	    stimestructure.TimeFormat = RTC_HOURFORMAT_24;
	    stimestructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
	    stimestructure.StoreOperation = RTC_STOREOPERATION_RESET;


	HAL_RTC_SetTime(&st_lib_rtc_handle, &stimestructure, FORMAT_BIN);
	HAL_RTCEx_SetSynchroShift(&st_lib_rtc_handle, RTC_SHIFTADD1S_SET, (1000-milliseconds)*(RTC_SYNCH_PREDIV)/1000);
}



Time_Typedef_t RTC_GetTime(){
	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;
	Time_Typedef_t ret_time;
	HAL_RTC_GetTime(&st_lib_rtc_handle, &sTime,FORMAT_BIN );
	HAL_RTC_GetDate(&st_lib_rtc_handle, &sDate, FORMAT_BIN);
	ret_time.hour=sTime.Hours;
	ret_time.minute=sTime.Minutes;
	ret_time.second=sTime.Seconds;
	ret_time.millisecond=1000-1000*sTime.SubSeconds/(RTC_SYNCH_PREDIV+1);
	return ret_time;
}



void Set_WakeupTimer(uint32_t milliseconds){
	/*Wakeup Time Base = 16 /(~39.000KHz) = ~0,410 ms
	      Wakeup Time = seconds = 0,410ms  * WakeUpCounter
	       ==> WakeUpCounter = seconds /0,410ms = 9750 = 0x2616 */
    HAL_RTCEx_SetWakeUpTimer_IT(&st_lib_rtc_handle, milliseconds/0.488, RTC_WAKEUPCLOCK_RTCCLK_DIV16);

}

int Set_Alarm(uint8_t hours, uint8_t minutes, uint8_t seconds){

	  RTC_AlarmTypeDef sAlarm;

	  if(hours>24 ||  minutes>60 || seconds>60){
		  return HAL_ERROR;
	  }

	  sAlarm.AlarmTime.Hours = hours;
	  sAlarm.AlarmTime.Minutes = minutes;
	  sAlarm.AlarmTime.Seconds= seconds;
	  sAlarm.AlarmDateWeekDay = 0x0;
	  sAlarm.AlarmTime.TimeFormat = RTC_HOURFORMAT_24;
	  sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	  sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
	  sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY;
	  sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
	  sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
	  sAlarm.Alarm = RTC_ALARM_A;
	  return HAL_RTC_SetAlarm_IT(&st_lib_rtc_handle, &sAlarm, FORMAT_BIN);
}

void Disable_WakeupTimer(){
    HAL_RTCEx_DeactivateWakeUpTimer(&st_lib_rtc_handle);

}

void Disable_Alarm(){
	HAL_RTC_DeactivateAlarm(&st_lib_rtc_handle,RTC_ALARM_A);
}


Alarm_Typedef_t GetAlarm(){
	RTC_AlarmTypeDef alarm;
	Alarm_Typedef_t alarm_ret;
	HAL_RTC_GetAlarm(&st_lib_rtc_handle, &alarm, RTC_ALARM_A, FORMAT_BIN);
	alarm_ret.hour= alarm.AlarmTime.Hours;
	alarm_ret.minute=alarm.AlarmTime.Minutes;
	alarm_ret.second=alarm.AlarmTime.Seconds;
	return alarm_ret;
}


/*gives 1 if alarm greater than time*/
int Compare_Alarm(Alarm_Typedef_t alarm){
	Time_Typedef_t time;
	time=RTC_GetTime();
	if(time.hour<alarm.hour){
		return 1;
	}
	else if(time.hour==alarm.hour && time.minute<alarm.minute){
		return 1;
	}
		else if(time.hour==alarm.hour && time.minute==alarm.minute && time.second < alarm.second){
			return 1;
		}
	return 0;
}

/*return 1 if alarm1 > alarm2 else 0*/
int Compare_Alarms(Alarm_Typedef_t alarm1, Alarm_Typedef_t alarm2){

	if(alarm1.hour>alarm2.hour){
		return 1;
	}
	else if(alarm1.hour==alarm2.hour && alarm1.minute>alarm1.minute){
		return 1;
	}
	else if(alarm1.hour==alarm2.hour && alarm1.minute==alarm2.minute &&alarm1.second> alarm2.second){
		return 1;
	}
	return 0;
}


/**
 * @brief  This function is executed in case of error occurrence.
 * @param  None
 * @retval None
 */
static void Error_Handler(void)
{
  /* User may add here some code to deal with this error */
      while(1)
      {
      }
}

