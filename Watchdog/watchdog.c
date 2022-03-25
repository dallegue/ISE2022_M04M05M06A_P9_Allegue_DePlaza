/* Includes ------------------------------------------------------------------*/
#include "watchdog.h"
#include "LPC17xx.h"


/* Macros --------------------------------------------------------------------*/

/** Watchdog timer value register mask */
#define WDT_WDCLKSEL_WDSEL_MASK     (uint32_t)(0x7fffffff)

#define CLKPWR_PCLK_WDT_BITMASK  (uint32_t)(0x00000003)
#define WDT_CLKSRC_PCLK (uint32_t) (1 << 0)
#define WDT_WDMOD_WDRESET (uint8_t) (1 << 1)

/** WDT time out flag bit */
#define WDT_WDMOD_WDTOF (uint8_t) (1 << 2)

/** Define divider index for microsecond ( us ) */
#define WDT_US_INDEX	((uint32_t)(1000000))

/** WDT interrupt enable bit */
#define WDT_WDMOD_WDEN			    ((uint32_t)(1<<0))

#define WDT_WDMOD_WDTOF_BITMASK (uint8_t) (0xf4)

/* Public variables ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Other -------------------------------------------------------------------- */
/* Function prototypes -------------------------------------------------------*/
/* Private functions -------------------------------------------------------- */

/********************************************************************//**
 * @brief     Set WDT time out value and WDT mode
 * @param[in]  timeout value of time-out for WDT (us) entre 10 us y 171 s
 * @return    None
 *********************************************************************/
static void WDT_SetTimeOut (uint32_t timeout)
{
  uint32_t pclk_wdt = 0;
  uint32_t tempval = 0;
  
  // Get WDT clock with CCLK divider = 4
  pclk_wdt = SystemCoreClock / 4;
  
  // Calculate TC in WDT
  tempval  = (uint32_t) ((((uint64_t)pclk_wdt * (uint64_t)timeout / 4) / (uint64_t)WDT_US_INDEX));

  LPC_WDT->WDTC = tempval;
}

/* Public functions --------------------------------------------------------- */

/********************************************************************//**
 * @brief 		After set WDTEN, call this function to start Watchdog
 * 				or reload the Watchdog timer
 * @param[in]	None
 *
 * @return		None
 *********************************************************************/
void WDT_Feed (void)
{
	// Disable irq interrupt
	__disable_irq();
	LPC_WDT->WDFEED = 0xAA;
	LPC_WDT->WDFEED = 0x55;
	// Then enable irq interrupt
	__enable_irq();
}

/*********************************************************************//**
* @brief     Start WDT activity with given timeout value
* @param[in]  timeout value of time-out for WDT (us) entre 10 us y 171 s @ 100MHz
* @return     None
 **********************************************************************/
void WDT_Start(uint32_t TimeOut)
{
  WDT_SetTimeOut(TimeOut);
  //enable watchdog
  LPC_WDT->WDMOD |= WDT_WDMOD_WDEN;
  WDT_Feed();
}

/*********************************************************************//**
* @brief     Initial for Watchdog function
*           Clock source = PCLK ,
* @param[in]  WDTMode WDT mode, should be:
*         - WDT_MODE_INT_ONLY: Use WDT to generate interrupt only
*         - WDT_MODE_RESET: Use WDT to generate interrupt and reset MCU
* @return     None
 **********************************************************************/
void WDT_Init (WDT_MODE_OPT WDTMode)
{
  /* PCLK_WDT = CCLK/4 */
  LPC_SC->PCLKSEL0 &= ~CLKPWR_PCLK_WDT_BITMASK;

  //Set clock source PCLK
  LPC_WDT->WDCLKSEL &= ~WDT_WDCLKSEL_WDSEL_MASK;
  LPC_WDT->WDCLKSEL |= WDT_CLKSRC_PCLK;
  
  //Set WDT mode
  if (WDTMode == WDT_MODE_RESET)
  {
    LPC_WDT->WDMOD |= WDT_WDMOD_WDRESET;
  }
}

bool get_watchdog_causo_reset(void)
{
  bool watchdog_causo_reset = LPC_WDT->WDMOD & WDT_WDMOD_WDTOF;
  
  LPC_WDT->WDMOD &= ~WDT_WDMOD_WDTOF_BITMASK;
  
  return watchdog_causo_reset;
}
