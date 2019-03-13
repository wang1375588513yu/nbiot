#ifndef __USER_LED_H__
#define __USER_LED_H__

#include "sys.h"

void user_led_init(void);
void user_led_task(void);

void POWER_led_on(void);
void POWER_led_off(void);
void GSM_led_on(void);
void GSM_led_off(void);


#endif

