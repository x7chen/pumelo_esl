#ifndef _SSD1608_H_
#define _SSD1608_H_
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "app_error.h"
#include "app_util_platform.h"
#include "nrf_drv_spi.h"

void ssd1608_init(void);
void WRITE_LUT(void);
void display_image(unsigned char num);
void READBUSY(void);
#endif
