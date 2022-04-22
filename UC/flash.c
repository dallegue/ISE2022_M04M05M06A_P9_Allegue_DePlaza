/* Includes ------------------------------------------------------------------*/

#include <string.h>

#include "flash.h"
#include "system_lpc17xx.h"
#include "lpc17xx.h"
#include "Net_Config_ETH_0.h"
#include "thread_hora.h"

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
#define IAP_Call   iap_entry_flash_c

/** The area will be erase and program */
/* Sector 20 (0x14), es de 32kB */
#define FLASH_PROG_AREA_START       0x00030000
#define FLASH_PROG_AREA_SIZE        0x00008000

/** The origin buffer_wr on RAM, el minimo num de bytes que 
    pueden escribirse es 256 */
#define BUFF_SIZE           256

#define GANANCIA_OFFSET 0
#define OVERLOAD_OFFSET 17

/* Public variables ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

static uint32_t i;
static uint32_t flash_prog_area_sec_start;
static uint32_t flash_prog_area_sec_end;
static uint8_t __attribute__ ((aligned (4))) buffer_wr[BUFF_SIZE];

/* Other -------------------------------------------------------------------- */


/**
 * @brief IAP command code definitions
 */
typedef enum
{
    IAP_PREPARE = 50,       // Prepare sector(s) for write operation
    IAP_COPY_RAM2FLASH = 51,     // Copy RAM to Flash
    IAP_ERASE = 52,              // Erase sector(s)
    IAP_BLANK_CHECK = 53,        // Blank check sector(s)
    IAP_READ_PART_ID = 54,       // Read chip part ID
    IAP_READ_BOOT_VER = 55,      // Read chip boot code version
    IAP_COMPARE = 56,            // Compare memory areas
    IAP_REINVOKE_ISP = 57,       // Reinvoke ISP
    IAP_READ_SERIAL_NUMBER = 58, // Read serial number
}  IAP_COMMAND_CODE;

/**
 * @brief IAP status code definitions
 */
typedef enum
{
    CMD_SUCCESS,	             // Command is executed successfully.
    INVALID_COMMAND,             // Invalid command.
    SRC_ADDR_ERROR,              // Source address is not on a word boundary.
    DST_ADDR_ERROR,              // Destination address is not on a correct boundary.
    SRC_ADDR_NOT_MAPPED,         // Source address is not mapped in the memory map.
    DST_ADDR_NOT_MAPPED,         // Destination address is not mapped in the memory map.
    COUNT_ERROR,	               // Byte count is not multiple of 4 or is not a permitted value.
    INVALID_SECTOR,	           // Sector number is invalid.
    SECTOR_NOT_BLANK,	           // Sector is not blank.
    SECTOR_NOT_PREPARED_FOR_WRITE_OPERATION,	// Command to prepare sector for write operation was not executed.
    COMPARE_ERROR,               // Source and destination data is not same.
    BUSY,		                   // Flash programming hardware interface is busy.
} IAP_STATUS_CODE;

/**
 * @brief IAP write length definitions
 */
typedef enum {
  IAP_WRITE_256  = 256,
  IAP_WRITE_512  = 512,
  IAP_WRITE_1024 = 1024,
  IAP_WRITE_4096 = 4096,
} IAP_WRITE_SIZE;

/**
 * @brief IAP command structure
 */
typedef struct {
    uint32_t cmd;   // Command
    uint32_t param[4];      // Parameters
    uint32_t status;        // status code
    uint32_t result[4];     // Result
} IAP_COMMAND_Type;

/* Function prototypes -------------------------------------------------------*/
/* Private functions -------------------------------------------------------- */


/*********************************************************************//**
 * @brief    Get Sector Number
 *
 * @param[in] adr             Sector Address
 *
 * @return   Sector Number.
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
 * @brief    Prepare sector(s) for write operation
 *
 * @param[in] start_sec            The number of start sector
 * @param[in] end_sec            The number of end sector
 *
 * @return   CMD_SUCCESS/BUSY/INVALID_SECTOR.
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
 * @brief     Copy RAM to Flash
 *
 * @param[in] dest            destination buffer (in Flash memory).
 * @param[in] source     source buffer (in RAM).
 * @param[in] size            the write size.
 *
 * @return   CMD_SUCCESS.
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
 * @brief     Erase sector(s)
 *
 * @param[in] start_sec     The number of start sector
 * @param[in] end_sec     The number of end sector
 *
 * @return   CMD_SUCCESS.
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

static void borrar_sector(void)
{
  IAP_STATUS_CODE status;
  status = EraseSector(flash_prog_area_sec_start, flash_prog_area_sec_end); 
  while(status != CMD_SUCCESS);
}

static void escribir_sector(void)
{
  IAP_STATUS_CODE status;
  status =  CopyRAM2Flash((uint8_t*) FLASH_PROG_AREA_START, buffer_wr, IAP_WRITE_256);
  while(status != CMD_SUCCESS);
}

static void escribir_FLASH_timestamp (uint8_t addr_offset)
{
 /* lee los bytes que hay en la flash y los guarda en buffer_wr */
 for (i = 0;i < sizeof(buffer_wr);i++)
 {
   buffer_wr[i] = *(uint8_t*)(FLASH_PROG_AREA_START + i);
 }
 
 borrar_sector();
 
 /* copia la mac en las primeras 17 posiciones del buffer */
 for (i = 0;i < strlen(lineaFechaHora);i++)
 {
   buffer_wr[i + addr_offset] = lineaFechaHora[i];
 }
 
 escribir_sector();
}

/* Public functions --------------------------------------------------------- */

void escribir_FLASH_timestamp_ganancia ()
{
  escribir_FLASH_timestamp(GANANCIA_OFFSET);
}

void escribir_FLASH_timestamp_overload ()
{
  escribir_FLASH_timestamp(OVERLOAD_OFFSET);
}

void init_FLASH ()
{
  flash_prog_area_sec_start = GetSecNum(FLASH_PROG_AREA_START);
  flash_prog_area_sec_end =  GetSecNum(FLASH_PROG_AREA_START + FLASH_PROG_AREA_SIZE - 1);
}
