#include "low-power.h"

#include "stm32l1xx.h"
#include "stm32l1xx_hal_rcc.h"
#include "stm32l1xx_hal_cortex.h"
#include "stm32l1xx_hal.h"
#include "stm32cube_hal_init.h"
#include "st-lib.h"

extern st_lib_tim_handle_typedef htim2;
/**
* @brief  This routine puts the MCU in low-power run mode. It may fail if clock configurations not set properly
* @param  None
* @retval None
*/

void MCU_Enter_LowPowerRunMode(void){
    HAL_PWREx_EnableLowPowerRunMode();
}


/**
* @brief  This routine puts the MCU in stop mode
* @param  None
* @retval None
*/
void MCU_Enter_StopMode(void)
{
  HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);  /* Infinite loop */

     /* Configures system clock after wake-up from STOP: enable HSE, PLL and select
      PLL as system clock source (HSE and PLL are disabled in STOP mode) */
    SystemClock_Config();

    printf("wakeup flag before: %d\r\n", __HAL_PWR_GET_FLAG(PWR_FLAG_WU));

     if(__HAL_PWR_GET_FLAG(PWR_FLAG_WU)){
        __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
        }

     printf("wakeup flag after: %d\r\n", __HAL_PWR_GET_FLAG(PWR_FLAG_WU));
      /* Initialize LEDs*/
      RadioShieldLedInit(RADIO_SHIELD_LED);
      //BSP_LED_Init(LED2);

}



/**
* @brief  This routine puts the MCU in standby mode
* @param  None
* @retval None
*/
void MCU_Enter_StandbyMode(void)
{
	/*in case of bad return to stanby*/
	if(__HAL_PWR_GET_FLAG(PWR_FLAG_WU)){
	   __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
	   }
	HAL_PWR_EnterSTANDBYMode();  /* Infinite loop */
}


/**
* @brief  This routine puts the MCU in lowsleep mode. It may fail if clock configurations not set properly
* @param  None
* @retval None
*/
void MCU_Enter_LowPowerSleepMode(void)
{
  /*Suspend Tick increment to prevent wakeup by Systick interrupt.
  Otherwise the Systick interrupt will wake up the device within 1ms (HAL time base)*/
  HAL_SuspendTick();
  HAL_TIM_Base_Stop_IT(&htim2);
  HAL_PWR_EnterSLEEPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI);
  /*when returning from sleep mode resume systick*/
  HAL_ResumeTick();
  st_lib_hal_tim_base_start_it(&htim2);
}



/**
* @brief  This routine puts the MCU in sleep mode
* @param  None
* @retval None
*/
void MCU_Enter_SleepMode(void)
{
  /*Suspend Tick increment to prevent wakeup by Systick interrupt.
  Otherwise the Systick interrupt will wake up the device within 1ms (HAL time base)*/
  HAL_SuspendTick();
  HAL_TIM_Base_Stop_IT(&htim2);

  HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
  /*when returning from sleep mode resume systick*/
  HAL_ResumeTick();
  st_lib_hal_tim_base_start_it(&htim2);

}
