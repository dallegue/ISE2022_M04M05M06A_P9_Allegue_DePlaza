#ifndef __THREAD_HORA_H
#define __THREAD_HORA_H

/* Includes ------------------------------------------------------------------*/

#include <stdbool.h>

/* Macros ------------------------------------------------------------------- */
/* Public variables ----------------------------------------------------------*/

extern bool pagina_hora_seleccionada;
extern char lineaHora[20];
extern char lineaFecha[20];

/* Public functions --------------------------------------------------------- */

void thread_hora (void const *arg);

/* Other -------------------------------------------------------------------- */

#endif
