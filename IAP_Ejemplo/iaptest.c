/**********************************************************************
* $Id$        iaptest.c            2012-04-18
*//**
* @file        lpc17xx_iap.h
 * @brief    IAP demo
* @version    1.0
* @date        18. April. 2012
* @author    NXP MCU SW Application Team
* 
* Copyright(C) 2011, NXP Semiconductor
* All rights reserved.
*
***********************************************************************
* Software that is described herein is for illustrative purposes only
* which provides customers with programming information regarding the
* products. This software is supplied "AS IS" without any warranties.
* NXP Semiconductors assumes no responsibility or liability for the
* use of the software, conveys no license or title under any patent,
* copyright, or mask work right to the product. NXP Semiconductors
* reserves the right to make changes in the software without
* notification. NXP Semiconductors also make no representation or
* warranty that such application will be suitable for the specified
* use without further testing or modification.
* Permission to use, copy, modify, and distribute this software and its
* documentation is hereby granted, under NXP Semiconductors'
* relevant copyright in the software, without fee, provided that it
* is used in conjunction with NXP Semiconductors microcontrollers.  This
* copyright, permission, and disclaimer notice must appear in all copies of
* this code.
**********************************************************************/

#include "lpc17xx.h"
#include "lpc_types.h"
#include "lpc17xx_iap.h"

/** The area will be erase and program */
#define FLASH_PROG_AREA_START       0x8000
#define FLASH_PROG_AREA_SIZE        0x1000

/** The origin buffer_wr on RAM, el minimo num de bytes que 
    pueden escribirse es 256 */
#define BUFF_SIZE           256

uint8_t __attribute__ ((aligned (4))) buffer_wr[BUFF_SIZE];

/* Solo los primeros 16 bytes son de interes */
#define BUFF_READ_SIZE           16

static uint32_t i;
static uint32_t flash_prog_area_sec_start;
static uint32_t flash_prog_area_sec_end;
static IAP_STATUS_CODE status;

static uint8_t buffer_rd[BUFF_READ_SIZE];

static void borrar_sector(void)
{
  /* Borrar sector 8 */
  status = EraseSector(flash_prog_area_sec_start, flash_prog_area_sec_end); 
  if(status != CMD_SUCCESS)
  {
     /* Erase chip failed */
     while(1); 
  }
}

static void escribir_sector(void)
{
  /* Escribir las primeras 16 posiciones del sector 8 */
  status =  CopyRAM2Flash((uint8_t*) FLASH_PROG_AREA_START, buffer_wr, IAP_WRITE_256);
  if(status != CMD_SUCCESS)
  {
     /* Program chip failed */
     while(1);
  }
}

static void modificar_byte(uint8_t posicion, uint8_t valor)
{
  /* Leer las primeras 16 posiciones del sector 8, modificar la segunda posicion
     y escribir el resultado en la flash otra vez */
  for (i = 0;i < sizeof(buffer_wr);i++)
  {
    buffer_wr[i] = *(uint8_t*)(FLASH_PROG_AREA_START + i);
  }
  
  borrar_sector();
  
  buffer_wr[posicion] = valor;
  
  escribir_sector();
}

static void leer_sector(void)
{
  /* Leer las primeras 16 posiciones del sector 8 */
  for (i = 0;i < sizeof(buffer_rd);i++)
  {
    buffer_rd[i] = *(uint8_t*)(FLASH_PROG_AREA_START + i);
  }
}

int main (void)
{
  // Initialize
  for (i = 0;i < sizeof(buffer_wr);i++)
  {
    if (i < 16)
    {
      buffer_wr[i] = (uint8_t)i;
    }
    else
    {
      buffer_wr[i] = 0;
    }
  }
  
  flash_prog_area_sec_start = GetSecNum(FLASH_PROG_AREA_START);
  flash_prog_area_sec_end =  GetSecNum(FLASH_PROG_AREA_START + FLASH_PROG_AREA_SIZE - 1); /* selecciona solo el sector 8 */

  borrar_sector();
  
  escribir_sector();
  
  leer_sector();
  
  modificar_byte(1, 0xea);
  
  while (1);
}
