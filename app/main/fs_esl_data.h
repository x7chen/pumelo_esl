#ifndef  _FS_ESL_DATA_H_
#define  _FS_ESL_DATA_H_

#include "fstorage.h"

typedef fs_cb_t esl_flash_callback_t;

uint32_t esl_flash_init(bool sd_enabled);
fs_ret_t esl_flash_store(uint32_t const * p_dest, uint32_t const * const p_src, uint32_t len_words, esl_flash_callback_t callback);
fs_ret_t esl_flash_erase(uint32_t const * p_dest, uint32_t num_pages, esl_flash_callback_t callback);
void esl_flash_error_clear(void);
fs_ret_t esl_flash_wait(void);


#endif
