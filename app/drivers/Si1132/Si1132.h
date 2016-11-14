#ifndef _SI1132_H_
#define _SI1132_H_

typedef void (*si1132_callback_t)(void * content, uint16_t length);
typedef struct{
    si1132_callback_t on_UV_update;
}si1132_callbacks_t;

uint32_t Si1132_INT_ON(void);
void Si1132_Init(void);
void register_si1132_callbacks(si1132_callbacks_t *callbacks);
uint32_t Si1132_get_UV_Index(uint8_t *rd_val);

#endif
