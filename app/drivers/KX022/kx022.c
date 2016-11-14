#include "KX022.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "config.h"
#include "twi_master.h"
#include "app_timer.h"
#include "oled_drv.h"
#include "app_scheduler.h"
#include "nrf_drv_gpiote.h"
#include "bsp.h"

#define     TX_RX_DATA_LENGTH       32
static uint8_t m_tx_data_i2c[TX_RX_DATA_LENGTH]; /**< SPI master TX buffer. */
//static uint8_t m_rx_data_i2c[TX_RX_DATA_LENGTH]; /**< SPI master RX buffer. */
uint8_t KX_INT_Value = 0;
AxesRaw_t KX_Value;
//bool KX_I2C_Read(uint8_t reg_addr,uint8_t *data,uint8_t length)
//{
//    bool Error;
//    Error = twi_master_transfer(KX022_I2C_READ_BIT+(KX022_DEVICE_ADDR<<1), reg_addr, data, length, TWI_ISSUE_STOP);    
//    return Error;
//}
//bool KX_I2C_Write(uint8_t reg_addr,uint8_t *data,uint8_t length)
//{
//    bool Error;
//    Error = twi_master_transfer(KX022_I2C_WRITE_BIT+(KX022_DEVICE_ADDR<<1), reg_addr, data, length, TWI_ISSUE_STOP); 
//    return Error;
//}
//uint8_t  KX_I2C_Read_Byte(uint8_t reg_addr)
//{
//    uint8_t data;
//    twi_master_transfer(KX022_I2C_READ_BIT+(KX022_DEVICE_ADDR<<1), reg_addr, &data, 1, TWI_ISSUE_STOP);    
//    return data;
//}
//void KX_I2C_Write_Byte(uint8_t reg_addr,uint8_t data)
//{
//    twi_master_transfer(KX022_I2C_WRITE_BIT+(KX022_DEVICE_ADDR<<1), reg_addr, &data, 1, TWI_ISSUE_STOP); 
//    
//}
bool KX_I2C_Read(uint8_t reg_addr,uint8_t *data,uint8_t length)
{
    bool transfer_succeeded;
    transfer_succeeded = twi_master_transfer((KX022_DEVICE_ADDR<<1), &reg_addr, 1, TWI_DONT_ISSUE_STOP);
    transfer_succeeded &= twi_master_transfer((KX022_DEVICE_ADDR<<1)|TWI_READ_BIT, data, length, TWI_ISSUE_STOP);   
    return transfer_succeeded;
}
bool KX_I2C_Write(uint8_t reg_addr,uint8_t *data,uint8_t length)
{
    bool transfer_succeeded;
    m_tx_data_i2c[0] = reg_addr;
    memcpy(m_tx_data_i2c+1,data,length);
    transfer_succeeded = twi_master_transfer((KX022_DEVICE_ADDR<<1), m_tx_data_i2c, length+1, TWI_ISSUE_STOP); 
    return transfer_succeeded;
}
uint8_t  KX_I2C_Read_Byte(uint8_t reg_addr)
{
    uint8_t data;
    KX_I2C_Read(reg_addr,&data,1);    
    return data;
}
void KX_I2C_Write_Byte(uint8_t reg_addr,uint8_t data)
{
    KX_I2C_Write(reg_addr,&data,1); 
    
}

bool KX_Init()
{
    bool Error;
    KX_I2C_Write_Byte(KX_CNTL1_ADDR, 0x00);
    KX_I2C_Write_Byte(KX_CNTL2_ADDR, 0x00);
    KX_I2C_Write_Byte(KX_CNTL3_ADDR, 0x9C);     //开启运动检测 12.5Hz 9C
    KX_I2C_Write_Byte(KX_ODCNTL_ADDR,0x01);  //01:25hz 03:100hz
    KX_I2C_Write_Byte(KX_INC1_ADDR,  0x20); 
    KX_I2C_Write_Byte(KX_INC2_ADDR,  0x3F);     // 3F
    KX_I2C_Write_Byte(KX_INC3_ADDR,  0x00);
    //KX_I2C_Read_Byte(KX_INTREL_ADDR);
    KX_I2C_Write_Byte(KX_INC4_ADDR,  0x02);     //02
    KX_I2C_Write_Byte(KX_WUFC_ADDR,  0x03);     //03
    KX_I2C_Write_Byte(KX_ATH_ADDR,   0x04);     //04
    
    KX_I2C_Write_Byte(KX_BUF_CNTL1_ADDR,  0x00);
    KX_I2C_Write_Byte(KX_BUF_CNTL2_ADDR,  0xC0);
    KX_I2C_Write_Byte(KX_BUF_CLEAR_ADDR,  0x00);
    KX_I2C_Write_Byte(KX_LP_CNTL_ADDR,  0x2B);
    KX_I2C_Write_Byte(KX_CNTL1_ADDR,      0x88);    //连接运动检测中断 0x88
    return Error;
}
bool KX_Close()
{
    
    bool Error;
    KX_I2C_Write_Byte(KX_CNTL1_ADDR, 0x00);
    return Error;
}

void KX_ReadBuf(uint8_t *val,uint8_t *sets)
{
    uint8_t len;
    KX_I2C_Read(KX_BUF_STATUS1_ADDR,&len,1);
    KX_I2C_Read(KX_BUF_READ_ADDR,val,len);
    *sets = len/6;
}
static bool kx022_int1_status = false;
uint32_t KX022_INT1_Init()
{
    uint32_t err_code;
    if (!nrf_drv_gpiote_is_init())
    {
        err_code = nrf_drv_gpiote_init();
        if (err_code != NRF_SUCCESS)
        {
            return err_code;
        }
    }
    nrf_drv_gpiote_in_config_t G_Sensor_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(false);
    nrf_drv_gpiote_in_init(KX022_INT1, &G_Sensor_config, (nrf_drv_gpiote_evt_handler_t)KX_INT1_handle);
    nrf_drv_gpiote_in_event_enable(KX022_INT1, true);
}
uint32_t KX022_INT1_Enable()
{
    KX_I2C_Write_Byte(KX_CNTL1_ADDR,      0x8A);   
}
uint32_t KX022_INT1_Disable()
{
    KX_I2C_Write_Byte(KX_CNTL1_ADDR,      0x88);
}
void KX_INT1_handle()
{
    uint8_t status = KX_I2C_Read_Byte(KX_INS2_ADDR);
    if(status&0x02)
    {
        KX_INT_Value = 1;
        KX022_INT1_Disable();
    }
    KX_I2C_Read_Byte(KX_INTREL_ADDR);
}

