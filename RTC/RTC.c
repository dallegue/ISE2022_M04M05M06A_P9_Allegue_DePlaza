/* Includes ------------------------------------------------------------------*/

#include <stdint.h>
#include "RTC.h"

/* Macros --------------------------------------------------------------------*/

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

bool minuto_incrementado = false;

/* Private variables ---------------------------------------------------------*/

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

static void RTC_ResetClockTickCounter(void);
static void RTC_Init (void);
static void RTC_SetTime (uint32_t Timetype, uint32_t TimeValue);


/* Public functions --------------------------------------------------------- */

/* ISR */
void RTC_IRQHandler(void)
{
  /* This is increment counter interrupt*/
  if (LPC_RTC->ILR & RTC_IRL_RTCCIF)
  {
    minuto_incrementado = true;
    
    // Clear pending interrupt
    LPC_RTC->ILR |= RTC_IRL_RTCCIF;
  }
}

/*********************************************************************//**
 * @brief 		Get full of time in RTC peripheral
 * @param[in]	RTCx	RTC peripheral selected, should be LPC_RTC
 * @param[in]	pFullTime Pointer to a RTC_TIME_Type structure that
 * 				will be stored time in full.
 * @return 		None
 **********************************************************************/
void RTC_GetFullTime (RTC_TIME_Type *pFullTime)
{
	pFullTime->DOM = LPC_RTC->DOM & RTC_DOM_MASK;
	pFullTime->DOW = LPC_RTC->DOW & RTC_DOW_MASK;
	pFullTime->DOY = LPC_RTC->DOY & RTC_DOY_MASK;
	pFullTime->HOUR = LPC_RTC->HOUR & RTC_HOUR_MASK;
	pFullTime->MIN = LPC_RTC->MIN & RTC_MIN_MASK;
	pFullTime->SEC = LPC_RTC->SEC & RTC_SEC_MASK;
	pFullTime->MONTH = LPC_RTC->MONTH & RTC_MONTH_MASK;
	pFullTime->YEAR = LPC_RTC->YEAR & RTC_YEAR_MASK;
}

void RTC_startup(RTC_TIME_Type *pFullTime)
{
  /* RTC Block section ------------------------------------------------------ */
  // Init RTC module
  RTC_Init();

  /* Disable RTC interrupt */
  NVIC_DisableIRQ(RTC_IRQn);

  /* Enable rtc (starts increase the tick counter and second counter register) */
  RTC_ResetClockTickCounter();
  LPC_RTC->CCR |= RTC_CCR_CLKEN;
  LPC_RTC->CCR |= RTC_CCR_CCALEN;

  /* Set current time for RTC */
  // Current time is 8:00:00PM, 2009-04-24
  RTC_SetTime (RTC_TIMETYPE_SECOND, pFullTime->SEC);
  RTC_SetTime (RTC_TIMETYPE_MINUTE, pFullTime->MIN);
  RTC_SetTime (RTC_TIMETYPE_HOUR, pFullTime->HOUR);
  RTC_SetTime (RTC_TIMETYPE_MONTH, pFullTime->MONTH);
  RTC_SetTime (RTC_TIMETYPE_YEAR, pFullTime->YEAR);
  RTC_SetTime (RTC_TIMETYPE_DAYOFMONTH, pFullTime->DOM);

  /* Set the CIIR for minute counter interrupt */
  LPC_RTC->CIIR |= RTC_CIIR_IMMIN;

  /* Enable RTC interrupt */
  NVIC_EnableIRQ(RTC_IRQn);
  
  /* Configurar led para parpadeo */
  GPIO_SetDir (PORT_LEDS, PIN_LED4, GPIO_DIR_OUTPUT);
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
static void RTC_SetTime (uint32_t Timetype, uint32_t TimeValue)
{
	switch ( Timetype)
	{
	case RTC_TIMETYPE_SECOND:
		LPC_RTC->SEC = TimeValue & RTC_SEC_MASK;
		break;

	case RTC_TIMETYPE_MINUTE:
		LPC_RTC->MIN = TimeValue & RTC_MIN_MASK;
		break;

	case RTC_TIMETYPE_HOUR:
		LPC_RTC->HOUR = TimeValue & RTC_HOUR_MASK;
		break;

	case RTC_TIMETYPE_DAYOFWEEK:
		LPC_RTC->DOW = TimeValue & RTC_DOW_MASK;
		break;

	case RTC_TIMETYPE_DAYOFMONTH:
		LPC_RTC->DOM = TimeValue & RTC_DOM_MASK;
		break;

	case RTC_TIMETYPE_DAYOFYEAR:
		LPC_RTC->DOY = TimeValue & RTC_DOY_MASK;
		break;

	case RTC_TIMETYPE_MONTH:
		LPC_RTC->MONTH = TimeValue & RTC_MONTH_MASK;
		break;

	case RTC_TIMETYPE_YEAR:
		LPC_RTC->YEAR = TimeValue & RTC_YEAR_MASK;
		break;
	}
}

/********************************************************************//**
 * @brief		Initializes the RTC peripheral.
 * @return 		None
 *********************************************************************/
static void RTC_Init (void)
{
	/* Set up clock and power for RTC module */
  LPC_SC->PCONP |= CLKPWR_PCONP_PCRTC & CLKPWR_PCONP_BITMASK;

	// Clear all register to be default
	LPC_RTC->ILR = 0x00;
	LPC_RTC->CCR = 0x00;
	LPC_RTC->CIIR = 0x00;
	LPC_RTC->AMR = 0xFF;
	LPC_RTC->CALIBRATION = 0x00;
}

/*********************************************************************//**
 * @brief 		Reset clock tick counter in RTC peripheral
 * @return 		None
 **********************************************************************/
static void RTC_ResetClockTickCounter(void)
{
	LPC_RTC->CCR |= RTC_CCR_CTCRST;
	LPC_RTC->CCR &= (~RTC_CCR_CTCRST) & RTC_CCR_BITMASK;
}
