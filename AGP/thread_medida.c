/* Includes ------------------------------------------------------------------*/

#include "cmsis_os.h"                                           // CMSIS RTOS header file
#include "Driver_I2C.h"
#include "GPIO_LPC17xx.h" 
#include "PIN_LPC17xx.h"
#include "LPC17xx.h"

#include "thread_medida.h"
#include "adc.h"

/* Macros --------------------------------------------------------------------*/

#define PORT_INT 0
#define PIN_INT 24
#define INT_ON 1
#define INT_OFF !INT_ON

/* Leds en pines 1.18 */
#define PORT_LEDS 1
#define PIN_LED1 18
#define LED_ON 1
#define LED_OFF !LED_ON

/* Public variables ----------------------------------------------------------*/

uint8_t overload_valor = 1;
bool overload_int_enable = true;
uint16_t v_out = 0;
float v_out_volts;
float v_adc_volts;

/* Private variables ---------------------------------------------------------*/

static osThreadId tid_thread_medida;

/* Function prototypes -------------------------------------------------------*/

static void thread_medida (void const *argument);

/* Other -------------------------------------------------------------------- */

osThreadDef (thread_medida, osPriorityNormal, 1, 0);

/* Private functions -------------------------------------------------------- */

static void realizar_medida()
{
  uint16_t overload_adaptado;
  
  v_out = AD_in(0);
  //v_adc_volts = (v_out * 3.13f) / 4096;
  //v_out_volts = (v_adc_volts - 1.5) / 0.3;
  overload_adaptado = overload_valor * 409 + 2047;
  
  if (overload_int_enable && 
    ((v_out >= 2047 && v_out > overload_adaptado) ||
    (v_out < 2047 && v_out < overload_adaptado)))
  {
    GPIO_PinWrite (PORT_INT, PIN_INT, INT_ON);
    GPIO_PinWrite (PORT_LEDS, PIN_LED1, LED_ON);
  }
  else
  {
    GPIO_PinWrite (PORT_LEDS, PIN_LED1, LED_OFF);
  }
}

static void thread_medida (void const *argument)
{
  ADC_Initialize();
  
  /* pin de interrupcion por overload */
  GPIO_SetDir (PORT_INT, PIN_INT, GPIO_DIR_OUTPUT);
  
  /* led indicador de overload */
  GPIO_SetDir (PORT_LEDS, PIN_LED1, GPIO_DIR_OUTPUT);

  while (1) {
    osDelay(125);
    
    realizar_medida();
    
    osDelay(125);
    
    /* Clear linea int */
    GPIO_PinWrite (PORT_INT, PIN_INT, INT_OFF);
    
    osThreadYield ();
  }
}

/* Public functions --------------------------------------------------------- */

int init_thread_medida (void)
{
  tid_thread_medida = osThreadCreate (osThread(thread_medida), NULL);
  
  if (!tid_thread_medida)
  {
    return -1;
  }
  
  return(0);
}
