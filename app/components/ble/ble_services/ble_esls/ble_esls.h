
#ifndef BLE_ESLS_H__
#define BLE_ESLS_H__

#include "ble.h"
#include "ble_srv_common.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BLE_UUID_ESLS_SERVICE 0x1800                     
#define BLE_ESLS_MAX_DATA_LEN (GATT_MTU_SIZE_DEFAULT - 3) 

typedef struct ble_esls_s ble_esls_t;

typedef void (*ble_esls_handler_t) (ble_esls_t * p_esls, uint8_t * p_data, uint16_t length);

typedef struct
{
    ble_esls_handler_t packet_handler; 
    ble_esls_handler_t cp_handler; 
} ble_esls_init_t;

struct ble_esls_s
{
    uint8_t                  	uuid_type;               
    uint16_t                 	service_handle;         
    ble_gatts_char_handles_t 	cp_char_handles;             
    ble_gatts_char_handles_t 	packet_char_handles;            
    uint16_t                 	conn_handle;           
    ble_esls_handler_t       	packet_handler;     
    ble_esls_handler_t       	cp_handler;     
    bool                     	is_notification_enabled; 
};

uint32_t ble_esls_init(ble_esls_t * p_esls, const ble_esls_init_t * p_esls_init);

void ble_esls_on_ble_evt(ble_esls_t * p_esls, ble_evt_t * p_ble_evt);

uint32_t ble_esls_cp_update(ble_esls_t * p_esls, uint8_t * p_string, uint16_t length);

#ifdef __cplusplus
}
#endif

#endif // BLE_ESLS_H__

/** @} */
