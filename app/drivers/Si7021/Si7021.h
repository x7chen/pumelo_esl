#ifndef _SI_7021_H_
#define _SI_7021_H_

#include <stdint.h>

uint8_t SI7021_ReadTemperature(uint8_t *Temperature );
uint8_t SI7021_ReadHumidity(uint8_t *Humidity);

#endif
