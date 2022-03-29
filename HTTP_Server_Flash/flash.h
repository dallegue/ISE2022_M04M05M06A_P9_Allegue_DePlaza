#ifndef _LPC17xx_IAP_H
#define _LPC17xx_IAP_H

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>

/* Macros ------------------------------------------------------------------- */

#define LONGITUD_MAC 6
#define LONGITUD_IP 4

#define FLASH_LEDS_ROTATORIO (uint8_t) 0


/* Public variables ----------------------------------------------------------*/
/* Public functions --------------------------------------------------------- */

void escribir_FLASH_MAC_IP (void);
uint8_t leer_FLASH_LEDS (void);
void escribir_FLASH_LEDS (uint8_t estado_leds);

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

#endif /*_LPC17xx_IAP_H*/
