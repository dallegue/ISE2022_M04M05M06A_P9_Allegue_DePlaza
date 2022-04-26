#ifndef __THREAD_MEDIDA_H
#define __THREAD_MEDIDA_H

/* Includes ------------------------------------------------------------------*/
#include "cmsis_os.h"

#include <stdbool.h>

/* Macros ------------------------------------------------------------------- */
/* Public variables ----------------------------------------------------------*/

extern uint8_t overload_valor;
extern bool overload_int_enable;
extern uint16_t v_out;

/* Public functions --------------------------------------------------------- */

int init_thread_medida (void);

/* Other -------------------------------------------------------------------- */

#endif
