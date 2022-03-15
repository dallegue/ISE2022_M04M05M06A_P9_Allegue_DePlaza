#ifndef __LCD_H
#define __LCD_H

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Public variables ----------------------------------------------------------*/
/* Public functions --------------------------------------------------------- */

/* Llamar a esta funcion antes de usar LCD */
void LCD_SPI_startup(void);

/* Numero maximo caracteres por linea de 25, puede que el display pueda con menos aun */
void print_linea(uint8_t num_linea, char linea[]);

void print_lineas(char line1[], char line2[]);

void clear_screen(void);

void clear_linea_buffer(uint8_t num_linea);

/* Other -------------------------------------------------------------------- */
#define LINEA_1 1U
#define LINEA_2 2U



#endif
