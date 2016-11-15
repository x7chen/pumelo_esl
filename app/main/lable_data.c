#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "fs_esl_data.h"
#include "ble_esls.h"
#define ONE_PACKET_DATA_LENGTH 16
uint16_t sequency = 0;
bool is_transporting = 0;
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
    esl_flash_store(sequency*ONE_PACKET_DATA_LENGTH,
			        get_packet_data_aligned(packet),
			        4,
			        NULL);
    sequency++;
}
/*****************************************************************************
 * 0x00:初始化,设定数据长度和CRC16，启动传输过程
 * 0x01:结束传输，验证数据完整性，并返回成功标志
*****************************************************************************/
void esl_cp_handle(ble_esls_t * p_esls, uint8_t * control_point,uint16_t length)
{
    switch(control_point[0])
    {
        case 0x00:
            sequency = 0;
            break;
        case 0x01:
            break;
        default:
            break;
    }
}
void esl_cp_resp(ble_esls_t * p_esls, uint8_t * resp,uint16_t length)
{
    
}

