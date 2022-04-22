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
#include "Net_Config_ETH_0.h"

#include "adc.h"
#include "lcd.h"
#include "thread_hora.h"
#include "flash.h"
#include "thread_i2c.h"

/* Macros --------------------------------------------------------------------*/

#define LED_COUNT (4)

#define PORT_RGB_LEDS 2
#define PIN_RGB_LED_RED 3
#define PIN_RGB_LED_GREEN 2
#define PIN_RGB_LED_BLUE 1
#define RGB_LED_ON 0
#define RGB_LED_OFF !RGB_LED_ON

/* Public variables ----------------------------------------------------------*/

bool LEDrun;
char lcd_text[2][20+1];
uint8_t ganancia = 1;
uint8_t overload_valor = 1;
bool overload_int_enable = false;
bool overload_status = false;
uint16_t v_out = 0;

/* Private variables ---------------------------------------------------------*/
/* Function prototypes -------------------------------------------------------*/

osThreadDef(thread_hora, osPriorityNormal, 1, 0);
osThreadDef(thread_i2c, osPriorityNormal, 1, 0);

osMessageQDef(queue_i2c, 4, uint32_t);

/* Private functions -------------------------------------------------------- */
/* Public functions --------------------------------------------------------- */

int main (void) {
  ADC_Initialize     ();
  net_initialize     ();
  LCD_SPI_startup();
  init_FLASH();
  
  queue_i2c_id = osMessageCreate(osMessageQ(queue_i2c), NULL);
  
  osThreadCreate (osThread(thread_hora), NULL);
  tid_thread_i2c = osThreadCreate (osThread(thread_i2c), NULL);
  
  while(1) {
    net_main ();
    osThreadYield ();
  }
}
