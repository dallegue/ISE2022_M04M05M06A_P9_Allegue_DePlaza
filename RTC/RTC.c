/* Includes ------------------------------------------------------------------*/

#include <stdio.h>
#include <stdint.h>
//#include "lpc17xx_rtc.h"
#include "lcd.h"
#include "cmsis_os.h"                   /* CMSIS RTOS definitions             */
#include "GPIO_LPC17xx.h"
#include "RTC.h"
#include <stdbool.h>

/* Macros --------------------------------------------------------------------*/

#define PORT_LEDS 1
#define PIN_LED4 23
#define LED_ON 1
#define LED_OFF 0
#define DURACION_PARPADEO_MS 5000
#define PERIODO_REFRESCO_MS 250

/** SEC register mask */
#define RTC_SEC_MASK			(0x0000003F)
/** MIN register mask */
#define RTC_MIN_MASK			(0x0000003F)
/** HOUR register mask */
#define RTC_HOUR_MASK			(0x0000001F)
/** DOM register mask */
#define RTC_DOM_MASK			(0x0000001F)
/** DOW register mask */
#define RTC_DOW_MASK			(0x00000007)
/** DOY register mask */
#define RTC_DOY_MASK			(0x000001FF)
/** MONTH register mask */
#define RTC_MONTH_MASK			(0x0000000F)
/** YEAR register mask */
#define RTC_YEAR_MASK			(0x00000FFF)

#define RTC_SECOND_MAX		59 /*!< Maximum value of second */
#define RTC_MINUTE_MAX		59 /*!< Maximum value of minute*/
#define RTC_HOUR_MAX		23 /*!< Maximum value of hour*/
#define RTC_MONTH_MIN		1 /*!< Minimum value of month*/
#define RTC_MONTH_MAX		12 /*!< Maximum value of month*/
#define RTC_DAYOFMONTH_MIN 	1 /*!< Minimum value of day of month*/
#define RTC_DAYOFMONTH_MAX 	31 /*!< Maximum value of day of month*/
#define RTC_DAYOFWEEK_MAX	6 /*!< Maximum value of day of week*/
#define RTC_DAYOFYEAR_MIN	1 /*!< Minimum value of day of year*/
#define RTC_DAYOFYEAR_MAX	366 /*!< Maximum value of day of year*/
#define RTC_YEAR_MAX		4095 /*!< Maximum value of year*/

/** The RTC power/clock control bit */
#define	 CLKPWR_PCONP_PCRTC  	((uint32_t)(1<<9))

/** Power Control for Peripherals bit mask */
#define CLKPWR_PCONP_BITMASK	0xEFEFF7DE

/** Clock reset */
#define RTC_CCR_CTCRST			((1<<1))

/** CCR register mask */
#define RTC_CCR_BITMASK			((0x00000013))

/** Clock enable */
#define RTC_CCR_CLKEN			((1<<0))

/** Calibration counter enable */
#define RTC_CCR_CCALEN			((1<<4))

/** Counter Increment Interrupt bit for minute */
#define RTC_CIIR_IMMIN			((1<<1))

/** Bit inform the source interrupt is counter increment*/
#define RTC_IRL_RTCCIF			((1<<0))

/* Public variables ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

static uint16_t cuenta = 0;
static uint8_t estado_led = LED_OFF;
static bool parpadear_led = false;

/** @brief RTC time type option */
typedef enum {
	RTC_TIMETYPE_SECOND = 0, 		/*!< Second */
	RTC_TIMETYPE_MINUTE = 1, 		/*!< Month */
	RTC_TIMETYPE_HOUR = 2, 			/*!< Hour */
	RTC_TIMETYPE_DAYOFWEEK = 3, 	/*!< Day of week */
	RTC_TIMETYPE_DAYOFMONTH = 4, 	/*!< Day of month */
	RTC_TIMETYPE_DAYOFYEAR = 5, 	/*!< Day of year */
	RTC_TIMETYPE_MONTH = 6, 		/*!< Month */
	RTC_TIMETYPE_YEAR = 7 			/*!< Year */
} RTC_TIMETYPE_Num;

/* Function prototypes -------------------------------------------------------*/

void RTC_IRQHandler(void);
static void RTC_startup(void);
void RTC_GetFullTime (LPC_RTC_TypeDef *RTCx, RTC_TIME_Type *pFullTime);


/* Public functions --------------------------------------------------------- */

/* ISR */
void RTC_IRQHandler(void)
{
  /* This is increment counter interrupt*/
  if (LPC_RTC->ILR & RTC_IRL_RTCCIF)
  {
    parpadear_led = true;
    
    // Clear pending interrupt
    LPC_RTC->ILR |= RTC_IRL_RTCCIF;
  }
}

int main (void)
{
  RTC_TIME_Type RTCFullTime;
  char lineaHora[20];
  char lineaFecha[20];
  
  GPIO_SetDir (PORT_LEDS, PIN_LED4, GPIO_DIR_OUTPUT); 
  RTC_startup();
  LCD_SPI_startup();
  
  /* Loop forever */
  while(1)
  {
    // Get and print current time
    RTC_GetFullTime (LPC_RTC, &RTCFullTime);
    sprintf(lineaHora, "%02d:%02d:%02d", RTCFullTime.HOUR, RTCFullTime.MIN, RTCFullTime.SEC);
    sprintf(lineaFecha, "%02d/%02d/%02d", RTCFullTime.DOM, RTCFullTime.MONTH, RTCFullTime.YEAR);
    print_lineas(lineaHora, lineaFecha);
    osDelay(PERIODO_REFRESCO_MS);
    
    if (parpadear_led)
    {
      estado_led = (estado_led == LED_OFF) ? LED_ON : LED_OFF;
      GPIO_PinWrite (PORT_LEDS, PIN_LED4, estado_led);
      
      if (++cuenta == DURACION_PARPADEO_MS/PERIODO_REFRESCO_MS)
      {
        cuenta = 0;
        parpadear_led = false;
      }
    }
    
    osThreadYield ();
  }
}

/*********************************************************************//**
 * @brief 		Get full of time in RTC peripheral
 * @param[in]	RTCx	RTC peripheral selected, should be LPC_RTC
 * @param[in]	pFullTime Pointer to a RTC_TIME_Type structure that
 * 				will be stored time in full.
 * @return 		None
 **********************************************************************/
void RTC_GetFullTime (LPC_RTC_TypeDef *RTCx, RTC_TIME_Type *pFullTime)
{
	pFullTime->DOM = RTCx->DOM & RTC_DOM_MASK;
	pFullTime->DOW = RTCx->DOW & RTC_DOW_MASK;
	pFullTime->DOY = RTCx->DOY & RTC_DOY_MASK;
	pFullTime->HOUR = RTCx->HOUR & RTC_HOUR_MASK;
	pFullTime->MIN = RTCx->MIN & RTC_MIN_MASK;
	pFullTime->SEC = RTCx->SEC & RTC_SEC_MASK;
	pFullTime->MONTH = RTCx->MONTH & RTC_MONTH_MASK;
	pFullTime->YEAR = RTCx->YEAR & RTC_YEAR_MASK;
}

/* Private functions -------------------------------------------------------- */

/*********************************************************************//**
 * @brief 		Set current time value for each time type in RTC peripheral
 * @param[in]	RTCx	RTC peripheral selected, should be LPC_RTC
 * @param[in]	Timetype: Time Type, should be:
 * 				- RTC_TIMETYPE_SECOND
 * 				- RTC_TIMETYPE_MINUTE
 * 				- RTC_TIMETYPE_HOUR
 * 				- RTC_TIMETYPE_DAYOFWEEK
 * 				- RTC_TIMETYPE_DAYOFMONTH
 * 				- RTC_TIMETYPE_DAYOFYEAR
 * 				- RTC_TIMETYPE_MONTH
 * 				- RTC_TIMETYPE_YEAR
 * @param[in]	TimeValue Time value to set
 * @return 		None
 **********************************************************************/
static void RTC_SetTime (LPC_RTC_TypeDef *RTCx, uint32_t Timetype, uint32_t TimeValue)
{
	switch ( Timetype)
	{
	case RTC_TIMETYPE_SECOND:
		RTCx->SEC = TimeValue & RTC_SEC_MASK;
		break;

	case RTC_TIMETYPE_MINUTE:
		RTCx->MIN = TimeValue & RTC_MIN_MASK;
		break;

	case RTC_TIMETYPE_HOUR:
		RTCx->HOUR = TimeValue & RTC_HOUR_MASK;
		break;

	case RTC_TIMETYPE_DAYOFWEEK:
		RTCx->DOW = TimeValue & RTC_DOW_MASK;
		break;

	case RTC_TIMETYPE_DAYOFMONTH:
		RTCx->DOM = TimeValue & RTC_DOM_MASK;
		break;

	case RTC_TIMETYPE_DAYOFYEAR:
		RTCx->DOY = TimeValue & RTC_DOY_MASK;
		break;

	case RTC_TIMETYPE_MONTH:
		RTCx->MONTH = TimeValue & RTC_MONTH_MASK;
		break;

	case RTC_TIMETYPE_YEAR:
		RTCx->YEAR = TimeValue & RTC_YEAR_MASK;
		break;
	}
}

/********************************************************************//**
 * @brief		Initializes the RTC peripheral.
 * @param[in]	RTCx	RTC peripheral selected, should be LPC_RTC
 * @return 		None
 *********************************************************************/
static void RTC_Init (LPC_RTC_TypeDef *RTCx)
{
	/* Set up clock and power for RTC module */
  LPC_SC->PCONP |= CLKPWR_PCONP_PCRTC & CLKPWR_PCONP_BITMASK;

	// Clear all register to be default
	RTCx->ILR = 0x00;
	RTCx->CCR = 0x00;
	RTCx->CIIR = 0x00;
	RTCx->AMR = 0xFF;
	RTCx->CALIBRATION = 0x00;
}

/*********************************************************************//**
 * @brief 		Reset clock tick counter in RTC peripheral
 * @param[in]	RTCx	RTC peripheral selected, should be LPC_RTC
 * @return 		None
 **********************************************************************/
static void RTC_ResetClockTickCounter(LPC_RTC_TypeDef *RTCx)
{
	RTCx->CCR |= RTC_CCR_CTCRST;
	RTCx->CCR &= (~RTC_CCR_CTCRST) & RTC_CCR_BITMASK;
}

static void RTC_startup(void)
{
  /* RTC Block section ------------------------------------------------------ */
  // Init RTC module
  RTC_Init(LPC_RTC);

  /* Disable RTC interrupt */
  NVIC_DisableIRQ(RTC_IRQn);

  /* Enable rtc (starts increase the tick counter and second counter register) */
  RTC_ResetClockTickCounter(LPC_RTC);
  LPC_RTC->CCR |= RTC_CCR_CLKEN;
  LPC_RTC->CCR |= RTC_CCR_CCALEN;

  /* Set current time for RTC */
  // Current time is 8:00:00PM, 2009-04-24
  RTC_SetTime (LPC_RTC, RTC_TIMETYPE_SECOND, 0);
  RTC_SetTime (LPC_RTC, RTC_TIMETYPE_MINUTE, 0);
  RTC_SetTime (LPC_RTC, RTC_TIMETYPE_HOUR, 16);
  RTC_SetTime (LPC_RTC, RTC_TIMETYPE_MONTH, 3);
  RTC_SetTime (LPC_RTC, RTC_TIMETYPE_YEAR, 2022);
  RTC_SetTime (LPC_RTC, RTC_TIMETYPE_DAYOFMONTH, 24);

  /* Set the CIIR for minute counter interrupt */
  LPC_RTC->CIIR |= RTC_CIIR_IMMIN;

  /* Enable RTC interrupt */
  NVIC_EnableIRQ(RTC_IRQn);
}
