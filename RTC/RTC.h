#ifndef __RTC_H
#define __RTC_H

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>

/* Macros ------------------------------------------------------------------- */
/* Public variables ----------------------------------------------------------*/

/** @brief Time structure definitions for easy manipulate the data */
typedef struct {
	uint32_t SEC; 		/*!< Seconds Register */
	uint32_t MIN; 		/*!< Minutes Register */
	uint32_t HOUR; 		/*!< Hours Register */
	uint32_t DOM;		/*!< Day of Month Register */
	uint32_t DOW; 		/*!< Day of Week Register */
	uint32_t DOY; 		/*!< Day of Year Register */
	uint32_t MONTH; 	/*!< Months Register */
	uint32_t YEAR; 		/*!< Years Register */
} RTC_TIME_Type;

/* Public functions --------------------------------------------------------- */
/* Other -------------------------------------------------------------------- */

#endif
