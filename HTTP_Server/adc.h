#ifndef __ADC_H
#define __ADC_H

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
/* Macros ------------------------------------------------------------------- */
/* Public variables ----------------------------------------------------------*/
/* Public functions --------------------------------------------------------- */
extern int32_t  ADC_Initialize      (void);
extern int32_t  ADC_StartConversion (void);
extern int32_t  ADC_ConversionDone  (void);
extern int32_t  ADC_GetValue        (void);
/* Other -------------------------------------------------------------------- */

#endif
