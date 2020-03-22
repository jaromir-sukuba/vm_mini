#include "stm32l1xx.h"

void spi_data (uint8_t data);
void disp_cmd (uint8_t data);
void disp_dat (uint8_t data);
void disp_init (void);
void refresh_lcd(void);
void refresh_lcd_p(void);
void refresh_lcd_line(uint8_t line, uint8_t row);
void print_char (unsigned char data, unsigned char x, unsigned char y, unsigned char invert);
void set_pixel (unsigned char x, unsigned char y, unsigned char color);
void print_char_b (unsigned char data,unsigned char x,unsigned char y,unsigned char invert);
unsigned char print_str (char *dat, unsigned char line, unsigned char off, unsigned char type, unsigned char invert);

void fill_disp_buff (unsigned char data);

#define	SMALL	2
#define	BIG		1
