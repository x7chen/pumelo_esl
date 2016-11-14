/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/** @file
* @defgroup spi_master_example_main main.c
* @{
* @ingroup spi_master_example
*
* @brief SPI Master Loopback Example Application main file.
*
* This file contains the source code for a sample application using SPI.
*
*/

#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "app_error.h"
#include "app_util_platform.h"
#include "spi_master.h"
#include "bsp.h"
#include "app_timer.h"
#include "nordic_common.h"
#include "oled_drv.h"
#include "kx022.h"
#include "step_counter.h"
#include "app_timer.h"
#include "app_scheduler.h"
#include "ui_res.h"

#define OLED_PIX_WIDTH      64
#define OLED_PIX_HEIGHT     32
#define OLED_PAGES          4

#define FRAME_BUFFER    1
#define PIXEL_DEVICE    2
#define OLED_MODE   FRAME_BUFFER
app_timer_id_t   m_oled_timer_id;
app_timer_id_t   m_oled_scrolling_id;


#define APP_TIMER_PRESCALER             0 
#define TEN_SECOND_INTERVAL         APP_TIMER_TICKS(1000*10, APP_TIMER_PRESCALER)

#define OLED_SCROLLING_TIME         APP_TIMER_TICKS(40, APP_TIMER_PRESCALER)
static uint8_t oled_frame_buffer[2][256]={0};

static uint8_t * p_oled_frame_current = oled_frame_buffer[0];
static uint8_t * p_oled_frame_next = oled_frame_buffer[1];

static bool oled_timer_status =  0;
static bool oled_scrolling_status =  0;

uint8_t oled_cnt=0;

#define TX_RX_MSG_LENGTH         8

static uint8_t m_tx_data_spi[TX_RX_MSG_LENGTH]; /**< SPI master TX buffer. */
static uint8_t m_rx_data_spi[TX_RX_MSG_LENGTH]; /**< SPI master RX buffer. */

#define LCM_CMD()       nrf_gpio_pin_clear(LCM_LSDC)
#define LCM_DATA()      nrf_gpio_pin_set(LCM_LSDC)



/**@brief Function for initializing a SPI master driver.
 *
 * @param[in] spi_master_instance       An instance of SPI master module.
 * @param[in] spi_master_event_handler  An event handler for SPI master events.
 * @param[in] lsb                       Bits order LSB if true, MSB if false.
 */
static void spi_master_start(spi_master_hw_instance_t   spi_master_instance)
{
    uint32_t err_code = NRF_SUCCESS;

    // Configure SPI master.
    spi_master_config_t spi_config = SPI_MASTER_INIT_DEFAULT;
    spi_config.SPI_Freq  = SPI_FREQUENCY_FREQUENCY_M4;
    spi_config.SPI_Pin_SCK  = SPIM0_SCK_PIN;
    //spi_config.SPI_Pin_MISO = SPIM0_MISO_PIN;
    spi_config.SPI_Pin_MOSI = SPIM0_MOSI_PIN;
    spi_config.SPI_Pin_SS   = SPIM0_SS0_PIN;
    spi_config.SPI_PriorityIRQ   = APP_IRQ_PRIORITY_LOW;
    
//    spi_config.SPI_CONFIG_ORDER = SPI_CONFIG_ORDER_LsbFirst;
    spi_config.SPI_CONFIG_ORDER = SPI_CONFIG_ORDER_MsbFirst;
    
//    spi_config.SPI_CONFIG_CPOL   = SPI_CONFIG_CPOL_ActiveLow;
    spi_config.SPI_CONFIG_CPOL   = SPI_CONFIG_CPOL_ActiveHigh;
    
//    spi_config.SPI_CONFIG_CPHA   = SPI_CONFIG_CPHA_Trailing;
    spi_config.SPI_CONFIG_CPHA   = SPI_CONFIG_CPHA_Leading;
    
    spi_config.SPI_DisableAllIRQ = 1;
    err_code = spi_master_open(spi_master_instance, &spi_config);
    APP_ERROR_CHECK(err_code);

    // Register event handler for SPI master.
    //spi_master_evt_handler_reg(spi_master_instance, spi_master_event_handler);
}
void spi_master_stop(spi_master_hw_instance_t   spi_master_instance)
{
    spi_master_close(spi_master_instance);
}
void OLED_WriteCommand(uint8_t command)
{
    //nrf_gpio_pin_clear(LCM_LSCE);
    nrf_gpio_pin_clear(LCM_LSDC);
    spi_master_send_recv(SPI_MASTER_0, &command,1,m_rx_data_spi, 1);
    //nrf_delay_us(100);
    nrf_gpio_pin_set(LCM_LSDC);
    //nrf_gpio_pin_set(LCM_LSCE);
}
void OLED_WriteLongCommand(uint8_t *command,uint8_t length)
{
    //nrf_gpio_pin_clear(LCM_LSCE);
    nrf_gpio_pin_clear(LCM_LSDC);
    spi_master_send_recv(SPI_MASTER_0, command,length,m_rx_data_spi, length);
    //nrf_delay_us(100);
    nrf_gpio_pin_set(LCM_LSDC);
    //nrf_gpio_pin_set(LCM_LSCE);
}
void OLED_WriteData(uint8_t data)
{
    //nrf_gpio_pin_clear(LCM_LSCE);
    nrf_gpio_pin_set(LCM_LSDC);
    spi_master_send_recv(SPI_MASTER_0, &data,1,m_rx_data_spi, 1);
    //nrf_delay_us(100);
    //nrf_gpio_pin_set(LCM_LSCE);
}
void OLED_WriteLongData(uint8_t *data,uint8_t length)
{
    //nrf_gpio_pin_clear(LCM_LSCE);
    nrf_gpio_pin_set(LCM_LSDC);
    spi_master_send_recv(SPI_MASTER_0, data,1,m_rx_data_spi, length);
    //nrf_delay_us(100);
    //nrf_gpio_pin_set(LCM_LSCE);
}
void OLED_Init()
{
    spi_master_start(SPI_MASTER_0);
    nrf_gpio_cfg_output(LCM_LSDC); 
    nrf_gpio_cfg_output(LCM_LSRST);
    nrf_gpio_pin_clear(LCM_LSRST);
    nrf_delay_ms(10);
    nrf_gpio_pin_set(LCM_LSRST);
    nrf_delay_ms(10);
    OLED_WriteCommand(0xAE);        //Set Display Off 
    
    OLED_WriteCommand(0xD5);        //Set Display Clock
    OLED_WriteCommand(0xD1);        //D1

    OLED_WriteCommand(0xA8);
    OLED_WriteCommand(0x1F);        //Set Multiplex Ratio 64
    
    OLED_WriteCommand(0xD3);        //Set Display Offset
    OLED_WriteCommand(0x00);
    
    OLED_WriteCommand(0x40);        //Set Display Start Line 
    
    OLED_WriteCommand(0x8D);        //Set Display PUMP
    OLED_WriteCommand(0x14);        //For enabling charge pump  
//  OLED_WriteCommand(0x10);        //Disable charge pump 

    OLED_WriteCommand(0xA1);        //Segment Remap A0/A1 
                                    //...}
    OLED_WriteCommand(0xC8);        //Set COM Output Scan Direction C0/C8
    
    OLED_WriteCommand(0xDA);        //Set COM Pins Hardware Configuration
    OLED_WriteCommand(0x12);
    
    OLED_WriteCommand(0x81);        //对比度
    OLED_WriteCommand(0xF0);        //0xC0

    OLED_WriteCommand(0xD9);        //Set Pre-charge Period
    OLED_WriteCommand(0xF7);

    OLED_WriteCommand(0xDB);        //Set VCOMH Deselect Level
    OLED_WriteCommand(0x00);

    OLED_WriteCommand(0x20);        
    OLED_WriteCommand(0x00);        //Horizontal Addressing Mode
    
    OLED_WriteCommand(0xA4);        //Set Entire Display On/Off  A4/A5
    
    OLED_WriteCommand(0xA6);        //Set Normal Display  A6/A7
    
    OLED_WriteCommand(0xAF);
    spi_master_stop(SPI_MASTER_0);
}
void m_oled_handler(void * p_context)
{
    uint32_t err_code;
    err_code = app_sched_event_put(NULL, 0, (app_sched_event_handler_t)OLED_OFF);
    APP_ERROR_CHECK(err_code);
    oled_timer_status = false;
}

uint8_t oled_scrolling_index=0;

void m_oled_scrolling_handler(void * p_context)
{
    uint32_t err_code;
    oled_scrolling_status = false;
    err_code = app_sched_event_put(NULL, 0, (app_sched_event_handler_t)OLED_Display);
    APP_ERROR_CHECK(err_code);
    
    //nrf_gpio_pin_toggle(LED_0);
}
void OLED_ON_TIME(uint32_t * ms, uint16_t event_size)
{
    uint32_t    err_code;
    if(true == oled_timer_status)
    {
        err_code = app_timer_stop(m_oled_timer_id);
        APP_ERROR_CHECK(err_code);
        oled_timer_status = false;
    }
    else
    {
        OLED_ON();
    }
    err_code = app_timer_start(m_oled_timer_id,APP_TIMER_TICKS(*ms, APP_TIMER_PRESCALER),NULL);
    APP_ERROR_CHECK(err_code);
    oled_timer_status = true;
}

void OLED_ON(void)
{
    spi_master_start(SPI_MASTER_0);
    OLED_WriteCommand(0xAF);     //Set Display On
    OLED_WriteCommand(0x8D);     //Set Display PUMP  
    OLED_WriteCommand(0x14);      //Enable charge pump 
    spi_master_stop(SPI_MASTER_0);
}
void OLED_OFF()
{
    spi_master_start(SPI_MASTER_0);
    OLED_WriteCommand(0xAE);     //Set Display Off 
    OLED_WriteCommand(0x8D);     //Set Display PUMP 
    OLED_WriteCommand(0x10);      //Disable charge pump  
    spi_master_stop(SPI_MASTER_0);
}
bool OLED_STATUS()
{
    return oled_timer_status;
}
uint8_t scroll_direction = 0;
bool scroll_en = false;
bool load_data = false;
void set_scroll_en(bool val)
{
    scroll_en = val;
}
bool get_scroll_en(void)
{
    return scroll_en;
}
void OLED_Display()
{
    uint32_t i,j;
    uint32_t err_code;
    static uint8_t oled_page=0;
    spi_master_start(SPI_MASTER_0);
    if(load_data) 
    {    
        if(oled_page==0)
        {
            OLED_WriteCommand(0x21);     
            OLED_WriteCommand(0x20);       
            OLED_WriteCommand(0x5F);  
            OLED_WriteCommand(0x22);
            OLED_WriteCommand(0x00);
            OLED_WriteCommand(0x03);   
        }
        else
        {
            OLED_WriteCommand(0x21);     
            OLED_WriteCommand(0x20);       
            OLED_WriteCommand(0x5F);  
            OLED_WriteCommand(0x22);
            OLED_WriteCommand(0x04);
            OLED_WriteCommand(0x07);
        }
        for (i = 0; i < 256; i++)  
        {   
            OLED_WriteData(p_oled_frame_next[i]);
        }
        load_data  = false;
    }
    if(get_scroll_en())
    {
        if(scroll_direction==3)
        {
            //oled_scrolling_index += 8;
            oled_scrolling_index += ((OLED_PIX_HEIGHT+3-oled_scrolling_index)/3);
            if(oled_scrolling_index>=OLED_PIX_HEIGHT)
            {
                scroll_direction = 0;
                oled_scrolling_index = 0;
                oled_page = (oled_page+1)&0x01;
            }
            
        }

        if(scroll_direction==0)
        {
            err_code = app_timer_stop(m_oled_scrolling_id);
            APP_ERROR_CHECK(err_code);
            oled_scrolling_status = false;
            set_scroll_en(false);
        }
        else
        {
            err_code = app_timer_start(m_oled_scrolling_id,OLED_SCROLLING_TIME,NULL);;
            APP_ERROR_CHECK(err_code);
            oled_scrolling_status = true;
        }
    }
    else
    {
        oled_page = (oled_page+1)&0x01;
    }
    if(oled_page==1)
    {
//        OLED_WriteCommand(0xD3);//会闪烁
//        OLED_WriteCommand(0x00+oled_scrolling_index);
        OLED_WriteCommand(0x40+oled_scrolling_index);
    }
    else
    {
//        OLED_WriteCommand(0xD3);
//        OLED_WriteCommand(0x20+oled_scrolling_index);        
        OLED_WriteCommand(0x60+oled_scrolling_index);

    }
    spi_master_stop(SPI_MASTER_0);
    
}

void OLED_Clear()
{
    uint32_t i,j;
    uint8_t data[3];
    memset(p_oled_frame_next,0,OLED_PIX_WIDTH*OLED_PAGES);
    scroll_direction = 3;
    load_data = true;
}

void OLED_Message(uint8_t page,uint8_t col,void * msg)
{
    uint32_t i,j;
    if(col>(OLED_PIX_WIDTH-1)||page>(OLED_PAGES-1))
    {
        return;
    }
    i=0;
    while(*(uint8_t *)msg != '\0') 
    {  
        
        for(j=0;j<6;j++)
        {
            if((i+col)>=OLED_PIX_WIDTH)
            {
                break;
            }
            p_oled_frame_next[page*64+col+i]=(px6x8_ascii[*(uint8_t *)msg-0x20][j]);
            i++;
        }
        msg=((uint8_t *)msg)+1;
    }
}
uint8_t Byte_Swapbit(uint8_t byte)
{
    uint8_t i;
    uint8_t retval = 0;
    
    for(i = 0; i < 8; i++)
    {
        retval |= ((byte >> i) & 0x01) << (7 - i);     
    }
    
    return retval; 
}

void OLED_Picture(uint8_t page,uint8_t col,pic_block_t *pic)
{
    uint32_t i,j,k;
    uint8_t pages;
    uint8_t data[3];
    if(col>(OLED_PIX_WIDTH-1)||page>(OLED_PAGES-1))
    {
        return;
    }

    pages=pic->px.height/8;
    if((pic->px.height%8)!=0)
    {
        pages++;
    }
    for(i=0;i<pages;i++)
    {
        for(j=i,k=0;j<pages*pic->px.width;j+=pages,k++) 
        {  
            if((col+k)>=OLED_PIX_WIDTH)
            {
                break;
            }
            p_oled_frame_next[(page+i)*64+col+k]=Byte_Swapbit(pic->data[j]);
        }
    }
    
} 

void OLED_px10x16_num(uint8_t page,uint8_t col,void * num,bool adapting)
{
    uint8_t i;
    int8_t col_offset=0;
    pic_block_t dis;
    for(i=0;(*(uint8_t *)num != '\0')&&i<6 ;i++,num=((uint8_t *)num)+1)
    {  
        if((*(uint8_t *)num == ' '))
        {
            if(adapting)
            {
                col_offset += 6;
            }
            else
            {
                col_offset += 12;
            }
        }
        else
        {
            dis.px.width = 10;
            dis.px.height = 16;
            dis.data = (uint8_t *)px10x16_num[(*(uint8_t *)num-'0')];
            OLED_Picture(page,col+col_offset,&dis);
            col_offset += 12;
        }
    }
}

void OLED_px6x16_num(uint8_t page,uint8_t col,void * num)
{
    uint8_t i;
    int8_t col_offset=0;
    pic_block_t dis;
    for(i=0;(*(uint8_t *)num != '\0')&&i<8 ;i++,num=((uint8_t *)num)+1)
    {  
        if((*(uint8_t *)num == ' '))
        {
            col_offset += 8;
        }
        else
        {
            dis.px.width = 6;
            dis.px.height = 16;
            dis.data = (uint8_t *)px6x16_num[(*(uint8_t *)num-'0')];
            OLED_Picture(page,col+col_offset,&dis);
            col_offset += 8;
        }
    }
}

 void test_OLED()
{
//    pic_block_t hao={16,16,(uint8_t *)px16x16_Hao};
//    pic_block_t ji={16,16,(uint8_t *)px16x16_Ji};
//    pic_block_t bu={16,16,(uint8_t *)px16x16_Bu};
//    pic_block_t pic={24,24,(uint8_t *)px24x24_happy};
//    OLED_Picture(1,0,&pic);
//    OLED_Picture(1,26,&ji);
//    OLED_Picture(1,44,&bu);
////    OLED_Picture(1,20,&hao);
////    OLED_Picture(2,40,&hao);
//    OLED_Message(0,4,"64x32 pix");
////    OLED_Message(3,0,"18:03");
}
