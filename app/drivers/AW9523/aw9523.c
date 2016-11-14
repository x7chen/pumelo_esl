#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "m_twi_master.h"
#include "app_error.h"
#include "nrf_gpio.h"
#include "aw9523.h"

#define AW9523_DEVICE_ADDR      0x5B
#define AW9523_I2C_SDA_PIN      25
#define AW9523_I2C_SCL_PIN      24
#define AW9523_RSTN_PIN         28
#define TX_RX_DATA_LENGTH       32

#define LED_DRIVE_LEVEL         0x10
#define LED_DRIVE_OFF           0x00



#define LED_PIN_BASE            0x24
#define LED_PIN_BASE2           0x20

#define AW9523_RSTN(Hi)     do{\
                                nrf_gpio_cfg_output(AW9523_RSTN_PIN);\
                                if(Hi==1) \
                                {nrf_gpio_pin_set(AW9523_RSTN_PIN);}\
                                else \
                                {nrf_gpio_pin_clear(AW9523_RSTN_PIN);}}while(0)

static uint8_t m_twi_tx_buf[TX_RX_DATA_LENGTH]; /**< SPI master TX buffer. */
bool AW9523_is_Initialized = false;
void AW9523_I2C_init()
{
    twi_master_init(AW9523_I2C_SDA_PIN,AW9523_I2C_SCL_PIN);
}
void AW9523_I2C_uninit()
{
    nrf_gpio_cfg_default(AW9523_I2C_SDA_PIN);
    nrf_gpio_cfg_default(AW9523_I2C_SCL_PIN);
}
bool AW9523_I2C_Read(uint8_t reg_addr,uint8_t *data,uint8_t length)
{
    bool transfer_succeeded;
    transfer_succeeded = twi_master_transfer((AW9523_DEVICE_ADDR<<1), &reg_addr, 1, TWI_DONT_ISSUE_STOP);
    transfer_succeeded &= twi_master_transfer((AW9523_DEVICE_ADDR<<1)|TWI_READ_BIT, data, length, TWI_ISSUE_STOP);   
    return transfer_succeeded;
}
bool AW9523_I2C_Write(uint8_t reg_addr,uint8_t *data,uint8_t length)
{
    bool transfer_succeeded;
    m_twi_tx_buf[0] = reg_addr;
    memcpy(m_twi_tx_buf+1,data,length);
    transfer_succeeded = twi_master_transfer((AW9523_DEVICE_ADDR<<1), m_twi_tx_buf, length+1, TWI_ISSUE_STOP); 
    return transfer_succeeded;
}
uint8_t  AW9523_I2C_Read_Byte(uint8_t reg_addr)
{
    uint8_t data;
    AW9523_I2C_Read(reg_addr,&data,1);    
    return data;
}
void AW9523_I2C_Write_Byte(uint8_t reg_addr,uint8_t data)
{
    AW9523_I2C_Write(reg_addr,&data,1); 
    
}
static void LEDS_INIT()
{
    AW9523_RSTN(1);
    AW9523_I2C_Write_Byte(0x11,0x03);
    //模式切换
    AW9523_I2C_Write_Byte(0x12,0x00);
    AW9523_I2C_Write_Byte(0x13,0x00); 
}
void LEDS_SET(uint16_t leds)
{
    uint8_t i;
    AW9523_I2C_init();
    LEDS_INIT();
    for(i=0;i<8;i++)
    {
        if(leds&(0x0001<<i))
        {
            AW9523_I2C_Write_Byte(LED_PIN_BASE+i,LED_DRIVE_LEVEL);
        }
        else
        {
            AW9523_I2C_Write_Byte(LED_PIN_BASE+i,LED_DRIVE_OFF);
        }
    }
    if(leds&(0x0001<<8))
    {
        AW9523_I2C_Write_Byte(LED_PIN_BASE2,LED_DRIVE_LEVEL);
    }
    else
    {
        AW9523_I2C_Write_Byte(LED_PIN_BASE2,LED_DRIVE_OFF);
    } 
    AW9523_I2C_uninit();
}
void LEDS_ON()
{
//    LED_SET(0x01FF);
    LEDS_SET(LED1_COLOR_RED|LED2_COLOR_RED|LED3_COLOR_RED);
}
void LEDS_OFF()
{
    LEDS_SET(0x000);
    AW9523_RSTN(0);
}
