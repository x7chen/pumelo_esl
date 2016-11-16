#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "fs_esl_data.h"
#include "lable_data.h"
#include "ble_esls.h"
#include "crc16.h"

#define TRANS_FLAG_START     (1<<0)
#define TRANS_FLAG_DOING     (1<<1)
#define TRANS_FLAG_DONE      (1<<2)
#define TRANS_FLAG_FAULT     (1<<3)

#define ONE_PACKET_DATA_LENGTH 16
uint16_t sequency = 0;
bool is_transporting = 0;
static uint32_t m_flags;
uint8_t __attribute((aligned (4))) aligned_data[20];
esl_data_t esl_data_info;
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
    if((m_flags & TRANS_FLAG_START)!=0)
    {
        m_flags = TRANS_FLAG_DOING;
        sequency = 0;
    }
    if((m_flags & TRANS_FLAG_DOING) == 0)
    {
        return;
    }
    if(get_packet_index(packet) != sequency)
    {
        return;
    }
    esl_flash_store(sequency*ONE_PACKET_DATA_LENGTH,
			        get_packet_data_aligned(packet),
			        4,
			        NULL);
    sequency++;
}

void esl_cp_resp(ble_esls_t * p_esls, uint8_t * resp,uint16_t length)
{
    ble_esls_cp_update(p_esls,resp,length);
}

uint32_t esl_data_check()
{
    return 0;
    /*
    uint16_t init_crc = 0;
    return (esl_data_info.crc16 == crc16_compute( esl_data_info.start_addr, \
                                                  esl_data_info.length, \
                                                  &init_crc));
    */
}

/* 函数说明：
 * 0x00:初始化,设定数据长度和CRC16，启动传输过程
 * 0x01:结束传输，验证数据完整性，并返回成功标志
 */
void esl_cp_handle(ble_esls_t * p_esls, uint8_t * control_point,uint16_t length)
{
    uint8_t resp[20];
    switch(control_point[0])
    {
        case 0x00:
            esl_data_info.start_addr = get_esl_fs_config()->p_start_addr;
            esl_data_info.length = control_point[1]*256+control_point[2];
            esl_data_info.crc16 =  control_point[3]*256+control_point[4]; 
            esl_flash_erase(0, (1023+esl_data_info.length)/1024, NULL);
            m_flags = TRANS_FLAG_START;
            resp[0] = 0x00;
            resp[1] = 0;
            esl_cp_resp(p_esls, resp,2);    
            break;
        case 0x01:
            if(esl_data_check()==0)
            {
                m_flags = TRANS_FLAG_DONE;
                resp[0] = 0x01;
                resp[1] = 0x00;
                esl_cp_resp(p_esls, resp,length);
            }
            else
            {
                m_flags = TRANS_FLAG_FAULT;
                resp[0] = 0x01;
                resp[1] = 0x01;
                esl_cp_resp(p_esls, resp,length);
            }
            break;
        default:
            break;
    }
}

