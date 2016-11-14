#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "m_twi_master.h"
#include "nrf_drv_gpiote.h"
#include "app_error.h"
#include "si1132.h"
#include "si1132_map.h"
#include "nrf_delay.h"

#define SI1132_DEVICE_ADDR      0x60
#define SI1132_I2C_SDA_PIN      16
#define SI1132_I2C_SCL_PIN      17
#define SI1132_INT_PIN          18
#define TX_RX_DATA_LENGTH       32
static uint8_t m_twi_tx_buf[TX_RX_DATA_LENGTH]; /**< SPI master TX buffer. */
bool Si1132_is_Initialized = false;
bool Si1132_UV_Updated = false;
bool Si1132_Command_Finish = false;
uint8_t UV_Value;
si1132_callbacks_t *Si1132_Callbacks = NULL;
void Si1132_INT_Handle(void);
uint32_t Si1132_ForceMeasure(void);

uint32_t Si1132_INT_ON()
{
    uint32_t err_code = 0;
    if (!nrf_drv_gpiote_is_init())
    {
        err_code = nrf_drv_gpiote_init();
        if (err_code != NRF_SUCCESS)
        {
            return err_code;
        }
    }
    nrf_drv_gpiote_in_config_t gpiote_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
    //gpiote_config.pull = NRF_GPIO_PIN_PULLUP;
    nrf_drv_gpiote_in_init(SI1132_INT_PIN, &gpiote_config, (nrf_drv_gpiote_evt_handler_t)Si1132_INT_Handle);
    nrf_drv_gpiote_in_event_enable(SI1132_INT_PIN, true);
    return err_code;
}

void Si1132_I2C_init()
{
    twi_master_init(SI1132_I2C_SDA_PIN,SI1132_I2C_SCL_PIN);
}
void Si1132_I2C_uninit()
{
    nrf_gpio_cfg_default(SI1132_I2C_SDA_PIN);
    nrf_gpio_cfg_default(SI1132_I2C_SCL_PIN);
}
bool Si1132_I2C_Read(uint8_t reg_addr,uint8_t *data,uint8_t length)
{
    bool transfer_succeeded;
    transfer_succeeded = twi_master_transfer((SI1132_DEVICE_ADDR<<1), &reg_addr, 1, TWI_DONT_ISSUE_STOP);
    transfer_succeeded &= twi_master_transfer((SI1132_DEVICE_ADDR<<1)|TWI_READ_BIT, data, length, TWI_ISSUE_STOP);   
    return transfer_succeeded;
}
bool Si1132_I2C_Write(uint8_t reg_addr,uint8_t *data,uint8_t length)
{
    bool transfer_succeeded;
    m_twi_tx_buf[0] = reg_addr;
    memcpy(m_twi_tx_buf+1,data,length);
    transfer_succeeded = twi_master_transfer((SI1132_DEVICE_ADDR<<1), m_twi_tx_buf, length+1, TWI_ISSUE_STOP); 
    return transfer_succeeded;
}
uint8_t  Si1132_I2C_Read_Byte(uint8_t reg_addr)
{
    uint8_t data;
    Si1132_I2C_Read(reg_addr,&data,1);    
    return data;
}
void Si1132_I2C_Write_Byte(uint8_t reg_addr,uint8_t data)
{
    Si1132_I2C_Write(reg_addr,&data,1); 
    
}

uint32_t Si1132_SetParameter(uint8_t reg,uint8_t val)
{
    uint8_t response;
    Si1132_I2C_init();
    Si1132_I2C_Write_Byte(REG_PARAM_WR,val);
    Si1132_I2C_Write_Byte(REG_COMMAND,COMMAND_PARAM_SET+reg);
    response = Si1132_I2C_Read_Byte(REG_RESPONSE);
    Si1132_I2C_uninit();
    return response;
}

uint32_t Si1132_GetParameter(uint8_t reg,uint8_t *rd_value)
{
    uint8_t response;
    Si1132_I2C_init();
    Si1132_I2C_Write_Byte(REG_COMMAND,COMMAND_PARAM_QUERY+reg);
    response = Si1132_I2C_Read_Byte(REG_RESPONSE);
    *rd_value = Si1132_I2C_Read_Byte(REG_PARAM_RD);
    Si1132_I2C_uninit();
    return response;
}
uint32_t Si1132_ForceMeasure()
{
    uint8_t response;
    Si1132_I2C_init();
    Si1132_I2C_Write_Byte(REG_COMMAND,COMMAND_ALS_FORCE); 
    response = Si1132_I2C_Read_Byte(REG_RESPONSE);
    Si1132_I2C_uninit();
    return response;
}
void Si1132_Init()
{
    Si1132_INT_ON();
    Si1132_I2C_init();
    //Si1132_I2C_Write_Byte(REG_COMMAND,COMMAND_RESET);
    nrf_delay_ms(10);
    Si1132_I2C_Write_Byte(REG_HW_KEY,HW_KEY_VAL0);
    Si1132_I2C_Write_Byte(REG_INT_CFG,0x01);
    Si1132_I2C_Write_Byte(REG_IRQ_ENABLE,0x01);
    Si1132_I2C_Write_Byte(REG_MEAS_RATE,0x00);
    Si1132_I2C_Write_Byte(REG_ALS_RATE,0x00);
    Si1132_I2C_Write_Byte(REG_UCOEF0,0x7B);
    Si1132_I2C_Write_Byte(REG_UCOEF1,0x6B);
    Si1132_I2C_Write_Byte(REG_UCOEF2,0x01);
    Si1132_I2C_Write_Byte(REG_UCOEF3,0x00);
    Si1132_I2C_Write_Byte(REG_IRQ_STATUS,0xFF);
    Si1132_I2C_uninit();
    Si1132_SetParameter(PARAM_CH_LIST,0x80);
    Si1132_is_Initialized = true;
    
}

void Si1132_INT_Handle()
{
    uint8_t ALS_Value[2];
    Si1132_I2C_init();
    if(Si1132_I2C_Read_Byte(REG_IRQ_STATUS)&0x01)
    {
        ALS_Value[0] = Si1132_I2C_Read_Byte(REG_AUX_DATA1);
        ALS_Value[1] = Si1132_I2C_Read_Byte(REG_AUX_DATA0);
        
        UV_Value = ((uint16_t)ALS_Value[0]*256+ALS_Value[1])/100;
        Si1132_UV_Updated = true;
        if(Si1132_Callbacks != NULL)
        {
            Si1132_Callbacks->on_UV_update(ALS_Value,2);
        }
    }
    
    Si1132_I2C_Write_Byte(REG_IRQ_STATUS,0xFF);
    Si1132_I2C_uninit();
}

uint32_t Si1132_get_UV_Index(uint8_t *rd_val)
{
    uint32_t err_code = 1;
    uint32_t delay = 100;
    if(!Si1132_is_Initialized)
    {
        Si1132_Init();
    }
    err_code = Si1132_ForceMeasure();
    while(delay--)
    {
        nrf_delay_ms(1);
        if(Si1132_UV_Updated)
        {
            *rd_val = UV_Value;
            Si1132_UV_Updated = false;
            err_code = 0;
            break;
        }
    }
    return err_code;
}

void si1132_callback_is_null(void * content, uint16_t length)
{
    
}
void register_si1132_callbacks(si1132_callbacks_t *callbacks)
{
    Si1132_Callbacks = callbacks;
    if(Si1132_Callbacks->on_UV_update == NULL)
    {
        Si1132_Callbacks->on_UV_update = si1132_callback_is_null;
    }
}
