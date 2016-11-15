#ifndef _LABLE_DATA_H_
#define _LABLE_DATA_H_
#include <stdint.h>
#include "ble_esls.h"
typedef struct
{
    uint16_t length;
    uint16_t crc16;
}esl_data_t;

void esl_packet_handle(ble_esls_t * p_esls, uint8_t * packet,uint16_t length);
void esl_cp_handle(ble_esls_t * p_esls, uint8_t * control_point,uint16_t length);
void esl_cp_resp(ble_esls_t * p_esls, uint8_t * resp,uint16_t length);





#endif

