#include <stdint.h>
#include <string.h>
#include "app_error.h"
#include "app_scheduler.h"
#include "softdevice_handler.h"
#include "app_timer_appsh.h"
#include "nrf_delay.h"
#include "nrf_drv_gpiote.h"
#include "bles.h"
#include "packet_parser.h"
#include "clock.h"
#include "alarm.h"
#include "wdt.h"
#include "app_uart.h"
#include "bsp.h"
#include "app_drivers.h"
#include "fs_esl_data.h"

#define APP_TIMER_PRESCALER             0                               /**< Value of the RTC1 PRESCALER register. */                           /**< Maximum number of simultaneously created timers. */
#define APP_TIMER_OP_QUEUE_SIZE         4                               /**< Size of timer operation queues. */
#define SCHED_MAX_EVENT_DATA_SIZE       sizeof(app_timer_event_t)       /**< Maximum size of scheduler events. Note that scheduler BLE stack events do not contain any data, as the events are being pulled from the stack in the event handler. */
#define SCHED_QUEUE_SIZE                10                              /**< Maximum number of events in the scheduler queue. */

static clock_callbacks_t clock_callbacks;

void second_handler(uint32_t second)
{
    wdt_feed();
    
}
void minute_handle(uint32_t minute)
{
    check_alarm();
}
void hour_handle(uint32_t hour)
{

}

static void timers_init(void)
{
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, false);
}
/**@brief Function for starting timers.
*/
static void register_clock_event(void)
{
    clock_callbacks.on_second_update =  (clock_callback_t)second_handler;
    clock_callbacks.on_minute_update = (clock_callback_t)minute_handle;
    clock_callbacks.on_hour_update = (clock_callback_t)hour_handle;
    register_clock_callbacks(&clock_callbacks);
}


static void scheduler_init(void)
{
    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
}

/**@brief Function for the Power manager.
 */
static void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}
static void bsp_event_handler(bsp_event_t evt)
{ 
    switch (evt)
    {
        case BSP_EVENT_KEY_0:
            //env_data_request_process(NULL,0);
            
//            LEDS_OFF();
            break;
        case BSP_EVENT_KEY_1:
//            firmware_version_request_process(NULL,0);
//            LEDS_ON();
            break;
        default:
            APP_ERROR_HANDLER(evt);
            break;
    }
}

/**@brief Function for initializing bsp module.
 */
static void bsp_module_init(void)
{
    uint32_t err_code;
    // Note: If the only use of buttons is to wake up, bsp_event_handler can be NULL.
    err_code = bsp_init(BSP_INIT_LED | BSP_INIT_BUTTONS, APP_TIMER_TICKS(100, APP_TIMER_PRESCALER), bsp_event_handler);
    APP_ERROR_CHECK(err_code);
}
/**@brief Function for application main entry.
 */
int main(void)
{
//    uint32_t err_code;
    nrf_delay_ms(500);
                                        
    nrf_drv_gpiote_uninit();
    timers_init();
    scheduler_init();
    ble_init();
    system_clock_init();
    bsp_module_init();
    wdt_init(); 
    wdt_start();
    register_clock_event();
    // Enter main loop
    printf("\r\nstart...");
    	 esl_flash_init(true);
	ssd1608_init();
    for (;;)
    {
        app_sched_execute();
        power_manage();
    }
}
