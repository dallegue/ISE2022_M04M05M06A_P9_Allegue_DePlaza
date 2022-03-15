/*------------------------------------------------------------------------------
 * MDK Middleware - Component ::Network
 * Copyright (c) 2004-2014 ARM Germany GmbH. All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    HTTP_Server.c
 * Purpose: HTTP Server example
 *----------------------------------------------------------------------------*/

#include <stdio.h>
#include "cmsis_os.h"                   /* CMSIS RTOS definitions             */
#include "rl_net.h"                     /* Network definitions                */

#include "Board_GLCD.h"
#include "GLCD_Config.h"
#include "Board_Buttons.h"
#include "adc.h"
#include "lcd.h"

extern GLCD_FONT GLCD_Font_6x8;
extern GLCD_FONT GLCD_Font_16x24;

bool LEDrun;
bool LCDupdate;
char lcd_text[2][20+1];

static void BlinkLed (void const *arg);
static void Display (void const *arg);
int32_t LED_SetOut (uint32_t val);

osThreadDef(BlinkLed, osPriorityNormal, 1, 0);
osThreadDef(Display, osPriorityNormal, 1, 0);

/// Read analog inputs
uint16_t AD_in (uint32_t ch) {
  int32_t val = 0;

  if (ch == 0) {
    ADC_StartConversion();
    while (ADC_ConversionDone () < 0);
    val = ADC_GetValue();
  }
  return (val);
}

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
    LCDupdate = true;
  }
}

/*----------------------------------------------------------------------------
  Thread 'Display': LCD display handler
 *---------------------------------------------------------------------------*/
static void Display (void const *arg) {
  char lcd_buf[20+1];

  GLCD_Initialize         ();
  GLCD_SetBackgroundColor (GLCD_COLOR_BLUE);
  GLCD_SetForegroundColor (GLCD_COLOR_WHITE);
  GLCD_ClearScreen        ();
  GLCD_SetFont            (&GLCD_Font_16x24);
  GLCD_DrawString         (0, 1*24, "       MDK-MW       ");
  GLCD_DrawString         (0, 2*24, "HTTP Server example ");

  sprintf (lcd_text[0], "");
  sprintf (lcd_text[1], "Waiting for DHCP");
  LCDupdate = true;

  while(1) {
    if (LCDupdate == true) {
      sprintf (lcd_buf, "%-20s", lcd_text[0]);
      GLCD_DrawString (0, 5*24, lcd_buf);
      sprintf (lcd_buf, "%-20s", lcd_text[1]);
      GLCD_DrawString (0, 6*24, lcd_buf);
      LCDupdate = false;
    }
    osDelay (250);
  }
}

/*----------------------------------------------------------------------------
  Thread 'BlinkLed': Blink the LEDs on an eval board
 *---------------------------------------------------------------------------*/
static void BlinkLed (void const *arg) {
  const uint8_t led_val[5] = { 0x00, 0x01, 0x02, 0x04, 0x08};
  int cnt = 0;

  LEDrun = true;
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

/*----------------------------------------------------------------------------
  mbedApp board support
 *---------------------------------------------------------------------------*/

#include "PIN_LPC17xx.h"
#include "GPIO_LPC17xx.h"

#define LED_COUNT (4)

/* LED pins:
   - LED1: P1_18 = GPIO1[18]
   - LED2: P1_20 = GPIO1[20]
   - LED3: P1_21  = GPIO1[21]
   - LED4: P1_23  = GPIO1[23] */

const PIN LED_PIN[] = {
  {1, 18},
  {1, 20},
  {1, 21},
  {1, 23},
};


int32_t LED_On (uint32_t num) {
  int32_t retCode = 0;

  if (num < LED_COUNT) {
    GPIO_PinWrite (LED_PIN[num].Portnum, LED_PIN[num].Pinnum, 1);
  }
  else {
    retCode = -1;
  }

  return retCode;
}

int32_t LED_Off (uint32_t num) {
  int32_t retCode = 0;

  if (num < LED_COUNT) {
    GPIO_PinWrite (LED_PIN[num].Portnum, LED_PIN[num].Pinnum, 0);
  }
  else {
    retCode = -1;
  }

  return retCode;
}

int32_t LED_SetOut (uint32_t val) {
  uint32_t n;

  for (n = 0; n < LED_COUNT; n++) {
    if (val & (1 << n)) LED_On (n);
    else                LED_Off(n);
  }

  return 0;
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

/*----------------------------------------------------------------------------
  Main Thread 'main': Run Network
 *---------------------------------------------------------------------------*/
int main (void) {
  LED_Initialize     ();
  //Buttons_Initialize ();
  ADC_Initialize     ();
  net_initialize     ();

  osThreadCreate (osThread(BlinkLed), NULL);
  //osThreadCreate (osThread(Display), NULL);
  
  /* Init lcd */
  LCD_SPI_startup();
  
  while(1) {
    net_main ();
    osThreadYield ();
  }
}
