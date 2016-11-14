#ifndef OLED_DRV_H
#define OLED_DRV_H
#include "stdint.h"

typedef struct
{
    uint8_t width;
    uint8_t height;
}px_t;
typedef struct
{
    px_t px;
    uint8_t *data;
}pic_block_t;
typedef struct
{
    px_t px;
    uint8_t **data;
}font_t;
typedef struct
{
    uint32_t ui_id;
    uint8_t **data;
}UI_t;
#define OLED_ON_MS          10000

extern app_timer_id_t   m_oled_timer_id;
extern app_timer_id_t   m_oled_scrolling_id;
extern uint8_t oled_cnt;
void OLED_WriteCommand(uint8_t command);
void OLED_WriteLongCommand(uint8_t *command,uint8_t length);
void OLED_Init(void);
void OLED_ON(void);
void OLED_OFF(void);
void m_oled_handler(void * p_context);
void m_oled_scrolling_handler(void * p_context);
void set_scroll_en(bool val);
bool get_scroll_en(void);
void OLED_ON_TIME(uint32_t * ms, uint16_t event_size);
bool OLED_STATUS(void);
void OLED_Clear(void);
void OLED_Display(void);
void OLED_Message(uint8_t page,uint8_t col,void * msg);
void OLED_Picture(uint8_t page,uint8_t col,pic_block_t *pic);
void OLED_px10x16_num(uint8_t page,uint8_t col,void * num,bool adapting);
void OLED_px6x16_num(uint8_t page,uint8_t col,void * num);

void OLED_DBG_View(void);
void test_OLED(void);
#endif
