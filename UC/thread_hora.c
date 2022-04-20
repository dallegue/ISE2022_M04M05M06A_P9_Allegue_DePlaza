/* Includes ------------------------------------------------------------------*/

#include <stdio.h>
#include <time.h>
#include "lcd.h"
#include "RTC.h"
#include "cmsis_os.h"
#include "rl_net.h"
#include "GPIO_LPC17xx.h"
#include "PIN_LPC17xx.h"
#include "LPC17xx.h"


/* Macros --------------------------------------------------------------------*/

#define PORT_LEDS 1
#define PIN_LED3 21
#define PIN_LED4 23
#define LED_ON 1
#define LED_OFF !LED_ON
#define DURACION_PARPADEO_MS 5000
#define PERIODO_REFRESCO_MS 250

#define PORT_SW 0
#define SW_CENTER 16

/* Public variables ----------------------------------------------------------*/

bool pagina_hora_seleccionada = false;
char lineaHora[20];
char lineaFecha[20];

/* Private variables ---------------------------------------------------------*/

static bool parpadear_led_3 = false;
static uint8_t estado_led_3 = LED_OFF;
static uint16_t cuenta_led_3 = 0;

static bool parpadear_led_4 = false;
static uint8_t estado_led_4 = LED_OFF;
static uint16_t cuenta_led_4 = 0;

static uint8_t minutos_pasados = 0;
static bool actualizar_hora = false;

static RTC_TIME_Type RTCFullTime;
static struct tm SNTPFullTime;

/* Function prototypes -------------------------------------------------------*/

static void time_cback (uint32_t time);
static void sw_center_config(void);

/* Public functions --------------------------------------------------------- */

void RTC_IRQHandler(void)
{
  /* This is increment counter interrupt*/
  if (LPC_RTC->ILR & RTC_IRL_RTCCIF)
  {
    parpadear_led_4 = true;
    minutos_pasados++;
    
    // Clear pending interrupt
    LPC_RTC->ILR |= RTC_IRL_RTCCIF;
  }
}

void EINT3_IRQHandler (void)
{
  if (LPC_GPIOINT->IO0IntStatR & 1 << SW_CENTER)
  {
    SNTPFullTime.tm_sec = 0;
    SNTPFullTime.tm_min = 0;
    SNTPFullTime.tm_hour = 23;
    SNTPFullTime.tm_mday = 1;
    SNTPFullTime.tm_mon = 1 - 1;
    SNTPFullTime.tm_year = 2000 - 1900;
    actualizar_hora = true;
    
    /* Borra el flag de la interrupcion */
    LPC_GPIOINT->IO0IntClr = 1 << SW_CENTER;
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
  
  /* Configurar leds para parpadeo */
  GPIO_SetDir (PORT_LEDS, PIN_LED3, GPIO_DIR_OUTPUT);
  GPIO_SetDir (PORT_LEDS, PIN_LED4, GPIO_DIR_OUTPUT);
  
  sw_center_config();
  
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
      RTCFullTime.HOUR = (SNTPFullTime.tm_hour == 23) ? 0 : SNTPFullTime.tm_hour + 1; /* GMT+1 */
      RTCFullTime.DOM = SNTPFullTime.tm_mday;
      RTCFullTime.MONTH = SNTPFullTime.tm_mon + 1; /* tm_mon es el mes de 0 a 11 */
      RTCFullTime.YEAR = SNTPFullTime.tm_year + 1900; /* tm_year son años desde 1900 */
      
      RTC_SetFullTime(&RTCFullTime);
    }
    
    sprintf(lineaHora, "%02d:%02d:%02d", RTCFullTime.HOUR, RTCFullTime.MIN, RTCFullTime.SEC);
    sprintf(lineaFecha, "%02d/%02d/%02d", RTCFullTime.DOM, RTCFullTime.MONTH, RTCFullTime.YEAR);
    
    if (pagina_hora_seleccionada)
    {
      print_lineas(lineaHora, lineaFecha);
    
      /* parpadeo de actualizacion de hora */
      if (parpadear_led_3)
      {
        estado_led_3 = GPIO_PinRead(PORT_LEDS, PIN_LED3);
        estado_led_3 = !estado_led_3;
        GPIO_PinWrite (PORT_LEDS, PIN_LED3, estado_led_3);
        
        if (++cuenta_led_3 == DURACION_PARPADEO_MS/PERIODO_REFRESCO_MS)
        {
          cuenta_led_3 = 0;
          parpadear_led_3 = false;
        }
      }
      
      /* parpadeo de incremento de minuto */
      if (parpadear_led_4)
      {
        estado_led_4 = GPIO_PinRead(PORT_LEDS, PIN_LED4);
        estado_led_4 = !estado_led_4;
        GPIO_PinWrite (PORT_LEDS, PIN_LED4, estado_led_4);
        
        if (++cuenta_led_4 == DURACION_PARPADEO_MS/PERIODO_REFRESCO_MS)
        {
          cuenta_led_4 = 0;
          parpadear_led_4 = false;
        }
      }
    }
    
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

static void sw_center_config(void)
{
    /* Configurar pines de sw como pulldown */
  PIN_Configure(PORT_SW,SW_CENTER,PIN_FUNC_0,PIN_PINMODE_PULLDOWN,PIN_PINMODE_NORMAL);

  /* Habilitar interrupciones por flanco de subida */
  LPC_GPIOINT->IO0IntEnR = 1 << SW_CENTER;
  
  /* Habilitar interrupciones de EINT3 (interrupciones de GPIO puertos 0 y 2) */
  NVIC_EnableIRQ(EINT3_IRQn);
}

static void time_cback (uint32_t time) {
  if (time != 0) {
    SNTPFullTime = *localtime(&time);
    actualizar_hora = true;
    parpadear_led_3 = true;
  }
}
