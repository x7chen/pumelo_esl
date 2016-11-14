#ifndef _AW9523_H_
#define _AW9523_H_

#define LED1_COLOR_RED          0x0100
#define LED1_COLOR_GREEN        0x0001
#define LED1_COLOR_BLUE         0x0002
#define LED2_COLOR_RED          0x0004 
#define LED2_COLOR_GREEN        0x0008
#define LED2_COLOR_BLUE         0x0010
#define LED3_COLOR_RED          0x0020
#define LED3_COLOR_GREEN        0x0040
#define LED3_COLOR_BLUE         0x0080 

void LEDS_SET(uint16_t leds);
void LEDS_ON(void);
void LEDS_OFF(void);
#endif
