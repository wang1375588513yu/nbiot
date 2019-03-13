/********************************Copyright (c)**********************************\
**
**                   (c) Copyright 2018, WangYu, China, SZ.
**                           All Rights Reserved
**
**                        By(深圳市北斗云信息科技有限公司)
**
**----------------------------------File Information----------------------------
** FileName:      led.c
** The Author:    WangYu
** Creation Date: 2018-08-28
** Description:
**
**--------------------------------Version Information---------------------------
** Version NO: V1.0
** Version Description: Initial Version
**
**------------------------------------------------------------------------------
\********************************End of Head************************************/

#include "led.h"
#include "timer.h"

void user_led_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(LED_RGB_RCC | LED_SYSTEM_RCC, ENABLE);//使能GPIOA时钟
    RCC_APB2PeriphClockCmd(LED_RCC, ENABLE);//使能GPIOA时钟

    GPIO_InitStructure.GPIO_Pin = RED_LED_PIN | BLUE_LED_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
    GPIO_Init(LED_RGB_GPIO, &GPIO_InitStructure);					 //根据设定参数初始化GPIOA6,A7

    GPIO_InitStructure.GPIO_Pin = SYSTEM_LED_PIN ;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
    GPIO_Init(LED_SYSTEM_GPIO, &GPIO_InitStructure);				//根据设定参数初始化GPIOA6,A7

    RED_LED   = LED_OFF;
    SYSTEM_LED = LED_SYSTEM_ON;
    BLUE_LED  = LED_OFF;
}

unsigned int led_milliloop_delay = 0;
static void user_led_milliloop(void)
{
//    static unsigned int num  = 0;
    static unsigned int softimersec=0xffffffff;
    if(softimersec==Get_SysmilliTick())return;
    softimersec=Get_SysmilliTick();

    if(led_milliloop_delay)
    {
        led_milliloop_delay--;
        return;
    }
    else
    {
		#if 0
        num++;
        led_milliloop_delay = 500;
        if(num == 1 || num == 2 || num == 6)
        {
            RED_LED = LED_RGB_ON;
        }
        else
        {
            RED_LED = LED_RGB_OFF;
        }
        if(num == 2 || num == 3 || num == 4)
            GREEN_LED = LED_RGB_ON;
        else
            GREEN_LED = LED_RGB_OFF;

        if(num == 4 || num == 5 || num == 6)
            BLUE_LED = LED_RGB_ON;
        else
            BLUE_LED = LED_RGB_OFF;

		if(num == 7)
		{
			RED_LED   = LED_RGB_ON;
			GREEN_LED = LED_RGB_ON;
			BLUE_LED  = LED_RGB_ON;
		}
        if(num == 8)
        {
            RED_LED   = LED_RGB_OFF;
            GREEN_LED = LED_RGB_OFF;
            BLUE_LED  = LED_RGB_OFF;
            num = 0;
        }
		#endif
    }

}

static void user_led_loop(void)
{
    static unsigned int softimersec=0xffffffff;
    if(softimersec==Get_SysTick())return;
    softimersec=Get_SysTick();

	RED_LED = ~RED_LED;
	BLUE_LED = ~BLUE_LED;
}

void user_led_task(void)
{
    user_led_milliloop();
    user_led_loop();
}


