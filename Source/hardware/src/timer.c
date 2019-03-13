#include "timer.h"
#include "stm32f10x_tim.h"
#include "user_config.h"
#include "led.h"

unsigned int millisec_tick=0;
unsigned int sec_tick=0;
unsigned int Run_sec_tick=0;
unsigned int rs485_delay_millisec=0;
unsigned int socket_delay_millisec=0;
unsigned int bluetooth_delay_millisec=0;


//通用定时器3中断初始化
//这里时钟选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数
//这里使用的是定时器3!
void TIM3_Int_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //时钟使能

    //定时器TIM3初始化
    TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值
    TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位

    TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); //使能指定的TIM3中断,允许更新中断

    //中断优先级NVIC设置
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
    NVIC_Init(&NVIC_InitStructure);  //初始化NVIC寄存器


    TIM_Cmd(TIM3, ENABLE);  //使能TIMx
}
//定时器3中断服务程序
void TIM3_IRQHandler(void)   //TIM3中断
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  //检查TIM3更新中断发生与否
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  //清除TIMx更新中断标志
        sec_tick++;
        Run_sec_tick++;		
		SYSTEM_LED = ~SYSTEM_LED; 
    }
}

void TIM3_set_tick(unsigned int ticks)
{
    TIM_ITConfig(TIM3,TIM_IT_Update,DISABLE );
    sec_tick=ticks;
    TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE );
}

unsigned int Get_SysTick(void)
{
    return sec_tick;
}

//通用定时器3中断初始化
//这里时钟选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数
//这里使用的是定时器4!
void TIM4_Int_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); //时钟使能

    //定时器TIM3初始化
    TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值
    TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位

    TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE ); //使能指定的TIM3中断,允许更新中断

    //中断优先级NVIC设置
    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;  //TIM3中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
    NVIC_Init(&NVIC_InitStructure);  //初始化NVIC寄存器

    TIM_Cmd(TIM4, ENABLE);  //使能TIMx
}
//定时器4中断服务程序
void TIM4_IRQHandler(void)   //TIM3中断
{
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)  //检查TIM3更新中断发生与否
	{
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update  );  //清除TIMx更新中断标志 
		millisec_tick++;
		
		if(socket_delay_millisec)socket_delay_millisec--;
		if(bluetooth_delay_millisec)bluetooth_delay_millisec--;
		if(rs485_delay_millisec)rs485_delay_millisec--;	
	}
}

unsigned int Get_SysmilliTick(void)
{
    return millisec_tick;
}

unsigned int Get_SysRunTick(void)
{
    return Run_sec_tick;
}

void Set_MillisecTimer_Rs485_delay(unsigned int delay)
{
    rs485_delay_millisec=delay;
}

unsigned int sensor_rs485_delay_millisec=0;

void Set_MillisecTimer_Sensor_Rs485_delay(unsigned int delay)
{
    sensor_rs485_delay_millisec=delay;
}

void Set_MillisecTimer_Bluetooth_delay(unsigned int delay)
{
    bluetooth_delay_millisec=delay;
}

unsigned int Get_MillisecTimer_Bluetooth_delay(void)
{
    return bluetooth_delay_millisec;
}
void Set_MillisecTimer_NBIOT_delay(unsigned int delay)
{
    socket_delay_millisec=delay;
}
unsigned int Get_MillisecTimer_NBIOT_delay(void)
{
    return socket_delay_millisec;
}

