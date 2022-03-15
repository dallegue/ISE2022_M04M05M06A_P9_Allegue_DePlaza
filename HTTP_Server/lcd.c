/******************************************************************************
 * Ejemplo_LCD
 * Fichero
 * Descripcion
 *
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/

#include "LPC17xx.h"
#include "PIN_LPC17xx.h"
#include "GPIO_LPC17xx.h"
#include "Driver_SPI.h"

#include "Arial12x12.h"
#include "lcd.h"
#include <string.h>

/* Macros --------------------------------------------------------------------*/

#define gpioPORT_SSP1 0
#define pin_nRESET_SSP1 8 
#define pin_A0_SSP1 6
#define pin_CS_SSP1 18

/* Public variables ----------------------------------------------------------*/

extern ARM_DRIVER_SPI Driver_SPI1;
ARM_DRIVER_SPI *SPIdrv = &Driver_SPI1;

/* Private variables ---------------------------------------------------------*/

static unsigned char buffer[512];
static uint16_t positionL1 = 0;
static uint16_t positionL2 = 0;

uint8_t flagW = 0;

/* Function prototypes -------------------------------------------------------*/

static void LCD_reset(void);
static void LCD_init(void);
static void LCD_update(uint8_t num_linea);
static void symbolToLocalBuffer(uint8_t line, uint8_t symbol);

/* Public functions --------------------------------------------------------- */

/* Llamar a esta funcion antes de usar LCD */
void LCD_SPI_startup(void)
{
    LCD_reset();
    LCD_init();
}

void print_linea(uint8_t num_linea, char linea[])
{
  uint8_t i;
  uint16_t linea_length = strlen(linea);
  
  if (num_linea == LINEA_1)
  {
    clear_linea_buffer(LINEA_1);
    
    positionL1 = 0;
    
    for (i = 0; i < linea_length; i++)
    {
      symbolToLocalBuffer(LINEA_1, (uint8_t) linea[i]);
    }
    
    LCD_update(LINEA_1);
  }
  else // num linea == LINEA_2
  {
    clear_linea_buffer(LINEA_2);
    
    positionL2 = 0;
    
    for (i = 0; i < linea_length; i++)
    {
      symbolToLocalBuffer(LINEA_2, (uint8_t) linea[i]);
    }
    
    LCD_update(LINEA_2);
  }
}

/*
 Numero maximo caracteres por linea de 25, puede que el display pueda con menos aun
*/

void print_lineas(char line1[], char line2[])
{
  print_linea(LINEA_1, line1);
  print_linea(LINEA_2, line2);
}

void clear_screen(void)
{
  clear_linea_buffer(LINEA_1);
  clear_linea_buffer(LINEA_2);
  
  LCD_update(LINEA_1);
  LCD_update(LINEA_2);
}

void clear_linea_buffer(uint8_t num_linea)
{
  uint16_t i;
  uint16_t offset_linea_2 = 0; // 0 para num_linea == LINEA_1
  
  if (num_linea == LINEA_2)
  {
    offset_linea_2 = 256;
  }
  
  for (i = 0; i < 256; i++)
  {
    buffer[i + offset_linea_2] = 0;
  }
}


/* Private functions -------------------------------------------------------- */

static void init_SPI(void)
{
    /* Initialize the SPI driver */
    SPIdrv->Initialize(NULL);

    /* Power up the SPI peripheral */
    SPIdrv->PowerControl(ARM_POWER_FULL);

    /* Configure the SPI to Master, 8-bit mode @20Mbits/sec */
    SPIdrv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA1 | 
                    ARM_SPI_SS_MASTER_UNUSED  | ARM_SPI_MSB_LSB | 
                    ARM_SPI_DATA_BITS(8), 20000000);
}

static void init_GPIO(void)
{
  /* RESETn */
    PIN_Configure (gpioPORT_SSP1, pin_nRESET_SSP1, PIN_FUNC_0, PIN_PINMODE_PULLUP, PIN_PINMODE_NORMAL);
    GPIO_SetDir (gpioPORT_SSP1, pin_nRESET_SSP1, GPIO_DIR_OUTPUT);
    GPIO_PinWrite (gpioPORT_SSP1,pin_nRESET_SSP1,1);
    
  /* CSn */
    PIN_Configure (gpioPORT_SSP1, pin_CS_SSP1, PIN_FUNC_0, PIN_PINMODE_PULLUP, PIN_PINMODE_NORMAL);
    GPIO_SetDir (gpioPORT_SSP1, pin_CS_SSP1, GPIO_DIR_OUTPUT);
    GPIO_PinWrite (gpioPORT_SSP1,pin_CS_SSP1,1);
    
  /* A0 */
    PIN_Configure (gpioPORT_SSP1, pin_A0_SSP1, PIN_FUNC_0, PIN_PINMODE_PULLUP, PIN_PINMODE_NORMAL);
    GPIO_SetDir (gpioPORT_SSP1, pin_A0_SSP1, GPIO_DIR_OUTPUT);
    GPIO_PinWrite (gpioPORT_SSP1,pin_A0_SSP1,1);
}

static void retardo_1us(void)
{
    uint32_t contador;

    for (contador = 0; contador < 20; contador++);
}
    
static void retardo_1ms(void){
    uint32_t contador;

    for (contador = 0; contador < 20000; contador++);
}

static void LCD_reset(void)
{
    init_SPI();
    init_GPIO();
    
    GPIO_PinWrite (gpioPORT_SSP1,pin_nRESET_SSP1,0);
    retardo_1us();
    GPIO_PinWrite (gpioPORT_SSP1,pin_nRESET_SSP1,1);
    retardo_1ms();
}

static void LCD_wr_cmd(uint8_t cmd)
{
    GPIO_PinWrite (gpioPORT_SSP1,pin_A0_SSP1,0);
    GPIO_PinWrite (gpioPORT_SSP1,pin_CS_SSP1,0);

    SPIdrv->Send(&cmd,sizeof(cmd));
    GPIO_PinWrite (gpioPORT_SSP1,pin_CS_SSP1,1);
}
 
static void LCD_wr_data(uint8_t dat)
{
    GPIO_PinWrite (gpioPORT_SSP1,pin_A0_SSP1,1);
    GPIO_PinWrite (gpioPORT_SSP1,pin_CS_SSP1,0);

    SPIdrv->Send(&dat,sizeof(dat));
    GPIO_PinWrite (gpioPORT_SSP1,pin_CS_SSP1,1);
}


static void LCD_init(void){
    LCD_wr_cmd(0xAE);    //Display off
    LCD_wr_cmd(0xA2);    //Fija el valor de la relación de la tensión de polarización del LCD a 1/9 
    LCD_wr_cmd(0xA0);    //El direccionamiento de la RAM de datos del display es la normal
    LCD_wr_cmd(0xC8);    //El scan en las salidas COM es el normal
    LCD_wr_cmd(0x22);    //Fija la relación de resistencias interna a 2
    LCD_wr_cmd(0x2F);    //Power on
    LCD_wr_cmd(0x40);    //Display empieza en la línea 0
    LCD_wr_cmd(0xAF);    //Display ON
    LCD_wr_cmd(0x81);    //Fija el contraste
    LCD_wr_cmd(0x0F);    //contraste //con 0x3F máximo contraste
    LCD_wr_cmd(0x17);    //Column address
    LCD_wr_cmd(0xA4);    // Display all points normal
    LCD_wr_cmd(0xA6);    //LCD Display normal
}

static void symbolToLocalBuffer_L1(uint8_t symbol)
{
  uint8_t i, value1, value2;
  uint16_t offset = 0;
  offset = 25 * (symbol - ' ');
  
  for (i = 0; i < 12; i++)
  {
    value1 = Arial12x12[offset + i*2 + 1];
    value2 = Arial12x12[offset + i*2 + 2];
    
    buffer[i + positionL1] = value1;
    buffer[i + 128 + positionL1] = value2;
  }
  
  positionL1 += Arial12x12[offset];
}

static void symbolToLocalBuffer_L2(uint8_t symbol)
{
  uint8_t i, value1, value2;
  uint16_t offset = 0;
  offset = 25 * (symbol - ' ');
  
  for (i = 0; i < 12; i++)
  {
    value1 = Arial12x12[offset + i*2 + 1];
    value2 = Arial12x12[offset + i*2 + 2];
    
    buffer[i + positionL2 + 256] = value1;
    buffer[i + 128 + positionL2 + 256] = value2;
  }
  
  positionL2 += Arial12x12[offset];
}

static void symbolToLocalBuffer(uint8_t line, uint8_t symbol)
{
  if (line == LINEA_1)
  {
    symbolToLocalBuffer_L1(symbol);
  }
  else // a la linea 2
  {
    symbolToLocalBuffer_L2(symbol);
  }
}

static void LCD_update(uint8_t num_linea) // escribe en linea 1 o 2 al usar Arial12x12
{
  int i;
  
  if (num_linea == LINEA_1)
  {
    LCD_wr_cmd(0x00); // 4 bits de la parte baja de la dirección a 0
    LCD_wr_cmd(0x10); // 4 bits de la parte alta de la dirección a 0
    LCD_wr_cmd(0xB0); // Página 0
    for(i=0;i<128;i++){
      LCD_wr_data(buffer[i]);
    }
    
    LCD_wr_cmd(0x00); // 4 bits de la parte baja de la dirección a 0
    LCD_wr_cmd(0x10); // 4 bits de la parte alta de la dirección a 0
    LCD_wr_cmd(0xB1); // Página 1
    for(i=128;i<256;i++){
      LCD_wr_data(buffer[i]);
    }
  }
  else // escribir en linea 2
  {
    LCD_wr_cmd(0x00);
    LCD_wr_cmd(0x10);
    LCD_wr_cmd(0xB2); //Página 2
    for(i=256;i<384;i++){
      LCD_wr_data(buffer[i]);
    }
    
    LCD_wr_cmd(0x00);
    LCD_wr_cmd(0x10);
    LCD_wr_cmd(0xB3); // Pagina 3
    for(i=384;i<512;i++){
      LCD_wr_data(buffer[i]);
    }
  }
}
