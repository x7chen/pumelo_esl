#include <stdint.h>
#include <string.h>
#include "m_twi_master.h"
#include "app_error.h"
#include "nrf_gpio.h"

#define SI7021_DEVICE_ADDR      0x40
#define SI7021_I2C_SDA_PIN      19
#define SI7021_I2C_SCL_PIN      20
#define TX_RX_DATA_LENGTH       32

/* Commands */
#define CMD_MEASURE_HUMIDITY_HOLD       0xE5
#define CMD_MEASURE_HUMIDITY_NO_HOLD    0xF5
#define CMD_MEASURE_TEMPERATURE_HOLD    0xE3
#define CMD_MEASURE_TEMPERATURE_NO_HOLD 0xF3
#define CMD_MEASURE_THERMISTOR_HOLD     0xEE
#define CMD_READ_PREVIOUS_TEMPERATURE   0xE0
#define CMD_RESET                       0xFE
#define CMD_WRITE_REGISTER_1            0xE6
#define CMD_READ_REGISTER_1             0xE7
#define CMD_WRITE_REGISTER_2            0x50
#define CMD_READ_REGISTER_2             0x10
#define CMD_WRITE_REGISTER_3            0x51
#define CMD_READ_REGISTER_3             0x11
#define CMD_WRITE_COEFFICIENT           0xC5
#define CMD_READ_COEFFICIENT            0x84

/* User Register 1 */
#define REG1_RESOLUTION_MASK            0x81
#define REG1_RESOLUTION_H12_T14         0x00
#define REG1_RESOLUTION_H08_T12         0x01
#define REG1_RESOLUTION_H10_T13         0x80
#define REG1_RESOLUTION_H11_T11         0x81
#define REG1_LOW_VOLTAGE                0x40
#define REG1_ENABLE_HEATER              0x04

/* User Register 2 */
#define REG2_VOUT                       0x01
#define REG2_VREF_VDD                   0x02
#define REG2_VIN_BUFFERED               0x04
#define REG2_RESERVED                   0x08
#define REG2_FAST_CONVERSION            0x10
#define REG2_MODE_CORRECTION            0x20
#define REG2_MODE_NO_HOLD               0x40
static uint8_t m_twi_tx_buf[TX_RX_DATA_LENGTH]; /**< SPI master TX buffer. */

void SI7021_I2C_init()
{
    twi_master_init(SI7021_I2C_SDA_PIN,SI7021_I2C_SCL_PIN);
}
void SI7021_I2C_uninit()
{
    nrf_gpio_cfg_default(SI7021_I2C_SDA_PIN);
    nrf_gpio_cfg_default(SI7021_I2C_SCL_PIN);
}
bool SI7021_I2C_Read(uint8_t reg_addr,uint8_t *data,uint8_t length)
{
    bool transfer_succeeded;
    transfer_succeeded = twi_master_transfer((SI7021_DEVICE_ADDR<<1), &reg_addr, 1, TWI_DONT_ISSUE_STOP);
    transfer_succeeded &= twi_master_transfer((SI7021_DEVICE_ADDR<<1)|TWI_READ_BIT, data, length, TWI_ISSUE_STOP);   
    return transfer_succeeded;
}
bool SI7021_I2C_Write(uint8_t reg_addr,uint8_t *data,uint8_t length)
{
    bool transfer_succeeded;
    m_twi_tx_buf[0] = reg_addr;
    memcpy(m_twi_tx_buf+1,data,length);
    transfer_succeeded = twi_master_transfer((SI7021_DEVICE_ADDR<<1), m_twi_tx_buf, length+1, TWI_ISSUE_STOP); 
    return transfer_succeeded;
}
uint8_t  SI7021_I2C_Read_Byte(uint8_t reg_addr)
{
    uint8_t data;
    SI7021_I2C_Read(reg_addr,&data,1);    
    return data;
}
void SI7021_I2C_Write_Byte(uint8_t reg_addr,uint8_t data)
{
    SI7021_I2C_Write(reg_addr,&data,1); 
    
}

uint8_t SI7021_ReadTemperature(uint8_t *Temperature )
{
    uint8_t Temp[2] = {0,0};
    int16_t Value;
    SI7021_I2C_init();
    SI7021_I2C_Read(CMD_MEASURE_TEMPERATURE_HOLD,Temp,2);
    Value = Temp[0]*256 + Temp[1];
    Value = (((int32_t)Value*1757)>>16) - 469;
    *Temperature=(uint8_t)(Value/5);
    SI7021_I2C_uninit();
    return 0;
}
uint8_t SI7021_ReadHumidity(uint8_t *Humidity)
{
    uint8_t Temp[2] = {0,0};
    uint16_t Value;
    SI7021_I2C_init();
    SI7021_I2C_Read(CMD_MEASURE_HUMIDITY_HOLD,Temp,2);
    Value = Temp[0]*256 + Temp[1];
    *Humidity = (((uint32_t)Value*125)>>16) - 6;
    SI7021_I2C_uninit();
    return 0;
}
