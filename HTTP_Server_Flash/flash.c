/**********************************************************************
* $Id$		lpc17xx_iap.c			2012-04-18
*//**
* @file		lpc17xx_iap.c
 * @brief	Contains all functions support for IAP on lpc17xx
* @version	1.0
* @date		18. April. 2012
* @author	NXP MCU SW Application Team
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

/* Includes ------------------------------------------------------------------*/

#include "flash.h"
#include "system_lpc17xx.h"
#include "lpc17xx.h"
#include "Net_Config_ETH_0.h"

/* Macros --------------------------------------------------------------------*/

/* NULL pointer */
#ifndef NULL
#define NULL ((void*) 0)
#endif

/** IAP entry location */
#define IAP_LOCATION              (0x1FFF1FF1UL)

//  IAP Command
typedef void (*IAP)(uint32_t *cmd,uint32_t *result);
IAP iap_entry_flash_c = (IAP) IAP_LOCATION;
#define IAP_Call 	iap_entry_flash_c

/** The area will be erase and program */
#define FLASH_PROG_AREA_START       0xf000
#define FLASH_PROG_AREA_SIZE        0x1000

/** The origin buffer_wr on RAM, el minimo num de bytes que 
    pueden escribirse es 256 */
#define BUFF_SIZE           256

uint8_t __attribute__ ((aligned (4))) buffer_wr[BUFF_SIZE];

/* Solo los primeros 16 bytes son de interes */
//#define BUFF_READ_SIZE           16

/* Public variables ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

static uint32_t i;
static uint32_t flash_prog_area_sec_start;
static uint32_t flash_prog_area_sec_end;
static IAP_STATUS_CODE status;

//static uint8_t buffer_rd[BUFF_READ_SIZE];

/* Other -------------------------------------------------------------------- */
/* Function prototypes -------------------------------------------------------*/

/**  Get sector number of an address */
static uint32_t GetSecNum (uint32_t adr);
/**  Prepare sector(s) for write operation */
static IAP_STATUS_CODE PrepareSector(uint32_t start_sec, uint32_t end_sec);
/**  Copy RAM to Flash */
static IAP_STATUS_CODE CopyRAM2Flash(uint8_t * dest, uint8_t* source, IAP_WRITE_SIZE size);
/**  Prepare sector(s) for write operation */
static IAP_STATUS_CODE EraseSector(uint32_t start_sec, uint32_t end_sec);


/* Private functions -------------------------------------------------------- */


/*********************************************************************//**
 * @brief		Get Sector Number
 *
 * @param[in] adr	           Sector Address
 *
 * @return 	Sector Number.
 *
 **********************************************************************/
static uint32_t GetSecNum (uint32_t adr)
{
    uint32_t n;

    n = adr >> 12;                               //  4kB Sector
    if (n >= 0x10) {
      n = 0x0E + (n >> 3);                       // 32kB Sector
    } 

    return (n);                                  // Sector Number
}

/*********************************************************************//**
 * @brief		Prepare sector(s) for write operation
 *
 * @param[in] start_sec	          The number of start sector
 * @param[in] end_sec	          The number of end sector
 *
 * @return 	CMD_SUCCESS/BUSY/INVALID_SECTOR.
 *
 **********************************************************************/
static IAP_STATUS_CODE PrepareSector(uint32_t start_sec, uint32_t end_sec)
{
    IAP_COMMAND_Type command;
    command.cmd    = IAP_PREPARE;                    // Prepare Sector for Write
    command.param[0] = start_sec;                    // Start Sector
    command.param[1] = end_sec;                      // End Sector
    IAP_Call (&command.cmd, &command.status);        // Call IAP Command
    return (IAP_STATUS_CODE)command.status;
}

/*********************************************************************//**
 * @brief		 Copy RAM to Flash
 *
 * @param[in] dest	          destination buffer (in Flash memory).
 * @param[in] source	   source buffer (in RAM).
 * @param[in] size	          the write size.
 *
 * @return 	CMD_SUCCESS.
 *                  SRC_ADDR_ERROR/DST_ADDR_ERROR
 *                  SRC_ADDR_NOT_MAPPED/DST_ADDR_NOT_MAPPED
 *                  COUNT_ERROR/SECTOR_NOT_PREPARED_FOR_WRITE_OPERATION
 *                  BUSY
 *
 **********************************************************************/
static IAP_STATUS_CODE CopyRAM2Flash(uint8_t * dest, uint8_t* source, IAP_WRITE_SIZE size)
{
    uint32_t sec;
    IAP_STATUS_CODE status;
    IAP_COMMAND_Type command;

	// Prepare sectors
    sec = GetSecNum((uint32_t)dest);
   	status = PrepareSector(sec, sec);
	if(status != CMD_SUCCESS)
        return status;
   
	// write
	command.cmd    = IAP_COPY_RAM2FLASH;             // Copy RAM to Flash
    command.param[0] = (uint32_t)dest;                 // Destination Flash Address
    command.param[1] = (uint32_t)source;               // Source RAM Address
    command.param[2] =  size;                          // Number of bytes
    command.param[3] =  SystemCoreClock / 1000;         // CCLK in kHz
    IAP_Call (&command.cmd, &command.status);              // Call IAP Command
	  
    return (IAP_STATUS_CODE)command.status;             // Finished without Errors	  
}

/*********************************************************************//**
 * @brief		 Erase sector(s)
 *
 * @param[in] start_sec	   The number of start sector
 * @param[in] end_sec	   The number of end sector
 *
 * @return 	CMD_SUCCESS.
 *                  INVALID_SECTOR
 *                  SECTOR_NOT_PREPARED_FOR_WRITE_OPERATION
 *                  BUSY
 *
 **********************************************************************/
static IAP_STATUS_CODE EraseSector(uint32_t start_sec, uint32_t end_sec)
{
    IAP_COMMAND_Type command;
    IAP_STATUS_CODE status;

	// Prepare sectors
   	status = PrepareSector(start_sec, end_sec);
	if(status != CMD_SUCCESS)
        return status;

	// Erase sectors
    command.cmd    = IAP_ERASE;                    // Prepare Sector for Write
    command.param[0] = start_sec;                  // Start Sector
    command.param[1] = end_sec;                    // End Sector
    command.param[2] =  SystemCoreClock / 1000;         // CCLK in kHz
    IAP_Call (&command.cmd, &command.status);      // Call IAP Command
    return (IAP_STATUS_CODE)command.status;  
}


/* Public functions --------------------------------------------------------- */

void borrar_sector(void)
{
  /* Borrar sector 8 */
  status = EraseSector(flash_prog_area_sec_start, flash_prog_area_sec_end); 
  if(status != CMD_SUCCESS)
  {
     /* Erase chip failed */
     while(1); 
  }
}

void escribir_sector(void)
{
  /* Escribir las primeras 16 posiciones del sector 8 */
  status =  CopyRAM2Flash((uint8_t*) FLASH_PROG_AREA_START, buffer_wr, IAP_WRITE_256);
  if(status != CMD_SUCCESS)
  {
     /* Program chip failed */
     while(1);
  }
}

void modificar_byte(uint8_t posicion, uint8_t valor)
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

//void leer_sector(void)
//{
//  /* Leer las primeras 16 posiciones del sector 8 */
//  for (i = 0;i < sizeof(buffer_rd);i++)
//  {
//    buffer_rd[i] = *(uint8_t*)(FLASH_PROG_AREA_START + i);
//  }
//}

void escribir_FLASH_MAC_IP (void)
{
  uint8_t mac [] = {ETH0_MAC1, ETH0_MAC2, ETH0_MAC3, ETH0_MAC4, ETH0_MAC5, ETH0_MAC6};
  uint8_t ip [] = {ETH0_IP1, ETH0_IP2, ETH0_IP3, ETH0_IP4};
  
  flash_prog_area_sec_start = GetSecNum(FLASH_PROG_AREA_START);
  flash_prog_area_sec_end =  GetSecNum(FLASH_PROG_AREA_START + FLASH_PROG_AREA_SIZE - 1); /* selecciona solo el sector 8 */
  
  for (i = 0;i < sizeof(buffer_wr);i++)
  {
    buffer_wr[i] = *(uint8_t*)(FLASH_PROG_AREA_START + i);
  }
  
  borrar_sector();
  
  /* copia la mac en las primeras 6 posiciones */
  for (i = 0;i < LONGITUD_MAC;i++)
  {
    buffer_wr[i] = mac[i];
  }
  
  /* copia la ip en las primeras 6 posiciones */
  for (i = 0;i < LONGITUD_IP;i++)
  {
    buffer_wr[i + 6] = ip[i];
  }
  
  escribir_sector();
}

//int main (void)
//{
//  // Initialize
//  for (i = 0;i < sizeof(buffer_wr);i++)
//  {
//    if (i < 16)
//    {
//      buffer_wr[i] = (uint8_t)i;
//    }
//    else
//    {
//      buffer_wr[i] = 0;
//    }
//  }
//  
//  flash_prog_area_sec_start = GetSecNum(FLASH_PROG_AREA_START);
//  flash_prog_area_sec_end =  GetSecNum(FLASH_PROG_AREA_START + FLASH_PROG_AREA_SIZE - 1); /* selecciona solo el sector 8 */

//  borrar_sector();
//  
//  escribir_sector();
//  
//  leer_sector();
//  
//  modificar_byte(1, 0xea);
//  
//  while (1);
//}
