#pragma once
#include "stm32g0xx.h"
#include <stdint.h>
extern volatile uint32_t ticks;
void delay_ms(uint32_t ms);
void lcd_init(void);
void lcd_clear(uint16_t color);
void lcd_text(unsigned x,unsigned y,const char *text,uint16_t color);
