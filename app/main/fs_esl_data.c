/* Copyright (c) 2016 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#include "fs_esl_data.h"
#include "softdevice_handler.h"
#include "nrf_log.h"

#ifdef SOFTDEVICE_PRESENT
// Only include fstorage if SD interaction is required
#include "fstorage.h"
#endif

#define FLASH_FLAG_NONE                 (0)
#define FLASH_FLAG_OPER                 (1<<0)
#define FLASH_FLAG_FAILURE_SINCE_LAST   (1<<1)
#define FLASH_FLAG_SD_ENABLED           (1<<2)

static uint32_t m_flags;

#ifdef BLE_STACK_SUPPORT_REQD

#define ESL_DATA_START_ADDRESS 		0x30000
#define ESL_DATA_END_ADDRESS		0x34000 
// Function prototypes
static void fs_evt_handler(fs_evt_t const * const evt, fs_ret_t result);

FS_REGISTER_CFG(fs_config_t fs_esl_config) =
{
    .callback       = fs_evt_handler,            // Function for event callbacks.
    .p_start_addr   = (uint32_t*)ESL_DATA_START_ADDRESS,
    .p_end_addr     = (uint32_t*)ESL_DATA_END_ADDRESS
};


static void fs_evt_handler(fs_evt_t const * const evt, fs_ret_t result)
{
    // Clear the operation flag
    m_flags &= ~FLASH_FLAG_OPER;

    if (result == FS_SUCCESS)
    {
        // Clear flag for ongoing operation and failure since last
        m_flags &= ~FLASH_FLAG_FAILURE_SINCE_LAST;
    }
    else
    {
        NRF_LOG_INFO("Generating failure\r\n");
        m_flags |= FLASH_FLAG_FAILURE_SINCE_LAST;
    }

    if (evt->p_context)
    {
        //lint -e611
        ((esl_flash_callback_t)evt->p_context)(evt, result);
    }
}

#endif


uint32_t esl_flash_init(bool sd_enabled)
{
    m_flags = FLASH_FLAG_SD_ENABLED;
    return NRF_SUCCESS;
}


fs_ret_t esl_flash_store(uint32_t const * p_dest, uint32_t const * const p_src, uint32_t len_words, esl_flash_callback_t callback)
{
    fs_ret_t ret_val = FS_SUCCESS;

    if ((m_flags & FLASH_FLAG_SD_ENABLED) != 0)
    {
        // Check if there is a pending error
        if ((m_flags & FLASH_FLAG_FAILURE_SINCE_LAST) != 0)
        {
            NRF_LOG_INFO("Flash: Failure since last\r\n");
            return FS_ERR_FAILURE_SINCE_LAST;
        }

        // Set the flag to indicate ongoing operation
        m_flags |= FLASH_FLAG_OPER;
        //lint -e611
        ret_val = fs_store(&fs_esl_config, p_dest, p_src, len_words, (void*)callback);

        if (ret_val != FS_SUCCESS)
        {
            NRF_LOG_INFO("Flash: failed %d\r\n", ret_val);
            return ret_val;
        }

        // Set the flag to indicate ongoing operation
        m_flags |= FLASH_FLAG_OPER;
    }

    return ret_val;
}


/** @brief Internal function to initialize DFU BLE transport
 */
fs_ret_t esl_flash_erase(uint32_t const * p_dest, uint32_t num_pages, esl_flash_callback_t callback)
{
    fs_ret_t ret_val = FS_SUCCESS;
    NRF_LOG_INFO("Erasing: 0x%08x, num: %d\r\n", (uint32_t)p_dest, num_pages);

    if ((m_flags & FLASH_FLAG_SD_ENABLED) != 0)
    {
        // Check if there is a pending error
        if ((m_flags & FLASH_FLAG_FAILURE_SINCE_LAST) != 0)
        {
            NRF_LOG_INFO("Erase: Failure since last\r\n");
            return FS_ERR_FAILURE_SINCE_LAST;
        }

        m_flags |= FLASH_FLAG_OPER;
        ret_val = fs_erase(&fs_esl_config, p_dest, num_pages, (void*)callback);

        if (ret_val != FS_SUCCESS)
        {
            NRF_LOG_INFO("Erase failed: %d\r\n", ret_val);
            m_flags &= ~FLASH_FLAG_OPER;
            return ret_val;
        }

        // Set the flag to indicate ongoing operation
        m_flags |= FLASH_FLAG_OPER;
    }

    return ret_val;
}


void esl_flash_error_clear(void)
{
    m_flags &= ~FLASH_FLAG_FAILURE_SINCE_LAST;
}


fs_ret_t esl_flash_wait(void)
{
    NRF_LOG_INFO("Waiting for finished...\r\n");

#ifdef BLE_STACK_SUPPORT_REQD
    if ((m_flags & FLASH_FLAG_SD_ENABLED) != 0)
    {
        while ((m_flags & FLASH_FLAG_OPER) != 0)
        {
            (void)sd_app_evt_wait();
        }

        if ((m_flags & FLASH_FLAG_FAILURE_SINCE_LAST) != 0)
        {
            NRF_LOG_INFO("Failure since last\r\n");
            return FS_ERR_FAILURE_SINCE_LAST;
        }
    }
#endif

    NRF_LOG_INFO("After wait!\r\n");
    return FS_SUCCESS;
}
