/*------------------------------------------------------------------------------
 * MDK Middleware - Component ::Network
 * Copyright (c) 2004-2014 ARM Germany GmbH. All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    HTTP_Server.c
 * Purpose: HTTP Server example
 *----------------------------------------------------------------------------*/

/* Includes ------------------------------------------------------------------*/

#include <stdio.h>
#include "cmsis_os.h"                   /* CMSIS RTOS definitions             */
#include "rl_net.h"                     /* Network definitions                */

#include "Board_Buttons.h"
#include "HTTP_Server.h"
#include "PIN_LPC17xx.h"
#include "GPIO_LPC17xx.h"

#include "adc.h"
#include "lcd.h"
#include "thread_hora.h"
#include "flash.h"
#include "Net_Config_ETH_0.h"

/* Macros --------------------------------------------------------------------*/

#define LED_COUNT (4)

/* Public variables ----------------------------------------------------------*/

bool LEDrun;
char lcd_text[2][20+1];

/* Private variables ---------------------------------------------------------*/

/* LED pins:
   - LED1: P1_18 = GPIO1[18]
   - LED2: P1_20 = GPIO1[20]
   - LED3: P1_21  = GPIO1[21]
   - LED4: P1_23  = GPIO1[23] */

static const PIN LED_PIN[] = {
  {1, 18},
  {1, 20},
  {1, 21},
  {1, 23},
};

/* Function prototypes -------------------------------------------------------*/

static void BlinkLed (void const *arg);

static int32_t LED_On (uint32_t num);
static int32_t LED_Off (uint32_t num);
static int32_t LED_Initialize (void);

osThreadDef(BlinkLed, osPriorityNormal, 1, 0);
osThreadDef(thread_hora, osPriorityNormal, 1, 0);

/* Private functions -------------------------------------------------------- */

static int32_t LED_On (uint32_t num) {
  int32_t retCode = 0;

  if (num < LED_COUNT) {
    GPIO_PinWrite (LED_PIN[num].Portnum, LED_PIN[num].Pinnum, 1);
  }
  else {
    retCode = -1;
  }

  return retCode;
}

static int32_t LED_Off (uint32_t num) {
  int32_t retCode = 0;

  if (num < LED_COUNT) {
    GPIO_PinWrite (LED_PIN[num].Portnum, LED_PIN[num].Pinnum, 0);
  }
  else {
    retCode = -1;
  }

  return retCode;
}

static int32_t LED_Initialize (void) {
  uint32_t n;

  /* Enable GPIO clock */
  GPIO_PortClock     (1);

  /* Configure pins: Output Mode with Pull-down resistors */
  for (n = 0; n < LED_COUNT; n++) {
    PIN_Configure (LED_PIN[n].Portnum, LED_PIN[n].Pinnum, PIN_FUNC_0, PIN_PINMODE_PULLDOWN, PIN_PINMODE_NORMAL);
    GPIO_SetDir   (LED_PIN[n].Portnum, LED_PIN[n].Pinnum, GPIO_DIR_OUTPUT);
    GPIO_PinWrite (LED_PIN[n].Portnum, LED_PIN[n].Pinnum, 0);
  }

  return 0;
}

/* Thread 'BlinkLed': Blink the LEDs on an eval board */
static void BlinkLed (void const *arg) {
  const uint8_t led_val[5] = { 0x00, 0x01, 0x02, 0x04, 0x08};
  int cnt = 0;

  while(1) {
    // Every 100 ms
    if (LEDrun == true) {
      LED_SetOut (led_val[cnt]);
      if (++cnt >= sizeof(led_val)) {
        cnt = 0;
      }
    }
    osDelay (100);
  }
}

static void procesar_estado_leds (void)
{
  uint8_t estado_leds = leer_FLASH_LEDS();
  
  if (estado_leds == FLASH_LEDS_ROTATORIO)
  {
    LEDrun = true;
  }
  else
  {
    LED_SetOut(estado_leds);
  }
}

/* Public functions --------------------------------------------------------- */

/* No se usa */
/// Read digital inputs
uint8_t get_button (void) {
  return (Buttons_GetState ());
}

/// IP address change notification
void dhcp_client_notify (uint32_t if_num,
                         dhcpClientOption opt, const uint8_t *val, uint32_t len) {
  if (opt == dhcpClientIPaddress) {
    // IP address has changed
    sprintf (lcd_text[0],"IP address:");
    sprintf (lcd_text[1],"%s", ip4_ntoa (val));
  }
}

int32_t LED_SetOut (uint32_t val) {
  uint32_t n;

  for (n = 0; n < LED_COUNT; n++) {
    if (val & (1 << n)) LED_On (n);
    else                LED_Off(n);
  }

  return 0;
}

int main (void) {
  
  LED_Initialize     ();
  ADC_Initialize     ();
  net_initialize     ();
  LCD_SPI_startup();
  
  escribir_FLASH_MAC_IP();
  procesar_estado_leds();

  osThreadCreate (osThread(BlinkLed), NULL);
  osThreadCreate (osThread(thread_hora), NULL);
  
  /* dar control de lcd y leds a thread_hora */
  pagina_hora_seleccionada = true;
  
  while(1) {
    net_main ();
    osThreadYield ();
  }
}
