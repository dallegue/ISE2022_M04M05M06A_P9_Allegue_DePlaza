#ifndef _LPC17xx_IAP_H
#define _LPC17xx_IAP_H

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>

/* Macros ------------------------------------------------------------------- */
/* Public variables ----------------------------------------------------------*/

extern char timestamp_ganancia_str[];
extern char timestamp_overload_str[];

/* Public functions --------------------------------------------------------- */

void escribir_FLASH_timestamp_ganancia (void);
void escribir_FLASH_timestamp_overload (void);
void init_FLASH (void);
void leer_FLASH_timestamp_ganancia (void);
void leer_FLASH_timestamp_overload (void);

/* Other -------------------------------------------------------------------- */

#endif /*_LPC17xx_IAP_H*/
