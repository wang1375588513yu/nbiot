#include "led.h"
#include "timer.h"
#include "user_led.h"
#include "ota_flash.h"
#include "delay.h"

void user_led_init(void)
{
    LED_Init();
	GSM_LED = LED_ON;	//»√¡Ω∏ˆµ∆¡¡¡Ω√Î
	POWER_LED = LED_ON;
	delay_ms(1000);
}

void GSM_led_on(void)
{
    GSM_LED = LED_ON;
}

void GSM_led_off(void)
{
    GSM_LED = LED_OFF;
}

void POWER_led_on(void)
{
    POWER_LED = LED_ON;
}

void POWER_led_off(void)
{
    POWER_LED = LED_OFF;
}

static void user_led_loop(void)
{
    static unsigned int softimersec=0xffffffff;
    if(softimersec==Get_SysTick())return;
    softimersec=Get_SysTick();

   
}

void user_led_task(void)
{
    user_led_loop();
}

