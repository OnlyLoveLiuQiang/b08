#include "key.h"
#include "stm32f10x_gpio.h"
#include "delay.h"
#include "led.h"

extern unsigned char mode;

static void pa0_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
}

static void pb6_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
}

void key_init(void)
{
	pa0_init();
	pb6_init();
}

void key_on(void)
{
	led_off(4);
	while(KEY_ON != 0);
	SW_ON;
	if(KEY_ON == 0){
		delay_ms(1000);
		delay_ms(1000);
		delay_ms(1000);
		if(KEY_ON == 0){
			mode = 1;
			led_on(4);
		}
	}
}

void key_off(void)
{
	SW_OFF;
}
