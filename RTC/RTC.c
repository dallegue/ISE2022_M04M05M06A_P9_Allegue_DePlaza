/*------------------------------------------------------------------------------
 * MDK Middleware - Component ::Network
 * Copyright (c) 2004-2014 ARM Germany GmbH. All rights reserved.
 *------------------------------------------------------------------------------
 * Name:  HTTP_Server.c
 * Purpose: HTTP Server example
 *----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdint.h>
#include "lpc17xx_rtc.h"
#include "lcd.h"
#include "cmsis_os.h"                   /* CMSIS RTOS definitions             */

void RTC_IRQHandler(void);


/*----------------- INTERRUPT SERVICE ROUTINES --------------------------*/
/*********************************************************************//**
 * @brief    RTC interrupt handler sub-routine
 * @param[in]  None
 * @return     None
 **********************************************************************/
void RTC_IRQHandler(void)
{
  uint32_t secval;

  /* This is increment counter interrupt*/
  if (RTC_GetIntPending(LPC_RTC, RTC_INT_COUNTER_INCREASE))
  {
    secval = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_SECOND);

    // Clear pending interrupt
    RTC_ClearIntPending(LPC_RTC, RTC_INT_COUNTER_INCREASE);
  }

  /* Continue to check the Alarm match*/
  if (RTC_GetIntPending(LPC_RTC, RTC_INT_ALARM))
  {

    // Clear pending interrupt
    RTC_ClearIntPending(LPC_RTC, RTC_INT_ALARM);
  }
}

/*-------------------------MAIN FUNCTION------------------------------*/
/*********************************************************************//**
 * @brief    c_entry: Main RTC program body
 * @param[in]  None
 * @return     int
 **********************************************************************/
static void RTC_startup(void)
{
  /* RTC Block section ------------------------------------------------------ */
  // Init RTC module
  RTC_Init(LPC_RTC);

  /* Disable RTC interrupt */
  NVIC_DisableIRQ(RTC_IRQn);

  /* Enable rtc (starts increase the tick counter and second counter register) */
  RTC_ResetClockTickCounter(LPC_RTC);
  RTC_Cmd(LPC_RTC, ENABLE);
  RTC_CalibCounterCmd(LPC_RTC, DISABLE);

  /* Set current time for RTC */
  // Current time is 8:00:00PM, 2009-04-24
  RTC_SetTime (LPC_RTC, RTC_TIMETYPE_SECOND, 0);
  RTC_SetTime (LPC_RTC, RTC_TIMETYPE_MINUTE, 0);
  RTC_SetTime (LPC_RTC, RTC_TIMETYPE_HOUR, 20);
  RTC_SetTime (LPC_RTC, RTC_TIMETYPE_MONTH, 4);
  RTC_SetTime (LPC_RTC, RTC_TIMETYPE_YEAR, 2009);
  RTC_SetTime (LPC_RTC, RTC_TIMETYPE_DAYOFMONTH, 24);

  /* Set ALARM time for second */
  RTC_SetAlarmTime (LPC_RTC, RTC_TIMETYPE_SECOND, 10);

  /* Set the CIIR for second counter interrupt -> cambiar a cada minuto*/
  //RTC_CntIncrIntConfig (LPC_RTC, RTC_TIMETYPE_SECOND, ENABLE);

  /* Enable RTC interrupt */
  NVIC_EnableIRQ(RTC_IRQn);
}

int main (void)
{
  RTC_TIME_Type RTCFullTime;
  char lineaHora[20];
  char lineaFecha[20];
  
  RTC_startup();
  LCD_SPI_startup();
  
  /* Loop forever */
  while(1)
  {
    // Get and print current time
    RTC_GetFullTime (LPC_RTC, &RTCFullTime);
    sprintf(lineaHora, "%02d:%02d:%02d", RTCFullTime.HOUR, RTCFullTime.MIN, RTCFullTime.SEC);
    sprintf(lineaFecha, "%02d:%02d:%02d", RTCFullTime.DOM, RTCFullTime.MONTH, RTCFullTime.YEAR);
    print_lineas(lineaHora, lineaFecha);
    osDelay(1000);
  }
}
