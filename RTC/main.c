/* Includes ------------------------------------------------------------------*/

#include <stdio.h>
#include "lcd.h"
#include "RTC.h"
#include "cmsis_os.h"

/* Macros --------------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

static uint16_t cuenta = 0;
static uint8_t estado_led = LED_OFF;

/* Function prototypes -------------------------------------------------------*/
/* Public functions --------------------------------------------------------- */

int main (void)
{
  RTC_TIME_Type RTCFullTime;
  char lineaHora[20];
  char lineaFecha[20];
  
  RTCFullTime.SEC = 0;
  RTCFullTime.MIN = 0;
  RTCFullTime.HOUR = 0;
  RTCFullTime.DOM = 19;
  RTCFullTime.MONTH = 3;
  RTCFullTime.YEAR  = 2022;
  
  RTC_startup(&RTCFullTime);
  LCD_SPI_startup();
  
  /* Loop forever */
  while(1)
  {
    RTC_GetFullTime (&RTCFullTime);
    sprintf(lineaHora, "%02d:%02d:%02d", RTCFullTime.HOUR, RTCFullTime.MIN, RTCFullTime.SEC);
    sprintf(lineaFecha, "%02d/%02d/%02d", RTCFullTime.DOM, RTCFullTime.MONTH, RTCFullTime.YEAR);
    print_lineas(lineaHora, lineaFecha);
    
    osDelay(PERIODO_REFRESCO_MS);
    
    if (minuto_incrementado)
    {
      estado_led = (estado_led == LED_OFF) ? LED_ON : LED_OFF;
      GPIO_PinWrite (PORT_LEDS, PIN_LED4, estado_led);
      
      if (++cuenta == DURACION_PARPADEO_MS/PERIODO_REFRESCO_MS)
      {
        cuenta = 0;
        minuto_incrementado = false;
      }
    }
    
    osThreadYield ();
  }
}

/* Private functions -------------------------------------------------------- */
