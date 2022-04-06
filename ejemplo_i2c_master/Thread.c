

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/

/* Includes ------------------------------------------------------------------*/

#include "cmsis_os.h"                                           // CMSIS RTOS header file
#include "Driver_I2C.h"
#include "GPIO_LPC17xx.h" 
#include "PIN_LPC17xx.h"
#include "LPC17xx.h"

/* Macros --------------------------------------------------------------------*/

#define LPC_SLAVE_I2C_ADDR         0x28

#define SIG_TEMP              0x0001
#define SIG_LED_BLUE_OFF      0x0002

#define PORT_INT 0
#define PIN_INT 23

#define PORT_RGB_LEDS 2
#define PIN_RGB_LED_RED 3
#define PIN_RGB_LED_GREEN 2
#define PIN_RGB_LED_BLUE 1
#define RGB_LED_ON 0
#define RGB_LED_OFF !RGB_LED_ON

/* Public variables ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

static int32_t status = 0;

/* I2C driver instance */
extern ARM_DRIVER_I2C            Driver_I2C2;
static ARM_DRIVER_I2C *I2Cdrv = &Driver_I2C2;

static volatile uint32_t I2C_Event;

/* Other -------------------------------------------------------------------- */
/* Function prototypes -------------------------------------------------------*/

void Thread (void const *argument);                             // thread function
osThreadId tid_Thread;                                          // thread id
osThreadDef (Thread, osPriorityNormal, 1, 0);                   // thread object

void thread_led_blue_off (void const *argument);                             // thread function
osThreadId tid_thread_led_blue_off;                                          // thread id
osThreadDef (thread_led_blue_off, osPriorityNormal, 1, 0);                   // thread object

/* Private functions -------------------------------------------------------- */

/* I2C Signal Event function callback */
static void I2C_SignalEvent (uint32_t event) {
 
  /* Save received events */
  I2C_Event |= event;
 
  /* Optionally, user can define specific actions for an event */
 
  if (event & ARM_I2C_EVENT_TRANSFER_INCOMPLETE) {
    /* Less data was transferred than requested */
  }
 
  if (event & ARM_I2C_EVENT_TRANSFER_DONE) {
    /* Transfer or receive is finished */
    
    osSignalSet (tid_Thread, SIG_TEMP);

  }
 
  if (event & ARM_I2C_EVENT_ADDRESS_NACK) {
    /* Slave address was not acknowledged */
  }
 
  if (event & ARM_I2C_EVENT_ARBITRATION_LOST) {
    /* Master lost bus arbitration */
  }
 
  if (event & ARM_I2C_EVENT_BUS_ERROR) {
    /* Invalid start/stop position detected */
  }
 
  if (event & ARM_I2C_EVENT_BUS_CLEAR) {
    /* Bus clear operation completed */
  }
 
  if (event & ARM_I2C_EVENT_GENERAL_CALL) {
    /* Slave was addressed with a general call address */
  }
 
  if (event & ARM_I2C_EVENT_SLAVE_RECEIVE) {
    /* Slave addressed as receiver but SlaveReceive operation is not started */
  }
 
  if (event & ARM_I2C_EVENT_SLAVE_TRANSMIT) {
    /* Slave addressed as transmitter but SlaveTransmit operation is not started */
  }
}

static void pin_int_config(void)
{
  /* Configurar pines de sw como pulldown */
  PIN_Configure(PORT_INT,PIN_INT,PIN_FUNC_0,PIN_PINMODE_PULLDOWN,PIN_PINMODE_NORMAL);

  /* Habilitar interrupciones por flanco de subida */
  LPC_GPIOINT->IO0IntEnR = 1 << PIN_INT;
  
  /* Habilitar interrupciones de EINT3 (interrupciones de GPIO puertos 0 y 2) */
  NVIC_EnableIRQ(EINT3_IRQn);
}

static void rgb_leds_config(void)
{
    /* Configura pines como salida */
    GPIO_SetDir (PORT_RGB_LEDS, PIN_RGB_LED_RED, GPIO_DIR_OUTPUT);
    GPIO_SetDir (PORT_RGB_LEDS, PIN_RGB_LED_GREEN, GPIO_DIR_OUTPUT); 
    GPIO_SetDir (PORT_RGB_LEDS, PIN_RGB_LED_BLUE, GPIO_DIR_OUTPUT);
  
    GPIO_PinWrite (PORT_RGB_LEDS, PIN_RGB_LED_RED, RGB_LED_OFF);
    GPIO_PinWrite (PORT_RGB_LEDS, PIN_RGB_LED_GREEN, RGB_LED_OFF);
    GPIO_PinWrite (PORT_RGB_LEDS, PIN_RGB_LED_BLUE, RGB_LED_OFF);
}

static void comprobar_byte(uint8_t byte_tx, uint8_t byte_rx)
{
  if (byte_rx == (uint8_t) ~byte_tx)
  {
    GPIO_PinWrite (PORT_RGB_LEDS, PIN_RGB_LED_GREEN, RGB_LED_ON);
    GPIO_PinWrite (PORT_RGB_LEDS, PIN_RGB_LED_RED, RGB_LED_OFF);
  }
  else
  {
    GPIO_PinWrite (PORT_RGB_LEDS, PIN_RGB_LED_GREEN, RGB_LED_OFF);
    GPIO_PinWrite (PORT_RGB_LEDS, PIN_RGB_LED_RED, RGB_LED_ON);
  }
}

/* Public functions --------------------------------------------------------- */

void EINT3_IRQHandler (void)
{
  if (LPC_GPIOINT->IO0IntStatR & 1 << PIN_INT)
  {
    /* Encender led azul */
    GPIO_PinWrite (PORT_RGB_LEDS, PIN_RGB_LED_BLUE, RGB_LED_ON);
    
    /* Arrancar "timer virtual" de 1 segundo */
    osSignalSet (tid_thread_led_blue_off, SIG_LED_BLUE_OFF);
  
    /* Borra el flag de la interrupcion */
    LPC_GPIOINT->IO0IntClr = 1 << PIN_INT;
  }
}

void Init_i2c(void){
  
  int32_t status;
  
  status = I2Cdrv->Initialize (I2C_SignalEvent);
  status = I2Cdrv->PowerControl (ARM_POWER_FULL);
  status = I2Cdrv->Control      (ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_STANDARD);
  status = I2Cdrv->Control      (ARM_I2C_BUS_CLEAR, 0);
  
}

int Init_Thread (void) {

  tid_Thread = osThreadCreate (osThread(Thread), NULL);
  if (!tid_Thread) return(-1);
  
  return(0);
}

void thread_led_blue_off (void const *argument)
{
  
  while(1)
  {
    osSignalWait (SIG_LED_BLUE_OFF, osWaitForever);
    
    osDelay(1000);
    
    /* Apagar led azul */
    GPIO_PinWrite (PORT_RGB_LEDS, PIN_RGB_LED_BLUE, RGB_LED_OFF);
  }
}

void Thread (void const *argument) {
  uint8_t byte_tx = 0;
  uint8_t byte_rx;
  
  rgb_leds_config();
  pin_int_config();
  
  tid_thread_led_blue_off = osThreadCreate(osThread (thread_led_blue_off), NULL);

  while (1) {
    osDelay(3000);
    
    status = I2Cdrv->MasterTransmit(LPC_SLAVE_I2C_ADDR, &byte_tx, 1, true);
    osSignalWait (SIG_TEMP, osWaitForever); 
    
    /* delay? */
    //osDelay(100);
    
    status = I2Cdrv->MasterReceive(LPC_SLAVE_I2C_ADDR, &byte_rx, 1, true);
    osSignalWait (SIG_TEMP, osWaitForever); 
    
    comprobar_byte(byte_tx, byte_rx);
    
    byte_tx++;
    
    osThreadYield ();                                           // suspend thread
  }
}
