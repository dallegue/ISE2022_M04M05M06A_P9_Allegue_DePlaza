#ifndef _LPC17xx_IAP_H
#define _LPC17xx_IAP_H

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>

/* Macros ------------------------------------------------------------------- */
/* Public variables ----------------------------------------------------------*/
/* Public functions --------------------------------------------------------- */

void escribir_FLASH_timestamp_ganancia (void);
void escribir_FLASH_timestamp_overload (void);
void init_FLASH (void);

/* Other -------------------------------------------------------------------- */

#endif /*_LPC17xx_IAP_H*/
