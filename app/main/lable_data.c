#include <stdint.h>
#include <stdbool.h>
#inlcude "fs_esl_data.h"
uint16_t sequency = 0;
uint16_t get_packet_index(uint8_t * packet)
{
    return (packet[0]*256 + packet[1]);
}
uint16_t get_packet_length(uint8_t * packet)
{
    return (packet[2]);
}
void esl_packet_handle(uint8_t * packet)
{
    if(get_packet_index(packet)!=sequency)
    {
        return;
    }
    esl_flash_store()    

}
