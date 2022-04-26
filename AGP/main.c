/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/

#define osObjectsPublic                     // define objects in main module
#include "osObjects.h"                      // RTOS object definitions

#include "thread_i2c.h"
#include "thread_medida.h"

/*
 * main: initialize and start the system
 */
int main (void) {
  
  osKernelInitialize ();                    // initialize CMSIS-RTOS
  
  init_thread_i2c();
  init_thread_medida();

  osKernelStart ();                         // start thread execution 
}
