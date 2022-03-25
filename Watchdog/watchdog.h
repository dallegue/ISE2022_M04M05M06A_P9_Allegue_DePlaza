#ifndef __WATCHDOG_H
#define __WATCHDOG_H

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>
#include <stdbool.h>

/* Macros ------------------------------------------------------------------- */
/* Other -------------------------------------------------------------------- */

/** @brief WDT operation mode */
typedef enum {
	WDT_MODE_INT_ONLY = 0, /*!< Use WDT to generate interrupt only */
	WDT_MODE_RESET = 1    /*!< Use WDT to generate interrupt and reset MCU */
} WDT_MODE_OPT;

/* Public variables ----------------------------------------------------------*/
/* Public functions --------------------------------------------------------- */

void WDT_Init (WDT_MODE_OPT WDTMode);
void WDT_Start(uint32_t TimeOut);
void WDT_Feed (void);
bool get_watchdog_causo_reset(void);

#endif
