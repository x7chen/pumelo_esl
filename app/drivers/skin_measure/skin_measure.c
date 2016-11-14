#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "app_error.h"
#include "nrf_delay.h"
#include "nrf_drv_timer.h"
#include "nrf_drv_gpiote.h"
#include "indication.h"
#include "app_timer.h"
#include "app_scheduler.h"
#include "skin_measure.h"
#include "bytewise.h"

#define SKIN_MEASURE_OUT_PIN        23
#define SKIN_MEASURE_PREPARE_PIN    22
#define SKIN_MEASURE_POWER_PIN      21

#define MEAS_DEVIATION              200
#define APP_TIMER_PRESCALER     0
#define SKIN_MEAS_INTERVAL      APP_TIMER_TICKS(50, APP_TIMER_PRESCALER)
#define SKIN_MEAS_TIMER_FREQUENCY   (16000000)
#define MEASURE_PREPARE(ON)     do{\
                                    nrf_gpio_cfg_output(SKIN_MEASURE_PREPARE_PIN);\
                                    if(ON==1) \
                                    {nrf_gpio_pin_set(SKIN_MEASURE_PREPARE_PIN);}\
                                    else \
                                    {nrf_gpio_pin_clear(SKIN_MEASURE_PREPARE_PIN);}}while(0)

#define MEASURE_POWER(ON)     do{\
                                    nrf_gpio_cfg_output(SKIN_MEASURE_POWER_PIN);\
                                    if(ON==1) \
                                    {nrf_gpio_pin_set(SKIN_MEASURE_POWER_PIN);}\
                                    else \
                                    {nrf_gpio_pin_clear(SKIN_MEASURE_POWER_PIN);}}while(0)

app_timer_id_t   m_skin_meas_timer_id;
const nrf_drv_timer_t TIMER_SKIN_MEASRUE = NRF_DRV_TIMER_INSTANCE(1);
bool measure_data_updated = false;
bool Skin_measure_is_Initialized = false;
bool Skin_measure_is_Idle = true; 
static uint32_t single_value=0;                                    
static uint32_t measure_value=0;
static uint32_t measure_base_value= 300 ; 
static uint8_t meas_mode = 0;                                    
skin_meas_callbacks_t *Skin_meas_callbacks = NULL;
void skin_measure_handle(uint8_t mode);                                     
void timer_skin_measure_event_handler(nrf_timer_event_t event_type, void* p_context)
{

}
void skin_measure_int_handle()
{
    single_value = nrf_drv_timer_capture(&TIMER_SKIN_MEASRUE,NRF_TIMER_CC_CHANNEL0);
    measure_value += single_value;
    measure_data_updated = true;
    MEASURE_PREPARE(0);
    
}
uint32_t skin_measure_int_on()
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
    nrf_drv_gpiote_in_config_t gpiote_config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
    gpiote_config.pull = NRF_GPIO_PIN_NOPULL;
    nrf_drv_gpiote_in_init(SKIN_MEASURE_OUT_PIN, &gpiote_config, (nrf_drv_gpiote_evt_handler_t)skin_measure_int_handle);
    nrf_drv_gpiote_in_event_enable(SKIN_MEASURE_OUT_PIN, true);
    return err_code;
}
uint32_t skin_measure_int_off()
{
    uint32_t err_code = 0;
    nrf_drv_gpiote_in_event_disable(SKIN_MEASURE_OUT_PIN);
    nrf_drv_gpiote_in_uninit(SKIN_MEASURE_OUT_PIN);
    return err_code;
}
void skin_measure_init()
{
    uint32_t err_code =0;
    MEASURE_POWER(1);
    nrf_delay_ms(5);
    err_code = nrf_drv_timer_init(&TIMER_SKIN_MEASRUE, NULL, timer_skin_measure_event_handler);
    APP_ERROR_CHECK(err_code);
    nrf_drv_timer_enable(&TIMER_SKIN_MEASRUE);
    skin_measure_int_on();
    Skin_measure_is_Initialized = true;
}
void skin_measure_uninit()
{
    MEASURE_POWER(0);
    nrf_gpio_cfg_default(SKIN_MEASURE_PREPARE_PIN);
    nrf_drv_timer_disable(&TIMER_SKIN_MEASRUE);
    nrf_drv_timer_uninit(&TIMER_SKIN_MEASRUE);
    
    skin_measure_int_off();
    Skin_measure_is_Initialized = false;
}

void skin_meas_timer_handle(void * p_context)
{
    skin_measure_handle(meas_mode|0x01);
}
uint32_t skin_measure_timer_start()
{
    uint32_t err_code =0;
    static bool skin_meas_timer_is_initialized = false;
    if(!skin_meas_timer_is_initialized)
    {
        err_code = app_timer_create(&m_skin_meas_timer_id,APP_TIMER_MODE_SINGLE_SHOT,skin_meas_timer_handle);
        APP_ERROR_CHECK(err_code);
        skin_meas_timer_is_initialized = true;
    }
    err_code = app_timer_start(m_skin_meas_timer_id,SKIN_MEAS_INTERVAL,NULL);
    APP_ERROR_CHECK(err_code);
    
    
    return err_code;
}
uint16_t Constrain(uint16_t value)
{
	uint16_t limit=0;
    static uint16_t pre_value = 0;
    limit = (uint16_t)abs((int)(pre_value-value));
	if((limit>10)&&(limit<30))
    {
		return pre_value;
    }
	else
    {
        pre_value = value;
		return value;
    }
}

uint16_t compute_skin_oil(uint32_t time)
{
    float skin_oil;
    skin_oil = 3.8209*log((float)time/16)+1.4388;
    return (uint16_t)(skin_oil*10);   
}

uint16_t compute_skin_water(uint32_t time)
{
    float skin_water;
    uint16_t result;
    skin_water = 11.712*log((float)time/16)-7.9093;
    result = Constrain((uint16_t)(skin_water*10));
    if(result>990) {result = 990;}
    return result;
}

/*****************************************************************************
* -----||----------50ms-----------|-----------50ms-----------|-----------  ...
*       ^                        
*1.开启测量指示灯                 ^
*2.开始充电                       | 
*3.开启定时器                     |
*                            1.开始放电
*                            2.读数据
*                            3.开始充电
*                            4.下一轮定时
******************************************************************************
* mode = 0x00 : 测量开始
* mode = 0x01 ：测量进行中
* mode = 0x10 : 校准开始
* mode = 0x11 : 校准进行中
*****************************************************************************/
void skin_measure_handle(uint8_t mode)
{
    static uint32_t index=0;
    uint16_t skin_water;
    uint16_t skin_oil;
    uint8_t buffer[4];
    
    if((mode&0x0f) == 0)
    {
        index=0;
    }
    if(!Skin_measure_is_Initialized)
    {
        skin_measure_init();
    }
    
    if(index == 0)
    {
        //测量开始，指示灯提示
        indication(INDICATION_SKIN_MEASURE);
        skin_measure_timer_start();
        MEASURE_PREPARE(0);
        measure_value = 0;
        index++;
        return;
    }
    if(index < 20)
    {
        index++;
        //开启下一次测量
        skin_measure_timer_start();
        nrf_drv_timer_clear(&TIMER_SKIN_MEASRUE);
        MEASURE_PREPARE(1);
        
    }
    else if(index == 20)
    {
        index++;
        //测量完成，指示灯提示
        indication(INDICATION_SKIN_MEASURE_COMPLETE);
        measure_value /= 20;
        if((mode & 0xF0) == SKIN_MEAS_MODE_CAL)
        {
            measure_base_value = measure_value;
            index = 0;
            skin_measure_uninit();
            Skin_measure_is_Idle = true; 
            return;
        }
        if(Skin_meas_callbacks != NULL)
        {
            if(measure_value<(measure_base_value+MEAS_DEVIATION))
            {
                Skin_meas_callbacks->on_skin_not_touch(NULL,0);
            }
            else
            {
                measure_value -= (measure_base_value-50);
                skin_oil = compute_skin_oil(measure_value);
                skin_water = compute_skin_water(measure_value);
                buffer[0] = (skin_water>>8)&0xff;
                buffer[1] = (skin_water)&0xff;
                buffer[2] = (skin_oil>>8)&0xff;
                buffer[3] = (skin_oil)&0xff;
                Skin_meas_callbacks->on_meas_complete(buffer,4);
            }
        }  
        //继续测量，直到皮肤离开为止
        skin_measure_timer_start();
        nrf_drv_timer_clear(&TIMER_SKIN_MEASRUE);
        MEASURE_PREPARE(1);
    }
    else
    {
        index++;
        do
        {
            //离开皮肤
            if(single_value<(measure_base_value+MEAS_DEVIATION))
            {
                index = 0;
                skin_measure_uninit();
                Skin_measure_is_Idle = true;
                break;
            }
            //继续测量，直到皮肤离开为止
            skin_measure_timer_start();
            nrf_drv_timer_clear(&TIMER_SKIN_MEASRUE);
            MEASURE_PREPARE(1);
        }while(0);
        
    }
}

void skin_measure(uint8_t mode)
{
    if(Skin_measure_is_Idle)
    {
        meas_mode = mode;
        skin_measure_handle(meas_mode);
        Skin_measure_is_Idle = false;
    }
    else
    {
        Skin_meas_callbacks->on_skin_not_leave(NULL,0);
    }
}

void skin_meas_callback_is_null(void * content, uint16_t length)
{
    
}
void register_skin_meas_callbacks(skin_meas_callbacks_t *callbacks)
{
    Skin_meas_callbacks = callbacks;
    if(Skin_meas_callbacks->on_meas_complete == NULL)
    {
        Skin_meas_callbacks->on_meas_complete = skin_meas_callback_is_null;
    }
    if(Skin_meas_callbacks->on_skin_not_touch == NULL)
    {
        Skin_meas_callbacks->on_skin_not_touch = skin_meas_callback_is_null;
    }
    if(Skin_meas_callbacks->on_skin_not_leave == NULL)
    {
        Skin_meas_callbacks->on_skin_not_leave = skin_meas_callback_is_null;
    }
}

