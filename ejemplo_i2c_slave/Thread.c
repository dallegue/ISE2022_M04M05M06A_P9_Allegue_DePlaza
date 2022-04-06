/* Includes ------------------------------------------------------------------*/

#include "cmsis_os.h"                                           // CMSIS RTOS header file
#include "Driver_I2C.h"
#include "GPIO_LPC17xx.h" 
#include "PIN_LPC17xx.h"
#include "LPC17xx.h"

/* Macros --------------------------------------------------------------------*/

#define SIG_TEMP              0x0001

#define PORT_INT 0
#define PIN_INT 24
#define INT_ON 1
#define INT_OFF !INT_ON

#define BYTE_RX_INT_MASK (uint8_t) (0x04)

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

/* Private functions -------------------------------------------------------- */
 
/* I2C Signal Event function callback */
static void I2C_SignalEvent_Slave (uint32_t event) {
 
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

/* Public functions --------------------------------------------------------- */

void Init_i2c(void){
  
  int32_t status;
  
  status = I2Cdrv->Initialize (I2C_SignalEvent_Slave);
  status = I2Cdrv->PowerControl (ARM_POWER_FULL);
  status = I2Cdrv->Control (ARM_I2C_OWN_ADDRESS, 0x28);
  status = I2Cdrv->Control (ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_STANDARD);
  status = I2Cdrv->Control (ARM_I2C_BUS_CLEAR, 0);
  
}

int Init_Thread (void) {

  tid_Thread = osThreadCreate (osThread(Thread), NULL);
  if (!tid_Thread) return(-1);
  
  return(0);
}

static void comprobar_byte(uint8_t byte_rx)
{
  /* Clear linea int */
  GPIO_PinWrite (PORT_INT, PIN_INT, INT_OFF);
  
  if (byte_rx & BYTE_RX_INT_MASK)
  {
    GPIO_PinWrite (PORT_INT, PIN_INT, INT_ON);
  }
}

void Thread (void const *argument) {
  uint8_t byte_rx;
  uint8_t byte_tx;
  
  GPIO_SetDir (PORT_INT, PIN_INT, GPIO_DIR_OUTPUT);

  while (1) {
    I2Cdrv->SlaveReceive(&byte_rx, 1);
    osSignalWait (SIG_TEMP, osWaitForever);
    
    byte_tx = ~byte_rx;
    
    comprobar_byte(byte_rx);
    
    I2Cdrv->SlaveTransmit(&byte_tx, 1);
    osSignalWait (SIG_TEMP, osWaitForever);
    
    osThreadYield ();                                           // suspend thread
  }
}
