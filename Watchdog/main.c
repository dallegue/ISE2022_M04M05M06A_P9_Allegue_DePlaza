/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>

#include "GPIO_LPC17xx.h" 
#include "PIN_LPC17xx.h"
#include "LPC17xx.h"
#include "watchdog.h"

/* Macros --------------------------------------------------------------------*/

/* Watchodog time out in us*/
#define WDT_TIMEOUT_US 5000000

#define RIT_CTRL_INT ((uint32_t) (0x01))
#define RIT_CTRL_ENCLR ((uint32_t) (0x02))
#define CLKPWR_PCONP_PCRIT ((uint32_t)(1<<16))
#define RIT_CLOCK ((uint32_t) SystemCoreClock/4)
#define RIT_PERIODO_MS ((uint32_t) 500)

/* Leds en pines 1.18, 1.20, 1.21, 1.23 */
#define PORT_LEDS 1
#define PIN_LED1 18
#define PIN_LED2 20
#define PIN_LED3 21
#define PIN_LED4 23
#define LED_ON 1
#define LED_OFF !LED_ON

/* valor de fin de cuenta para que el estado inicial sea de 5s */
#define CUENTA_FIN_ESTADO_INICIAL 10

/* Public variables ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

static bool estado_inicial = true;
static uint8_t cuenta = 0;
static uint8_t estado_led_1 = LED_ON;
static uint8_t estado_led_2 = LED_OFF;
static uint8_t estado_led_3 = LED_OFF;
//static uint8_t estado_led_4 = LED_OFF;

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

static void rit_config(void)
{
    /* Habilita reloj de periferico RIT */
    LPC_SC->PCONP |= CLKPWR_PCONP_PCRIT;
    
    /* Configuracion de compare register */
    LPC_RIT->RICOMPVAL = (RIT_CLOCK/1000) * RIT_PERIODO_MS;
    
    /* Deshabilita el mask register */
    LPC_RIT->RIMASK    = 0x00000000;
    
    /* Configuración para que la cuenta vuelva  a 0 una vez alcance */
    /* el valor en el compare register */
    LPC_RIT->RICTRL    = RIT_CTRL_ENCLR;
    
    /* Reset del valor del contador */
    LPC_RIT->RICOUNTER    = 0x00000000;
    
    NVIC_EnableIRQ(RIT_IRQn);
}

/* ISR ---------------------------------------------------------------------- */

void WDT_IRQHandler(void)
{
	// Disable WDT interrupt
	NVIC_DisableIRQ(WDT_IRQn);
  
  GPIO_PinWrite (PORT_LEDS, PIN_LED1, estado_led_3 = !estado_led_3);
  
	// Clear TimeOut flag
	LPC_WDT->WDMOD &=~WDT_WDMOD_WDTOF;
}

void RIT_IRQHandler(void)
{
  estado_inicial = (++cuenta <= CUENTA_FIN_ESTADO_INICIAL);
  
  if (!estado_inicial)
  {
    GPIO_PinWrite (PORT_LEDS, PIN_LED1, estado_led_1 = !estado_led_1);
    GPIO_PinWrite (PORT_LEDS, PIN_LED1, estado_led_2 = !estado_led_2);
  }
  
  
  /* Clears RIT interrupt flag */
  LPC_RIT->RICTRL |= RIT_CTRL_INT;
}

/* Public functions --------------------------------------------------------- */

int main (void)
{
  rit_config();
  leds_config();

  /* Install interrupt for WDT interrupt */
  NVIC_SetPriority(WDT_IRQn, 0x10);
  // Set Watchdog use internal RC, just generate interrupt only in 5ms if Watchdog is not feed

  // Init WDT, interrupt mode
  WDT_Init(WDT_MODE_INT_ONLY);

  /* Enable the Watch dog interrupt*/
  NVIC_EnableIRQ(WDT_IRQn);
  
  /* timeout = WDT_TIMEOUT_US */
  WDT_Start(WDT_TIMEOUT_US);
  
  GPIO_PinWrite (PORT_LEDS, PIN_LED1, estado_led_1);

  while (1)
  {
    
  }
}

