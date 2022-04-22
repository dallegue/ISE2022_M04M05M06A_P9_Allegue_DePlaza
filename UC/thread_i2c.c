/* Includes ------------------------------------------------------------------*/

#include "cmsis_os.h"                                           // CMSIS RTOS header file
#include "Driver_I2C.h"
#include "GPIO_LPC17xx.h" 
#include "PIN_LPC17xx.h"
#include "LPC17xx.h"

#include "thread_i2c.h"
#include "HTTP_Server.h"
#include "flash.h"

/* Macros --------------------------------------------------------------------*/

#define LPC_SLAVE_I2C_ADDR         0x28

#define SIG_TEMP              0x0001
#define SIG_OVERLOAD_OFF      0x0002

#define PORT_INT 0
#define PIN_INT 23

/* Public variables ----------------------------------------------------------*/

osMessageQId queue_i2c_id;
osThreadId tid_thread_i2c;

/* Private variables ---------------------------------------------------------*/

static int32_t status = 0;

/* I2C driver instance */
extern ARM_DRIVER_I2C            Driver_I2C2;
static ARM_DRIVER_I2C *I2Cdrv = &Driver_I2C2;

static volatile uint32_t I2C_Event;

/* Other -------------------------------------------------------------------- */

/* Function prototypes -------------------------------------------------------*/

void thread_overload_off (void const *argument);                             // thread function
osThreadId tid_thread_overload_off;                                          // thread id
osThreadDef (thread_overload_off, osPriorityNormal, 1, 0);                   // thread object

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

static void pin_int_config(void)
{
  /* Configurar pines de sw como pulldown */
  PIN_Configure(PORT_INT,PIN_INT,PIN_FUNC_0,PIN_PINMODE_PULLDOWN,PIN_PINMODE_NORMAL);

  /* Habilitar interrupciones por flanco de subida */
  LPC_GPIOINT->IO0IntEnR = 1 << PIN_INT;
  
  /* Habilitar interrupciones de EINT3 (interrupciones de GPIO puertos 0 y 2) */
  NVIC_EnableIRQ(EINT3_IRQn);
}

static void init_i2c(void){
  status = I2Cdrv->Initialize (I2C_SignalEvent);
  status = I2Cdrv->PowerControl (ARM_POWER_FULL);
  status = I2Cdrv->Control      (ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_STANDARD);
  status = I2Cdrv->Control      (ARM_I2C_BUS_CLEAR, 0);
}

/* Public functions --------------------------------------------------------- */

void EINT3_IRQHandler (void)
{
  if (LPC_GPIOINT->IO0IntStatR & 1 << PIN_INT)
  {
    
    /* Arrancar "timer virtual" de 0.25 segundo */
    osSignalSet (tid_thread_overload_off, SIG_OVERLOAD_OFF);
  
    /* Borra el flag de la interrupcion */
    LPC_GPIOINT->IO0IntClr = 1 << PIN_INT;
  }
}

void thread_overload_off (void const *argument)
{
  
  while(1)
  {
    osSignalWait (SIG_OVERLOAD_OFF, osWaitForever);
    
    overload_status = true;
    escribir_FLASH_timestamp_overload();
    
    osDelay(250);
    
    overload_status = false;
  }
}

void thread_i2c (void const *argument) {
  uint8_t byte_tx[2];
  uint8_t byte_rx[2];
  osEvent event;
  uint16_t mensaje_rx;
  uint8_t direccion;
  uint8_t datos;
  
  pin_int_config();
  init_i2c();
  
  tid_thread_overload_off = osThreadCreate(osThread (thread_overload_off), NULL);

  while (1) {
    event = osMessageGet(queue_i2c_id, osWaitForever);
    
    if (event.status == osEventMessage) {
      mensaje_rx = event.value.v;
      direccion = (uint8_t) ((mensaje_rx & 0x0000ff00) >> 8);
      datos = (uint8_t) (mensaje_rx & 0x000000ff);
      
      if(direccion != 0x03)
      {
        /* Write */
        byte_tx[0] = direccion;
        byte_tx[1] = datos; // OJO IGUAL ES AL REVES
        
        status = I2Cdrv->MasterTransmit(LPC_SLAVE_I2C_ADDR, byte_tx, 2, true);
        osSignalWait (SIG_TEMP, osWaitForever);
      }
      
      /* Apartado opcional, lectura de Vo */
      //else if (direccion == 0x03)
      //{
      //  /* Write direccion a leer */
      //  status = I2Cdrv->MasterTransmit(LPC_SLAVE_I2C_ADDR, &direccion, 1, true);
      //  osSignalWait (SIG_TEMP, osWaitForever);
      //  
      //  /* Read */
      //  status = I2Cdrv->MasterReceive(LPC_SLAVE_I2C_ADDR, byte_rx, 2, true);
      //  osSignalWait (SIG_TEMP, osWaitForever);
      //  
      //  v_out = (((uint16_t) byte_rx[1]) << 8) || byte_rx[0]; // OJO IGUAL ES AL REVES
      //}
    }
    
    osThreadYield ();
  }
}
