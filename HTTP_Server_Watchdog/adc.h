#ifndef __ADC_H
#define __ADC_H

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
/* Macros ------------------------------------------------------------------- */
/* Public variables ----------------------------------------------------------*/
/* Public functions --------------------------------------------------------- */
int32_t  ADC_Initialize      (void);
int32_t  ADC_StartConversion (void);
int32_t  ADC_ConversionDone  (void);
int32_t  ADC_GetValue        (void);
uint16_t AD_in (uint32_t ch);

/* Other -------------------------------------------------------------------- */

#endif
