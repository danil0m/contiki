#include "stm32l1xx.h"
#include "stm32l1xx_hal_rcc.h"
#include "stm32l1xx_hal_cortex.h"
#include "stm32l1xx_hal.h"
#include "st-lib.h"

#define RTC_CLOCK_SOURCE_LSI


#ifdef RTC_CLOCK_SOURCE_LSI
  #define RTC_ASYNCH_PREDIV  0x7F
  #define RTC_SYNCH_PREDIV   0x0130
#endif

#ifdef RTC_CLOCK_SOURCE_LSE
  #define RTC_ASYNCH_PREDIV  0x7F
  #define RTC_SYNCH_PREDIV   0x00FF
#endif

typedef struct {
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
} Alarm_Typedef_t;

typedef struct {
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
} Time_Typedef_t;

typedef RTC_DateTypeDef Date_Typedef_t;


void RTC_Config(void);
Date_Typedef_t RTC_GetDate();
HAL_StatusTypeDef RTC_SetDate(Date_Typedef_t date);
Time_Typedef_t RTC_GetTime();
HAL_StatusTypeDef RTC_TimeRegulate(uint8_t hh, uint8_t mm, uint8_t ss);
void Set_WakeupTimer(uint32_t milliseconds);
void Disable_WakeupTimer();
int Set_Alarm(uint8_t hour, uint8_t minutes, uint8_t seconds);
Alarm_Typedef_t GetAlarm();
void Disable_Alarm();
int Compare_Alarm(Alarm_Typedef_t alarm);
int Compare_Alarms(Alarm_Typedef_t alarm1, Alarm_Typedef_t alarm2);
