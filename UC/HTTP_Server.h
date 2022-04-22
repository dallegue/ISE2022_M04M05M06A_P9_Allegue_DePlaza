#ifndef __HTTP_SERVER_H
#define __HTTP_SERVER_H

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>

#include "cmsis_os.h"

/* Macros ------------------------------------------------------------------- */
/* Public variables ----------------------------------------------------------*/

extern bool LEDrun;
extern char lcd_text[2][20+1];
extern uint8_t ganancia;
extern uint8_t overload_valor;
extern bool overload_int_enable;
extern bool overload_status;
extern uint16_t v_out;

/* Public functions --------------------------------------------------------- */

/**
  \fn          int32_t LED_SetOut (uint32_t val)
  \brief       Control all LEDs with the bit vector \em val
  \param[in]   val  each bit represents the status of one LED.
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t  LED_SetOut       (uint32_t val);

void  comparar_valor_ADC (uint16_t valor);

/* Other -------------------------------------------------------------------- */

#endif
