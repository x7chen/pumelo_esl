#ifndef _SKIN_MEASURE_H_
#define _SKIN_MEASURE_H_

#include <stdint.h>

#define SKIN_MEAS_MODE_CAL          0x10
#define SKIN_MEAS_MODE_MEAS         0x00

typedef void (*skin_meas_callback_t)(void * content, uint16_t length);
typedef struct{
    skin_meas_callback_t on_meas_complete;
    skin_meas_callback_t on_skin_not_touch;
    skin_meas_callback_t on_skin_not_leave;
}skin_meas_callbacks_t;

uint32_t skin_measure_timer_start(void);
void skin_measure(uint8_t mode);
void get_skin_data(uint8_t * rd_value);
void register_skin_meas_callbacks(skin_meas_callbacks_t *callbacks);
#endif
