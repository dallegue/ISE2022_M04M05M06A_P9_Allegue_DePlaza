/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>

#include "GPIO_LPC17xx.h" 
#include "PIN_LPC17xx.h"
#include "LPC17xx.h"
#include "watchdog.h"

/* Macros --------------------------------------------------------------------*/

#define CLKPWR_PCONP_PCRIT ((uint32_t)(1<<16))
#define RIT_CTRL_INT ((uint32_t) (0x01))
#define RIT_CTRL_ENCLR ((uint8_t) (0x02))
#define RIT_CTRL_ENBR ((uint8_t) (0x04))
#define RIT_CTRL_EN ((uint8_t) (0x08))
#define RIT_CLOCK ((uint32_t) SystemCoreClock/4)

#define RIT_PERIODO_US ((uint32_t) 500000)

/* Watchodog time out in us*/
#define WDT_TIMEOUT_US 5000000

/* Leds en pines 1.18, 1.20, 1.21, 1.23 */
#define PORT_LEDS 1
#define PIN_LED1 18
#define PIN_LED2 20
#define PIN_LED3 21
#define PIN_LED4 23
#define LED_ON 1
#define LED_OFF !LED_ON

#define PORT_SW 0
#define SW_DOWN 17

#define PORT_RGB_LEDS 2
#define PIN_RGB_LED_RED 3
#define PIN_RGB_LED_GREEN 2
#define PIN_RGB_LED_BLUE 1
#define RGB_LED_ON 0
#define RGB_LED_OFF !RGB_LED_ON

/* valor de fin de cuenta para que el estado inicial sea de 4s */
#define CUENTA_FIN_ESTADO_INICIAL 8

/* valor de fin de cuenta para 3s */
#define CUENTA_3_S 6

/* Public variables ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

static bool estado_inicial = true;
static uint8_t cuenta = 0;
static uint8_t estado_led_1 = LED_ON;
static uint8_t estado_led_2 = LED_OFF;
static uint8_t estado_led_3 = LED_OFF;
static uint8_t estado_led_4 = LED_OFF;

/* Other -------------------------------------------------------------------- */
/* Function prototypes -------------------------------------------------------*/

void WDT_IRQHandler(void);
void RIT_IRQHandler(void);

/* Private functions -------------------------------------------------------- */

static void leds_config(void)
{
    /* Configura pines como salida */
    GPIO_SetDir (PORT_LEDS, PIN_LED1, GPIO_DIR_OUTPUT);
    GPIO_SetDir (PORT_LEDS, PIN_LED2, GPIO_DIR_OUTPUT); 
    GPIO_SetDir (PORT_LEDS, PIN_LED3, GPIO_DIR_OUTPUT); 
    GPIO_SetDir (PORT_LEDS, PIN_LED4, GPIO_DIR_OUTPUT); 
}

/* periodo de entre 1us y 170s  */
static void rit_config(uint32_t periodo_us)
{
    /* Habilita reloj de periferico RIT */
    LPC_SC->PCONP |= CLKPWR_PCONP_PCRIT;
    
    /* Configuracion de compare register */
    LPC_RIT->RICOMPVAL = (RIT_CLOCK/1000000) * periodo_us;
    
    /* Deshabilita el mask register */
    LPC_RIT->RIMASK    = 0x00000000;
    
    /* Configuración para que la cuenta vuelva  a 0 una vez alcance */
    /* el valor en el compare register */
    LPC_RIT->RICTRL = RIT_CTRL_ENCLR | RIT_CTRL_ENBR;
    
    /* Reset del valor del contador */
    LPC_RIT->RICOUNTER    = 0x00000000;
  
    /* Arranca el timer */
    LPC_RIT->RICTRL |= RIT_CTRL_EN;
    
    NVIC_EnableIRQ(RIT_IRQn);
}

static void switch_config(void)
{
    /* Configurar pines de sw como pulldown */
    PIN_Configure(PORT_SW,SW_DOWN,PIN_FUNC_0,PIN_PINMODE_PULLDOWN,PIN_PINMODE_NORMAL);

    /* Habilitar interrupciones por flanco de subida */
    LPC_GPIOINT->IO0IntEnR = 1 << SW_DOWN;
    
    /* Habilitar interrupciones de EINT3 (interrupciones de GPIO puertos 0 y 2) */
    NVIC_EnableIRQ(EINT3_IRQn);
}

static void rgb_leds_config(void)
{
    /* Configura pines como salida */
    GPIO_SetDir (PORT_RGB_LEDS, PIN_RGB_LED_RED, GPIO_DIR_OUTPUT);
    GPIO_SetDir (PORT_RGB_LEDS, PIN_RGB_LED_GREEN, GPIO_DIR_OUTPUT); 
    GPIO_SetDir (PORT_RGB_LEDS, PIN_RGB_LED_BLUE, GPIO_DIR_OUTPUT);
  
    GPIO_PinWrite (PORT_RGB_LEDS, PIN_RGB_LED_RED, RGB_LED_OFF);
    GPIO_PinWrite (PORT_RGB_LEDS, PIN_RGB_LED_GREEN, RGB_LED_OFF);
    GPIO_PinWrite (PORT_RGB_LEDS, PIN_RGB_LED_BLUE, RGB_LED_OFF);
}

static void indicar_motivo_reset(void)
{
  if (get_watchdog_causo_reset())
  {
    GPIO_PinWrite (PORT_RGB_LEDS, PIN_RGB_LED_RED, RGB_LED_ON);
  }
  else
  {
    GPIO_PinWrite (PORT_RGB_LEDS, PIN_RGB_LED_GREEN, RGB_LED_ON);
  }
}

/* ISR ---------------------------------------------------------------------- */

void WDT_IRQHandler(void)
{
	// Disable WDT interrupt
	//NVIC_DisableIRQ(WDT_IRQn);
  
  GPIO_PinWrite (PORT_LEDS, PIN_LED3, estado_led_3 = !estado_led_3);
}

void RIT_IRQHandler(void)
{
  estado_inicial = (++cuenta <= CUENTA_FIN_ESTADO_INICIAL);
  
  if (cuenta == CUENTA_3_S)
  {
    GPIO_PinWrite (PORT_RGB_LEDS, PIN_RGB_LED_RED, RGB_LED_OFF);
    GPIO_PinWrite (PORT_RGB_LEDS, PIN_RGB_LED_GREEN, RGB_LED_OFF);
  }
  
  if (!estado_inicial)
  {
    GPIO_PinWrite (PORT_LEDS, PIN_LED1, estado_led_1 = !estado_led_1);
    GPIO_PinWrite (PORT_LEDS, PIN_LED2, estado_led_2 = !estado_led_2);
  }
  
  WDT_Feed();
  
  /* Clears RIT interrupt flag */
  LPC_RIT->RICTRL |= RIT_CTRL_INT;
}

/* Al pulsar 10 veces el switch se deja de hacer clear de su interrupcion,
   y el core se queda atendiendo esta isr en bucle y no atendiendo la del
   RIT, en la que se hace el feed del watchdog, por lo que el watchdog
   termina por saltar */
void EINT3_IRQHandler (void) {
  static uint32_t cuenta;
  if (cuenta++ < 10) {
    GPIO_PinWrite(PORT_LEDS,PIN_LED4, estado_led_4 = !estado_led_4);
    LPC_GPIOINT->IO0IntClr = 1 << SW_DOWN;
  }
}

/* Public functions --------------------------------------------------------- */

int main (void)
{
  rit_config(RIT_PERIODO_US);
  leds_config();
  switch_config();
  rgb_leds_config();
  
  indicar_motivo_reset();

  // Init WDT, interrupt mode
  WDT_Init(WDT_MODE_RESET);

  /* Enable the Watch dog interrupt*/
  NVIC_EnableIRQ(WDT_IRQn);
  
  /* Si no se configura más prioritario EINT3_IRQn que RIT_IRQn no se cumple que salte el watchdog,
     aunque no debería hacer falta porque se supone que por defecto EINT3_IRQn es más prioritaria
     que RIT_IRQn */
  NVIC_SetPriority(WDT_IRQn, 0x0);
  NVIC_SetPriority(RIT_IRQn, 0x2);
  NVIC_SetPriority(EINT3_IRQn, 0x1);
  
  /* timeout = WDT_TIMEOUT_US */
  WDT_Start(WDT_TIMEOUT_US);
  
  GPIO_PinWrite (PORT_LEDS, PIN_LED1, estado_led_1);

  while (1)
  {
    
  }
}

