#include "board.h"
static void pin(GPIO_TypeDef *p,unsigned bit,int high){p->BSRR=1u<<(bit+(high?0:16));}
static void tx(uint8_t b){while(!(SPI1->SR&SPI_SR_TXE)){} *(__IO uint8_t *)&SPI1->DR=b;while(SPI1->SR&SPI_SR_BSY){}}
static void command(uint8_t c,const uint8_t *p,unsigned n){pin(GPIOA,4,0);pin(GPIOB,0,0);tx(c);pin(GPIOB,0,1);while(n--)tx(*p++);pin(GPIOA,4,1);}
static void reg(uint8_t c,uint8_t v){command(c,&v,1);}
void lcd_init(void){
    pin(GPIOB,2,0);pin(GPIOB,1,1);delay_ms(100);pin(GPIOB,1,0);delay_ms(100);pin(GPIOB,1,1);delay_ms(100);
    reg(0x36,0);reg(0x3a,5);
    const uint8_t porch[]={12,12,0,0x33,0x33};command(0xb2,porch,5);
    reg(0xb7,0x35);reg(0xbb,0x19);reg(0xc0,0x2c);reg(0xc2,1);reg(0xc3,0x12);reg(0xc4,0x20);reg(0xc6,0x0f);
    const uint8_t power[]={0xa4,0xa1};command(0xd0,power,2);
    const uint8_t gp[]={0xd0,4,13,17,19,43,63,84,76,24,13,11,31,35};
    const uint8_t gn[]={0xd0,4,12,17,19,44,63,68,81,47,31,31,32,35};
    command(0xe0,gp,14);command(0xe1,gn,14);command(0x21,0,0);command(0x11,0,0);delay_ms(120);command(0x29,0,0);delay_ms(20);pin(GPIOB,2,1);
}
static void rectangle(unsigned x,unsigned y,unsigned w,unsigned h,uint16_t c){
    if(x+w>240 || y+h>240)return;
    uint8_t a[]={x>>8,x,(x+w-1)>>8,x+w-1},b[]={y>>8,y,(y+h-1)>>8,y+h-1};
    command(0x2a,a,4);command(0x2b,b,4);command(0x2c,0,0);pin(GPIOA,4,0);pin(GPIOB,0,1);
    for(unsigned i=0;i<w*h;i++){tx(c>>8);tx(c&255);}pin(GPIOA,4,1);
}
void lcd_clear(uint16_t c){rectangle(0,0,240,240,c);}
/* Original compact 3x5 uppercase/digit font; each row is three bits. */
static const uint16_t glyphs[]={
  0x7b6f,0x2492,0x73e7,0x73cf,0x5bc9,0x79cf,0x79ef,0x7249,0x7bef,0x7bcf,
  0x2bed,0x6bae,0x7927,0x6b6e,0x79e7,0x79e4,0x796f,0x5bed,0x7497,0x126a,
  0x5bad,0x4927,0x5fed,0x5ffd,0x7b6f,0x7be4,0x7b7b,0x7bec,0x79cf,0x7492,
  0x5b6f,0x5b6a,0x5bfd,0x5aad,0x5a92,0x72a7};
void lcd_text(unsigned x,unsigned y,const char *s,uint16_t c){
    for(;*s && x+12<=240;s++,x+=12){uint16_t g=0;char ch=*s;
        if(ch>='0'&&ch<='9')g=glyphs[ch-'0'];else if(ch>='A'&&ch<='Z')g=glyphs[ch-'A'+10];
        else if(ch=='-')g=0x01c0;else if(ch=='.')g=2;else if(ch==':')g=0x0410;
        for(unsigned r=0;r<5;r++)for(unsigned b=0;b<3;b++)rectangle(x+b*3,y+r*3,3,3,(g&(1u<<(14-r*3-b)))?c:0);
    }
}
