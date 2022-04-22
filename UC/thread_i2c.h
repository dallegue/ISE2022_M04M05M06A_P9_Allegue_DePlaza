#ifndef __THREAD_I2C_H
#define __THREAD_I2C_H

/* Includes ------------------------------------------------------------------*/

#include <stdbool.h>

#include "cmsis_os.h"

/* Macros ------------------------------------------------------------------- */
/* Public variables ----------------------------------------------------------*/

extern osMessageQId queue_i2c_id;
extern osThreadId tid_thread_i2c;

/* Public functions --------------------------------------------------------- */

void thread_i2c (void const *argument);                             // thread function

/* Other -------------------------------------------------------------------- */

#endif
