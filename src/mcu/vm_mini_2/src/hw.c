#include "hw.h"

void hw_init (void)
    {
	RCC->APB1ENR = RCC->APB1ENR | RCC_APB1ENR_PWREN;
	RCC->APB2ENR = RCC->APB2ENR | RCC_APB2ENR_SYSCFGEN;
    RCC->AHBENR = RCC_AHBENR_GPIOAEN;
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;
    GPIOA->MODER = GPIOA->MODER & ~ (0x03<<2*PE_D_NUM);
    GPIOA->MODER = GPIOA->MODER |   (0x01<<2*PE_D_NUM);
    GPIOA->MODER = GPIOA->MODER & ~ (0x03<<2*PE_A_NUM);
    GPIOA->MODER = GPIOA->MODER |   (0x01<<2*PE_A_NUM);
    GPIOB->MODER = GPIOB->MODER & ~ (0x03<<2*D_BK_NUM);
    GPIOB->MODER = GPIOB->MODER |   (0x01<<2*D_BK_NUM);
    GPIOB->MODER = GPIOB->MODER & ~ (0x03<<2*D_RST_NUM);
    GPIOB->MODER = GPIOB->MODER |   (0x01<<2*D_RST_NUM);
    GPIOB->MODER = GPIOB->MODER & ~ (0x03<<2*D_A0_NUM);
    GPIOB->MODER = GPIOB->MODER |   (0x01<<2*D_A0_NUM);
    GPIOB->MODER = GPIOB->MODER & ~ (0x03<<2*D_CLK_NUM);
    GPIOB->MODER = GPIOB->MODER |   (0x01<<2*D_CLK_NUM);
    GPIOB->MODER = GPIOB->MODER & ~ (0x03<<2*D_CS_NUM);
    GPIOB->MODER = GPIOB->MODER |   (0x01<<2*D_CS_NUM);


    GPIOB->PUPDR = GPIOB->PUPDR |   (0x02<<2*7);
    GPIOB->PUPDR = GPIOB->PUPDR |   (0x02<<2*8);
    GPIOB->PUPDR = GPIOB->PUPDR |   (0x02<<2*9);
    GPIOB->PUPDR = GPIOB->PUPDR |   (0x02<<2*3);
    GPIOB->PUPDR = GPIOB->PUPDR |   (0x02<<2*4);
    GPIOB->PUPDR = GPIOB->PUPDR |   (0x02<<2*5);



    GPIOA->MODER = GPIOA->MODER & ~ (0x03<<2*D_SI_NUM);
    GPIOA->MODER = GPIOA->MODER |   (0x01<<2*D_SI_NUM);
    GPIOA->MODER = GPIOA->MODER & ~ (0x03<<2*X_D1_NUM);
    GPIOA->MODER = GPIOA->MODER |   (0x01<<2*X_D1_NUM);
    GPIOA->MODER = GPIOA->MODER & ~ (0x03<<2*X_D2_NUM);
    GPIOA->MODER = GPIOA->MODER |   (0x01<<2*X_D2_NUM);
    GPIOA->MODER = GPIOA->MODER & ~ (0x03<<2*X_D3_NUM);
    GPIOA->MODER = GPIOA->MODER |   (0x01<<2*X_D3_NUM);
    GPIOA->MODER = GPIOA->MODER & ~ (0x03<<2*A_CT_NUM);
    GPIOA->MODER = GPIOA->MODER |   (0x01<<2*A_CT_NUM);

    GPIOB->MODER = GPIOB->MODER & ~ (0x03<<2*12);
    GPIOB->MODER = GPIOB->MODER |   (0x03<<2*12);


//   GPIOB->MODER ~= ~ (0x03<<2*K_K1_NUM);


    D_CLK_H;
    PE_A_H;
    PE_D_H;
    D_BK_H;

    A_CT_L;

    TIM7->CR1=0x0000;
    TIM7->SR=0;
    TIM7->CNT=0x0000;
    TIM7->PSC=6*999;
    TIM7->ARR=99;
    TIM7->CR1=0x0001;

    usart_init();
    usart2_init();
    adc_init();


    }


void adc_init (void)
	{
	RCC->CR |= 0x01;
	RCC->APB2ENR = RCC->APB2ENR | RCC_APB2ENR_ADC1EN;
	ADC1->CR2 = 0x01;
	}

void adc_uninit (void)
    {
    ADC1->CR2 = 0;
    }

uint16_t adc_get (uint8_t channel)
	{
	ADC1->SQR5 = (channel);
	ADC1->CR2 |= (1<<30);
	while ((ADC1->SR & 0x2)==0);
	return ADC1->DR;
	}

double get_bat_volt (void)
	{
	double temp;
	temp = adc_get(18);
	temp = (temp * 3.3 * (2.4+6.2))/(4095.0*2.4);
	return temp;
	}

void turn_off (void)
	{
	refresh_lcd();
	sr_shift16(0xFFFF);
    PE_D_L;
    PE_A_L;
    D_BK_L;
    SR_CLK_L;
    SR_DAT_L;
    SR_LAT_L;
    usart2_uninit();
    usart_uninit();
    adc_uninit();
    A_CT_L;
    X_D1_L;
    X_D2_L;
    X_D3_L;
    D_CLK_L;
    D_CS_L;
    D_SI_L;
	D_A0_L;
	disp_cmd(0xAC);
	disp_cmd(0x00);
	disp_cmd(0xAE);
	disp_cmd(0xA5);
	SYSCFG->EXTICR[3] = (0b001<<8);
    EXTI->FTSR = (1<<14);
    EXTI->IMR = (1<<14);
	NVIC_SetPriority(EXTI15_10_IRQn,1);
    NVIC_EnableIRQ(EXTI15_10_IRQn);
    PWR->CR = PWR->CR | PWR_CR_LPSDSR;
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    __WFI();
    D_BK_H;
	}


void usart_init (void)
    {
    RCC->APB2ENR = RCC->APB2ENR | RCC_APB2ENR_USART1EN;
    GPIOA->MODER = GPIOA->MODER & ~((0b11<<(2*9)) | (0b11<<(2*10)));
    GPIOA->MODER = GPIOA->MODER | (0b10<<(2*9)) | (0b10<<(2*10));
    USART1->BRR = 6<<4;
    USART1->CR1 = USART_CR1_UE | USART_CR1_RE | USART_CR1_TE;
    GPIOA->AFR[1] = GPIOA->AFR[1] | (0b0111<<(4*1)) | (0b0111<<(4*2));
    USART1->CR1 = USART1->CR1 | USART_CR1_RXNEIE ;
    NVIC_EnableIRQ(USART1_IRQn);
    }

void usart_uninit (void)
    {
    USART1->CR1 = 0;
    }

void usart_send_byte(int8_t data)
    {
    while (((USART1->SR)&USART_SR_TXE)==0);
    USART1->DR = data;
    }


void usart_send_str (char * data)
    {
    while (*data) usart_send_byte(*data++);
    }

void usart2_init (void)
    {
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    GPIOA->MODER = GPIOA->MODER & ~((0b11<<(2*2)) | (0b11<<(2*3)));
    GPIOA->MODER = GPIOA->MODER | (0b10<<(2*2)) | (0b10<<(2*3));
    USART2->BRR = (3<<4)|(4);
    USART2->CR1 = USART_CR1_UE | USART_CR1_RE | USART_CR1_TE;
    GPIOA->AFR[0] = GPIOA->AFR[0] | (0b0111<<(4*3)) | (0b0111<<(4*2));
    USART2->CR1 = USART2->CR1 | USART_CR1_RXNEIE ;
    NVIC_EnableIRQ(USART2_IRQn);
    }

void usart2_uninit (void)
    {
    USART2->CR1 = 0;
    }

void usart2_send_byte(int8_t data)
    {
    while (((USART2->SR)&USART_SR_TXE)==0);
    USART2->DR = data;
    }


void usart2_send_str (char * data)
    {
    while (*data) usart2_send_byte(*data++);
    }

void sr_shift16 (uint16_t data)
    {
    uint8_t i;
    data = data^SR_INVERT_MASK;
    for (i=0;i<16;i++)
	{
	if (data&0x8000)
	    {
	    SR_DAT_H;
	    }
	else
	    {
	    SR_DAT_L;
	    }
	SR_CLK_H;
	data = data<<1;
	SR_CLK_L;
	}
    SR_LAT_H;
    data = data<<1;
    SR_LAT_L;
    }

