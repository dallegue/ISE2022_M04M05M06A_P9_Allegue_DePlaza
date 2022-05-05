/*------------------------------------------------------------------------------
 * MDK Middleware - Component ::Network:Service
 * Copyright (c) 2004-2014 ARM Germany GmbH. All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    HTTP_Server_CGI.c
 * Purpose: HTTP Server CGI Module
 * Rev.:    V6.00
 *----------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>

#include "rl_net.h"
#include "rl_net_lib.h"

#include "HTTP_Server.h"
#include "thread_i2c.h"
#include "flash.h"

/* Private variables ---------------------------------------------------------*/

static bool ver_timestamp_ganancia = true;

/* Public functions --------------------------------------------------------- */

// Process query string received by GET request.
void cgi_process_query (const char *qstr) {
  char var[40];

  do {
    // Loop through all the parameters
    qstr = http_get_env_var (qstr, var, sizeof (var));
  } while (qstr);
}

// Process data received by POST request.
// Type code: - 0 = www-url-encoded form data.
//            - 1 = filename for file upload (null-terminated string).
//            - 2 = file upload raw data.
//            - 3 = end of file upload (file close requested).
//            - 4 = any XML encoded POST data (single or last stream).
//            - 5 = the same as 4, but with more XML data to follow.
void cgi_process_data (uint8_t code, const char *data, uint32_t len) {
  char var[40];
  uint32_t mensaje_tx;
  
  uint8_t ganancia_anterior = ganancia;
  bool overload_int_enable_anterior = overload_int_enable;
  uint8_t overload_valor_anterior = overload_valor;

  if (code != 0) {
    // Ignore all other codes
    return;
  }
  
  if (len == 0) {
    return;
  }
  
  do
  {
    // Parse all parameters
    data = http_get_env_var (data, var, sizeof (var));
    
    /* Check if first character is non-null, if string exists */
    if (var[0] != 0)
    {
      /* control de ganancia */
      if (strcmp (var, "gain_sel=1") == 0)
      {
        ganancia = 1;
      }
      else if (strcmp (var, "gain_sel=5") == 0)
      {
        ganancia = 5;
      }
      else if (strcmp (var, "gain_sel=10") == 0)
      {
        ganancia = 10;
      }
      else if (strcmp (var, "gain_sel=50") == 0)
      {
        ganancia = 50;
      }
      else if (strcmp (var, "gain_sel=100") == 0)
      {
        ganancia = 100;
      }
      
      /* control de overload */
      else if (strcmp (var, "overload_sel=1") == 0)
      {
        overload_valor = 1;
      }
      else if (strcmp (var, "overload_sel=2") == 0)
      {
        overload_valor = 2;
      }
      else if (strcmp (var, "overload_sel=3") == 0)
      {
        overload_valor = 3;
      }
      else if (strcmp (var, "overload_sel=4") == 0)
      {
        overload_valor = 4;
      }
      else if (strcmp (var, "overload_sel=5") == 0)
      {
        overload_valor = 5;
      }
      else if (strcmp (var, "pg=overload") == 0)
      {
        
        overload_int_enable = false;
      }
      else if (strcmp (var, "overload_int=on") == 0)
      {
        overload_int_enable = true;
      }
      
      /* visualizacion de timestamp de la flash */
      else if (strcmp (var, "flash_sel=Ganancia") == 0)
      {
        ver_timestamp_ganancia = true;
      }
      else if (strcmp (var, "flash_sel=Overload") == 0)
      {
        ver_timestamp_ganancia = false;
      }
    }
  } while (data);
  
  /* Envio de datos por i2c */
  
  /* Si se ha modificado el valor de ganancia, se envia el valor nuevo por i2c a AGP */
  if (ganancia_anterior != ganancia)
  {
    mensaje_tx = 0x00000000;
    mensaje_tx |= ganancia;
    osMessagePut(queue_i2c_id, mensaje_tx, osWaitForever);
    
    escribir_FLASH_timestamp_ganancia();
  }
  
  /* Si se ha modificado el valor de overload, se envia el valor nuevo por i2c a AGP */
  if (overload_valor_anterior != overload_valor)
  {
    mensaje_tx = 0x00000100;
    mensaje_tx |= overload_valor;
    osMessagePut(queue_i2c_id, (uint32_t)mensaje_tx, osWaitForever);
  }
  
  /* Si se ha modificado la habilitacion de interrupcion por overload, se envia el valor por i2c a AGP */
  if (overload_int_enable_anterior != overload_int_enable)
  {
    mensaje_tx = 0x00000200;
    mensaje_tx |= (uint8_t) overload_int_enable;
    osMessagePut(queue_i2c_id, (uint32_t)mensaje_tx, osWaitForever);
  }
}

// Generate dynamic web data from a script line.
uint32_t cgi_script (const char *env, char *buf, uint32_t buflen, uint32_t *pcgi) {
  uint32_t len = 0;

  // Analyze a 'c' script line starting position 2
  switch (env[0]) {
    
    /* ganancia.cgi */
    case 'a' :
      switch (env[2])
      {
        case '1':
          /* Obtener valor de la ganancia */
          len = sprintf (buf, &env[4], ganancia);
          break;
        
        case '2':
          /* set ganancia linea 1 */
          len = sprintf (buf, &env[4], (ganancia == 1) ? "selected" : "",
                                       (ganancia == 5) ? "selected" : "",
                                       (ganancia == 10) ? "selected" : "");
          break;
        
        case '3':
          /* set ganancia linea 2 */
          len = sprintf (buf, &env[4], (ganancia == 50) ? "selected" : "",
                                       (ganancia == 100) ? "selected" : "");
          break;
      }
      break;

    case 'b':
      switch (env[2])
      {
        case '1':
          /* Obtener valor de overload */
          len = sprintf (buf, &env[4], overload_valor);
          break;
        
        case '2':
          /* set overload linea 1 */
          len = sprintf (buf, &env[4], (overload_valor == 1) ? "selected" : "",
                                       (overload_valor == 2) ? "selected" : "",
                                       (overload_valor == 3) ? "selected" : "");
          break;
        
        case '3':
          /* set overload linea 2 */
          len = sprintf (buf, &env[4], (overload_valor == 4) ? "selected" : "",
                                       (overload_valor == 5) ? "selected" : "");
          break;
        
        case '6':
          /* habilitar/deshabilitar interrupcion por overload */
          len = sprintf (buf, &env[4], (overload_int_enable) ? "checked" : "");
          break;
        
        case '7':
          /* estado de overload */
          len = sprintf (buf, &env[4], (overload_status) ? "checked" : "");
          break;
      }
      break;

    case 'c':
      switch (env[2])
      {
        case '1':
          /* Comprobar que timestamp esta seleccionado */
          len = sprintf (buf, &env[4], (ver_timestamp_ganancia) ? "selected" : "",
                                       (!ver_timestamp_ganancia) ? "selected" : "");
          break;
        
        case '2':
          /* mostrar informacion del timestamp almacenado en la flash */
        
          if (ver_timestamp_ganancia)
          {
            leer_FLASH_timestamp_ganancia();
          }
          else
          {
            leer_FLASH_timestamp_overload();
          }
          
          len = sprintf (buf, &env[4], (ver_timestamp_ganancia) ? timestamp_ganancia_str : timestamp_overload_str);
          break;
      }
      break;
  }
  return (len);
}
