/* Includes ------------------------------------------------------------------*/

#include <stdio.h>
#include <time.h>

#include "cmsis_os.h"
#include "rl_net.h"
#include "GPIO_LPC17xx.h"
#include "PIN_LPC17xx.h"
#include "LPC17xx.h"

#include "lcd.h"
#include "RTC.h"
#include "HTTP_Server.h"


/* Macros --------------------------------------------------------------------*/

#define PERIODO_REFRESCO_MS 250

/* Public variables ----------------------------------------------------------*/

char lineaFechaHora[] = "00/00/00 00:00:00";

/* Private variables ---------------------------------------------------------*/

static uint8_t minutos_pasados = 0;
static bool actualizar_hora = false;

static RTC_TIME_Type RTCFullTime;
static struct tm SNTPFullTime;

static char lineaGain[20];

/* Function prototypes -------------------------------------------------------*/

static void time_cback (uint32_t time);

/* Public functions --------------------------------------------------------- */

void RTC_IRQHandler(void)
{
  /* This is increment counter interrupt*/
  if (LPC_RTC->ILR & RTC_IRL_RTCCIF)
  {
    minutos_pasados++;
    
    // Clear pending interrupt
    LPC_RTC->ILR |= RTC_IRL_RTCCIF;
  }
}

void thread_hora (void const *arg)
{
  RTCFullTime.SEC = 0;
  RTCFullTime.MIN = 0;
  RTCFullTime.HOUR = 0;
  RTCFullTime.DOM = 1;
  RTCFullTime.MONTH = 1;
  RTCFullTime.YEAR  = 2000;
  
  RTC_startup(&RTCFullTime);
  
  osDelay(1000);
  
  sntp_get_time (NULL, time_cback);
  
  /* Loop forever */
  while(1)
  {
    if (!actualizar_hora)
    {
      RTC_GetFullTime (&RTCFullTime);
    }
    else
    {
      actualizar_hora = false;
      
      RTCFullTime.SEC = SNTPFullTime.tm_sec;
      RTCFullTime.MIN = SNTPFullTime.tm_min;
      RTCFullTime.HOUR = (SNTPFullTime.tm_hour == 23) ? 0 : SNTPFullTime.tm_hour + 2; /* GMT+1 */
      RTCFullTime.DOM = SNTPFullTime.tm_mday;
      RTCFullTime.MONTH = SNTPFullTime.tm_mon + 1; /* tm_mon es el mes de 0 a 11 */
      RTCFullTime.YEAR = SNTPFullTime.tm_year + 1900; /* tm_year son años desde 1900 */
      
      RTC_SetFullTime(&RTCFullTime);
    }
    
    sprintf(lineaFechaHora, "%02d/%02d/%02d %02d:%02d:%02d", RTCFullTime.DOM, RTCFullTime.MONTH, (RTCFullTime.YEAR)%100, RTCFullTime.HOUR, RTCFullTime.MIN, RTCFullTime.SEC);
    sprintf(lineaGain, "Ganancia actual: %d", ganancia);
    
    print_lineas(lineaFechaHora, lineaGain);
    
    /* get hora de SNTP */
    if (minutos_pasados == 3)
    {
      minutos_pasados = 0;
      sntp_get_time (NULL, time_cback);
    }
    
    osDelay(PERIODO_REFRESCO_MS);
    
    osThreadYield();
  }
}

/* Private functions -------------------------------------------------------- */

static void time_cback (uint32_t time) {
  if (time != 0) {
    SNTPFullTime = *localtime(&time);
    actualizar_hora = true;
  }
}
