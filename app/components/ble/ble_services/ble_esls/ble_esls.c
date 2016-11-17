
#include "sdk_config.h"
#define BLE_ESLS_ENABLED 1
#if BLE_ESLS_ENABLED
#include "ble_esls.h"
#include "ble_srv_common.h"
#include "sdk_common.h"

#define BLE_UUID_ESLS_CP_CHARACTERISTIC 	0x1801                      
#define BLE_UUID_ESLS_DATA_CHARACTERISTIC 	0x1802                      

#define BLE_ESLS_MAX_CP_CHAR_LEN        	BLE_ESLS_MAX_DATA_LEN        
#define BLE_ESLS_MAX_DATA_CHAR_LEN        	BLE_ESLS_MAX_DATA_LEN       
#define ESLS_BASE_UUID  {{0xDB,0x45,0x5A,0xAF,0x93,0xE0,0xDA,0xBC,0xC6,0x40,0x00,0xAF,0x00,0x00,0x1A,0xDA}} 

static void on_connect(ble_esls_t * p_esls, ble_evt_t * p_ble_evt)
{
    p_esls->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}

static void on_disconnect(ble_esls_t * p_esls, ble_evt_t * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_esls->conn_handle = BLE_CONN_HANDLE_INVALID;
}

static void on_write(ble_esls_t * p_esls, ble_evt_t * p_ble_evt)
{
    ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    if (
        (p_evt_write->handle == p_esls->cp_char_handles.cccd_handle)
        &&
        (p_evt_write->len == 2)
       )
    {
        if (ble_srv_is_notification_enabled(p_evt_write->data))
        {
            p_esls->is_notification_enabled = true;
        }
        else
        {
            p_esls->is_notification_enabled = false;
        }
    }
    else if (
             (p_evt_write->handle == p_esls->packet_char_handles.value_handle)
             &&
             (p_esls->packet_handler != NULL)
            )
    {
        p_esls->packet_handler(p_esls, p_evt_write->data, p_evt_write->len);
    }
    else if (
             (p_evt_write->handle == p_esls->cp_char_handles.value_handle)
             &&
             (p_esls->cp_handler != NULL)
            )
    {
        p_esls->cp_handler(p_esls, p_evt_write->data, p_evt_write->len);
    }
    else
    {

    }
}

static uint32_t cp_char_add(ble_esls_t * p_esls, const ble_esls_init_t * p_esls_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.notify = 1;
    char_md.char_props.write  = 1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_esls->uuid_type;
    ble_uuid.uuid = BLE_UUID_ESLS_CP_CHARACTERISTIC;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(uint8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = BLE_ESLS_MAX_CP_CHAR_LEN;

    return sd_ble_gatts_characteristic_add(p_esls->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_esls->cp_char_handles);
}

static uint32_t packet_char_add(ble_esls_t * p_esls, const ble_esls_init_t * p_esls_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.write         = 1;
    char_md.char_props.write_wo_resp = 1;
    char_md.p_char_user_desc         = NULL;
    char_md.p_char_pf                = NULL;
    char_md.p_user_desc_md           = NULL;
    char_md.p_cccd_md                = NULL;
    char_md.p_sccd_md                = NULL;

    ble_uuid.type = p_esls->uuid_type;
    ble_uuid.uuid = BLE_UUID_ESLS_DATA_CHARACTERISTIC;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = 1;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = BLE_ESLS_MAX_DATA_CHAR_LEN;

    return sd_ble_gatts_characteristic_add(p_esls->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_esls->packet_char_handles);
}


void ble_esls_on_ble_evt(ble_esls_t * p_esls, ble_evt_t * p_ble_evt)
{
    if ((p_esls == NULL) || (p_ble_evt == NULL))
    {
        return;
    }

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_esls, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_esls, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_esls, p_ble_evt);
            break;

        default:
            break;
    }
}


uint32_t ble_esls_init(ble_esls_t * p_esls, const ble_esls_init_t * p_esls_init)
{
    uint32_t      err_code;
    ble_uuid_t    ble_uuid;
    ble_uuid128_t esls_base_uuid = ESLS_BASE_UUID;

    VERIFY_PARAM_NOT_NULL(p_esls);
    VERIFY_PARAM_NOT_NULL(p_esls_init);

    p_esls->conn_handle             = BLE_CONN_HANDLE_INVALID;
    p_esls->packet_handler          = p_esls_init->packet_handler;
    p_esls->cp_handler              = p_esls_init->cp_handler;
    p_esls->is_notification_enabled = false;

    err_code = sd_ble_uuid_vs_add(&esls_base_uuid, &p_esls->uuid_type);
    VERIFY_SUCCESS(err_code);

    ble_uuid.type = p_esls->uuid_type;
    ble_uuid.uuid = BLE_UUID_ESLS_SERVICE;

    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &ble_uuid,
                                        &p_esls->service_handle);
    VERIFY_SUCCESS(err_code);

    err_code = cp_char_add(p_esls, p_esls_init);
    VERIFY_SUCCESS(err_code);

    err_code = packet_char_add(p_esls, p_esls_init);
    VERIFY_SUCCESS(err_code);

    return NRF_SUCCESS;
}


uint32_t ble_esls_cp_update(ble_esls_t * p_esls, uint8_t * p_string, uint16_t length)
{
    ble_gatts_hvx_params_t hvx_params;

    VERIFY_PARAM_NOT_NULL(p_esls);

    if ((p_esls->conn_handle == BLE_CONN_HANDLE_INVALID) || (!p_esls->is_notification_enabled))
    {
        return NRF_ERROR_INVALID_STATE;
    }

    if (length > BLE_ESLS_MAX_DATA_LEN)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    memset(&hvx_params, 0, sizeof(hvx_params));

    hvx_params.handle = p_esls->cp_char_handles.value_handle;
    hvx_params.p_data = p_string;
    hvx_params.p_len  = &length;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;

    return sd_ble_gatts_hvx(p_esls->conn_handle, &hvx_params);
}

#endif //BLE_ESLS_ENABLED
