#include "nbiot_power.h"
#include "timer.h"
#include "PUBLIC.H"
#include "sys.h"

#define NBIOT_POWER_RESET_DELAY 2


bool nbiot_power_reset_event=true;
unsigned int nbiot_power_reset_delay=0xFFFFFFFF;
bool nbiot_power_enable=false;

void nbiot_power_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(NBIOT_POWER_RCC, ENABLE);//ʹ��GPIOAʱ��

    GPIO_InitStructure.GPIO_Pin = NBIOT_POWER_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
    GPIO_Init(NBIOT_POWER_GPIO, &GPIO_InitStructure);					 //�����趨������ʼ��GPIOA6,A7

    GPIO_Init(NBIOT_POWER_GPIO, &GPIO_InitStructure);					 //�����趨������ʼ��
    nbiot_power_reset();
}

void nbiot_power_off(void)
{
    GPIO_ResetBits(NBIOT_POWER_GPIO,NBIOT_POWER_PIN);
    nbiot_power_enable=false;
}
void nbiot_power_on(void)
{
    GPIO_SetBits(NBIOT_POWER_GPIO,NBIOT_POWER_PIN);
    nbiot_power_enable=true;
}

void nbiot_power_reset(void)
{
    nbiot_power_reset_event = true;
    nbiot_power_reset_delay = NBIOT_POWER_RESET_DELAY;
    //��Դʹ�ܽ�����
    nbiot_power_off();
}

void nbiot_power_loop(void)
{
    if(nbiot_power_reset_event)
    {
        if(nbiot_power_reset_delay)nbiot_power_reset_delay--;
        else
        {
            //ʹ�ܽ�����
            nbiot_power_reset_event=false;
            nbiot_power_reset_delay=0xFFFFFFFF;
            nbiot_power_on();
        }
    }

}

bool nbiot_power_isenable(void)
{
    return  nbiot_power_enable;
}
