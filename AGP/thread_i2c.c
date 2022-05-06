/* Includes ------------------------------------------------------------------*/

#include "cmsis_os.h"                                           // CMSIS RTOS header file
#include "Driver_I2C.h"
#include "GPIO_LPC17xx.h" 
#include "PIN_LPC17xx.h"
#include "LPC17xx.h"

#include "thread_i2c.h"
#include "thread_medida.h"

/* Macros --------------------------------------------------------------------*/

#define SIG_TEMP              0x0001

#define PORT_INT 0
#define PIN_INT 24
#define INT_ON 1
#define INT_OFF !INT_ON

#define BYTE_RX_INT_MASK (uint8_t) (0x04)

#define PORT_SEL 0
#define PIN_SEL_0 17
#define PIN_SEL_1 15
#define PIN_SEL_2 16
#define SEL_ON 0
#define SEL_OFF !SEL_ON

/* Public variables ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* I2C driver instance */
extern ARM_DRIVER_I2C            Driver_I2C2;
static ARM_DRIVER_I2C *I2Cdrv = &Driver_I2C2;

static volatile uint32_t I2C_Event;

static uint8_t byte_rx [2];

static osThreadId tid_thread_i2c;

/* Function prototypes -------------------------------------------------------*/

static void thread_i2c (void const *argument);

/* Other -------------------------------------------------------------------- */

osThreadDef (thread_i2c, osPriorityNormal, 1, 0);

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
    
    osSignalSet (tid_thread_i2c, SIG_TEMP);

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

static void init_i2c(void)
{
  I2Cdrv->Initialize (I2C_SignalEvent_Slave);
  I2Cdrv->PowerControl (ARM_POWER_FULL);
  I2Cdrv->Control (ARM_I2C_OWN_ADDRESS, 0x28);
  I2Cdrv->Control (ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_STANDARD);
  I2Cdrv->Control (ARM_I2C_BUS_CLEAR, 0);
}

static void set_sel(uint8_t sel)
{
  uint32_t estado_sel;
  
  /* SEL0 */
  estado_sel = (sel & 0x01) ? SEL_ON : SEL_OFF;
  GPIO_PinWrite (PORT_SEL, PIN_SEL_0, estado_sel);
  
  /* SEL1 */
  estado_sel = (sel & 0x02) ? SEL_ON : SEL_OFF;
  GPIO_PinWrite (PORT_SEL, PIN_SEL_1, estado_sel);
  
  /* SEL2 */
  estado_sel = (sel & 0x04) ? SEL_ON : SEL_OFF;
  GPIO_PinWrite (PORT_SEL, PIN_SEL_2, estado_sel);
}

static void set_ganancia(uint8_t ganancia)
{
  uint8_t sel;
  
  switch (ganancia)
  {
    case 1:
      sel = 0x00;
      break;
    
    case 5:
      sel = 0x01;
      break;
    
    case 10:
      sel = 0x02;
      break;
    
    case 50:
      sel = 0x03;
      break;
    
    case 100:
      sel = 0x04;
      break;
  }
  
  set_sel(sel);
}

static void enviar_vo (void)
{
  uint8_t bytes_tx  [2];
  
  bytes_tx[0] = (uint8_t) (v_out >> 8);
  bytes_tx[1] = (uint8_t) v_out;
  
  I2Cdrv->SlaveTransmit(bytes_tx, 2);
  osSignalWait (SIG_TEMP, osWaitForever);
}

static void procesar_rx (void)
{
  switch (byte_rx[0])
  {
    case 0x00:
      set_ganancia(byte_rx[1]);
      break;
    
    case 0x01:
      overload_valor = byte_rx[1];
      break;
    
    case 0x02:
      overload_int_enable = byte_rx[1];
      break;
    
    case 0x03:
      enviar_vo();
      break;
  }
}

static void thread_i2c (void const *argument) {
  init_i2c();
  
  /* señales para seleccion de ganancia */
  GPIO_SetDir (PORT_SEL, PIN_SEL_0, GPIO_DIR_OUTPUT);
  GPIO_SetDir (PORT_SEL, PIN_SEL_1, GPIO_DIR_OUTPUT);
  GPIO_SetDir (PORT_SEL, PIN_SEL_2, GPIO_DIR_OUTPUT);
  
  /* inicializacion de ganancia a 1 */
  GPIO_PinWrite (PORT_SEL, PIN_SEL_0, SEL_OFF);
  GPIO_PinWrite (PORT_SEL, PIN_SEL_1, SEL_OFF);
  GPIO_PinWrite (PORT_SEL, PIN_SEL_2, SEL_OFF);

  while (1) {
    I2Cdrv->SlaveReceive(byte_rx, 2);
    osSignalWait (SIG_TEMP, osWaitForever);
    
    procesar_rx();
    
    osThreadYield ();
  }
}

/* Public functions --------------------------------------------------------- */

int init_thread_i2c (void)
{
  tid_thread_i2c = osThreadCreate (osThread(thread_i2c), NULL);
  
  if (!tid_thread_i2c)
  {
    return -1;
  }
  
  return(0);
}
