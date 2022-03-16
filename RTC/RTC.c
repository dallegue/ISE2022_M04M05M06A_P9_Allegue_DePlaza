/* Includes ------------------------------------------------------------------*/

#include <stdio.h>
#include <stdint.h>
#include "lpc17xx_rtc.h"
#include "lcd.h"
#include "cmsis_os.h"                   /* CMSIS RTOS definitions             */
#include "GPIO_LPC17xx.h"
#include <stdbool.h>

/* Macros --------------------------------------------------------------------*/

#define PORT_LEDS 1
#define PIN_LED4 23
#define LED_ON 1
#define LED_OFF 0
#define DURACION_PARPADEO_MS 5000
#define PERIODO_REFRESCO_MS 500

/* Public variables ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

static uint16_t cuenta = 0;
static uint8_t estado_led = LED_OFF;
static bool parpadear_led = false;

/* Function prototypes -------------------------------------------------------*/

void RTC_IRQHandler(void);
static void RTC_startup(void);


/* Public functions --------------------------------------------------------- */

/* ISR */
void RTC_IRQHandler(void)
{
  /* This is increment counter interrupt*/
  if (RTC_GetIntPending(LPC_RTC, RTC_INT_COUNTER_INCREASE))
  {
    parpadear_led = true;
    
    // Clear pending interrupt
    RTC_ClearIntPending(LPC_RTC, RTC_INT_COUNTER_INCREASE);
  }
}

int main (void)
{
  RTC_TIME_Type RTCFullTime;
  char lineaHora[20];
  char lineaFecha[20];
  
  GPIO_SetDir (PORT_LEDS, PIN_LED4, GPIO_DIR_OUTPUT); 
  RTC_startup();
  LCD_SPI_startup();
  
  /* Loop forever */
  while(1)
  {
    // Get and print current time
    RTC_GetFullTime (LPC_RTC, &RTCFullTime);
    sprintf(lineaHora, "%02d:%02d:%02d", RTCFullTime.HOUR, RTCFullTime.MIN, RTCFullTime.SEC);
    sprintf(lineaFecha, "%02d/%02d/%02d", RTCFullTime.DOM, RTCFullTime.MONTH, RTCFullTime.YEAR);
    print_lineas(lineaHora, lineaFecha);
    osDelay(PERIODO_REFRESCO_MS);
    
    if (parpadear_led)
    {
      estado_led = (estado_led == LED_OFF) ? LED_ON : LED_OFF;
      GPIO_PinWrite (PORT_LEDS, PIN_LED4, estado_led);
      
      if (++cuenta == DURACION_PARPADEO_MS/PERIODO_REFRESCO_MS)
      {
        cuenta = 0;
        parpadear_led = false;
      }
    }
    
    osThreadYield ();
  }
}

/* Private functions -------------------------------------------------------- */

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
  RTC_SetTime (LPC_RTC, RTC_TIMETYPE_HOUR, 16);
  RTC_SetTime (LPC_RTC, RTC_TIMETYPE_MONTH, 3);
  RTC_SetTime (LPC_RTC, RTC_TIMETYPE_YEAR, 2022);
  RTC_SetTime (LPC_RTC, RTC_TIMETYPE_DAYOFMONTH, 24);

  /* Set ALARM time for second */
  RTC_SetAlarmTime (LPC_RTC, RTC_TIMETYPE_SECOND, 10);

  /* Set the CIIR for second counter interrupt -> cambiar a cada minuto*/
  RTC_CntIncrIntConfig (LPC_RTC, RTC_TIMETYPE_MINUTE, ENABLE);

  /* Enable RTC interrupt */
  NVIC_EnableIRQ(RTC_IRQn);
}
