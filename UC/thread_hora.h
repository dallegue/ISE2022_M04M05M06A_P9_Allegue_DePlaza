#ifndef __THREAD_HORA_H
#define __THREAD_HORA_H

/* Includes ------------------------------------------------------------------*/

#include <stdbool.h>

/* Macros ------------------------------------------------------------------- */
/* Public variables ----------------------------------------------------------*/

extern char lineaFechaHora[20];
extern char lineaGain[20];

/* Public functions --------------------------------------------------------- */

void thread_hora (void const *arg);

/* Other -------------------------------------------------------------------- */

#endif
