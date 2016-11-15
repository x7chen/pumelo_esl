#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "fs_esl_data.h"
#include "ble_esls.h"
#define ONE_PACKET_DATA_LENGTH 16
uint16_t sequency = 0;
uint8_t __attribute((aligned (4))) aligned_data[20];

uint16_t get_packet_index(uint8_t * packet)
{
    return (packet[0]*256 + packet[1]);
}
uint16_t get_packet_length(uint8_t * packet)
{
    return (packet[2]);
}
uint8_t * get_packet_data_aligned(uint8_t * packet)
{
    memcpy(aligned_data,packet+4,ONE_PACKET_DATA_LENGTH);
    return aligned_data;
}
void esl_packet_handle(ble_esls_t * p_esls, uint8_t * packet,uint16_t length)
{
    if(get_packet_index(packet)!=sequency)
    {
        return;
    }
    esl_flash_store(	sequency*ONE_PACKET_DATA_LENGTH,
			get_packet_data_aligned(packet),
			4,
			NULL);
    sequency++;
}
void esl_cp_handle(ble_esls_t * p_esls, uint8_t * control_point,uint16_t length)
{
    
}
void esl_cp_resp(ble_esls_t * p_esls, uint8_t * resp,uint16_t length)
{
    
}

