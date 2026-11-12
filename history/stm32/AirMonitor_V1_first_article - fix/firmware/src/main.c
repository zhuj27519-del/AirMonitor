#include "board.h"
#include "protocol.h"
#include <stdio.h>
volatile uint32_t ticks;
void SysTick_Handler(void){ticks++;}
void delay_ms(uint32_t ms){uint32_t t=ticks;while((uint32_t)(ticks-t)<ms)__WFI();}
typedef struct {volatile uint16_t read,write;volatile uint32_t overflow,errors;uint8_t bytes[1024];} Ring;
static Ring pm_ring,voc_ring;
static void irq(USART_TypeDef *u,Ring *r){
    uint32_t sr=u->ISR;
    if(sr&(USART_ISR_ORE|USART_ISR_FE|USART_ISR_NE|USART_ISR_PE)){r->errors++;u->ICR=USART_ICR_ORECF|USART_ICR_FECF|USART_ICR_NECF|USART_ICR_PECF;}
    if(sr&USART_ISR_RXNE_RXFNE){uint8_t b=(uint8_t)u->RDR;uint16_t w=(r->write+1)&1023;if(w==r->read)r->overflow++;else{r->bytes[r->write]=b;__DMB();r->write=w;}}
}
void USART1_IRQHandler(void){irq(USART1,&pm_ring);}
void USART2_LPUART2_IRQHandler(void){irq(USART2,&voc_ring);}
static int pop(Ring *r){if(r->read==r->write)return -1;int b=r->bytes[r->read];__DMB();r->read=(r->read+1)&1023;return b;}
static void mode(GPIO_TypeDef *g,unsigned p,unsigned m){g->MODER=(g->MODER&~(3u<<(2*p)))|(m<<(2*p));}
static void af(GPIO_TypeDef *g,unsigned p,unsigned a){mode(g,p,2);g->AFR[p/8]=(g->AFR[p/8]&~(15u<<((p%8)*4)))|(a<<((p%8)*4));}
static void setup(void){
    RCC->CR|=RCC_CR_HSION;while(!(RCC->CR&RCC_CR_HSIRDY)){} RCC->CR&=~RCC_CR_HSIDIV;
    SystemCoreClockUpdate();SysTick_Config(SystemCoreClock/1000);
    RCC->IOPENR|=RCC_IOPENR_GPIOAEN|RCC_IOPENR_GPIOBEN;
    RCC->APBENR2|=RCC_APBENR2_SPI1EN|RCC_APBENR2_USART1EN;RCC->APBENR1|=RCC_APBENR1_USART2EN;
    (void)RCC->APBENR2;
    GPIOA->BSRR=1u<<4;mode(GPIOA,4,1);af(GPIOA,5,0);af(GPIOA,7,0);
    for(unsigned p=0;p<3;p++)mode(GPIOB,p,1);
    /* Keep PMS active and reset released. */
    GPIOB->BSRR=(1u<<10)|(1u<<11);mode(GPIOB,10,1);mode(GPIOB,11,1);
    for(unsigned p=3;p<=5;p++){mode(GPIOB,p,0);GPIOB->PUPDR=(GPIOB->PUPDR&~(3u<<(p*2)))|(1u<<(p*2));}
    af(GPIOA,9,1);af(GPIOA,10,1);af(GPIOA,2,1);af(GPIOA,3,1);
    USART1->BRR=(SystemCoreClock+4800)/9600;USART2->BRR=(SystemCoreClock+4800)/9600;
    USART1->CR1=USART_CR1_RE|USART_CR1_TE|USART_CR1_RXNEIE_RXFNEIE|USART_CR1_UE;
    USART2->CR1=USART1->CR1;
    NVIC_EnableIRQ(USART1_IRQn);NVIC_EnableIRQ(USART2_LPUART2_IRQn);
    /* Mode 0, 8-bit, TX-only, 2 MHz at HSI16. */
    SPI1->CR2=(7u<<SPI_CR2_DS_Pos)|SPI_CR2_FRXTH;
    SPI1->CR1=SPI_CR1_MSTR|SPI_CR1_SSM|SPI_CR1_SSI|SPI_CR1_BIDIMODE|SPI_CR1_BIDIOE|(2u<<SPI_CR1_BR_Pos)|SPI_CR1_SPE;
}
static void value(unsigned y,const char *label,int v,const char *unit){char s[21];snprintf(s,sizeof(s),"%s %d %s",label,v,unit);lcd_text(0,y,s,0xffff);}
static void decimal(unsigned y,const char *label,int v,const char *unit){char s[21];unsigned a=(unsigned)(v<0?-v:v);snprintf(s,sizeof(s),"%s %s%u.%u %s",label,v<0?"-":"",a/10,a%10,unit);lcd_text(0,y,s,0xffff);}
int main(void){
    setup();lcd_init();lcd_clear(0);
    PacketStream ps={0},vs={0};PmReading pm={0};VocReading voc={0};
    uint32_t pm_time=0,voc_time=0,draw=0,key_time=0;bool pm_seen=false,voc_seen=false;
    unsigned page=0,last_raw=0,stable=0;int b;
    for(;;){
        while((b=pop(&pm_ring))>=0)if(pm_feed(&ps,(uint8_t)b,&pm)){pm_seen=true;pm_time=ticks;}
        while((b=pop(&voc_ring))>=0)if(voc_feed(&vs,(uint8_t)b,&voc)){voc_seen=true;voc_time=ticks;}
        unsigned raw=(~GPIOB->IDR>>3)&7;
        if(raw!=last_raw){last_raw=raw;key_time=ticks;}
        if((uint32_t)(ticks-key_time)>=30 && raw!=stable){unsigned pressed=raw&~stable;stable=raw;
            if(pressed&1)page=(page+2)%3;if(pressed&2)page=(page+1)%3;if(pressed&4)page=2;draw=ticks-1000;}
        if((uint32_t)(ticks-draw)>=1000){draw=ticks;lcd_clear(0);
            lcd_text(0,0,page==0?"PARTICLES":page==1?"VOC ESTIMATES":"SYSTEM STATUS",0x07ff);
            if(page==0){
                if(ticks<30000)lcd_text(0,30,"WARMUP 30 SECONDS",0xffe0);
                else if(!pm_seen || (uint32_t)(ticks-pm_time)>10000)lcd_text(0,30,"PMS STALE OR MISSING",0xf800);
                else {value(35,"PM1",pm.pm1,"UG/M3");value(65,"PM2.5",pm.pm25,"UG/M3");value(95,"PM10",pm.pm10,"UG/M3");}
            }else if(page==1){
                if(!voc_seen || (uint32_t)(ticks-voc_time)>10000)lcd_text(0,30,"VOC STALE OR MISSING",0xf800);
                else {value(30,"TVOC",voc.tvoc,"UG/M3");value(60,"HCHO EST",voc.hcho,"UG/M3");value(90,"ECO2",voc.eco2,"PPM");decimal(120,"TEMP",voc.temperature,"C");decimal(150,"RH",voc.rh,"PCT");}
                lcd_text(0,195,"ECO2 IS ESTIMATED",0xffe0);lcd_text(0,215,"HCHO CROSS SENSITIVE",0xffe0);
            }else{value(30,"PM BAD",ps.rejected,"");value(60,"VOC BAD",vs.rejected,"");value(90,"RX LOSS",pm_ring.overflow+voc_ring.overflow,"");value(120,"UART ERR",pm_ring.errors+voc_ring.errors,"");lcd_text(0,165,"P0 UNVALIDATED",0xffe0);}
        }
        __WFI();
    }
}
