#include "adc.h"

#define ADC_SAMPLE_COUNT 10
#define ADC_CHANNAL_COUNT 2

vu16 AD_Value[ADC_SAMPLE_COUNT][ADC_CHANNAL_COUNT];

//初始化ADC
void MYDMA_Config(DMA_Channel_TypeDef* DMA_CHx,u32 cpar,u32 cmar,u16 cndtr)
{
    DMA_InitTypeDef DMA_InitStructure;
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//使能DMA时钟
    DMA_DeInit(DMA_CHx);   //将DMA的通道1寄存器重设为缺省值

    DMA_InitStructure.DMA_PeripheralBaseAddr = cpar;  //DMA外设基地址
    DMA_InitStructure.DMA_MemoryBaseAddr = cmar;  //DMA内存基地址
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //数据传输方向，从外设发送到内存  DMA_CCRX位4
    DMA_InitStructure.DMA_BufferSize = cndtr;  //DMA通道的DMA缓存的大小
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //外设地址寄存器不变
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //内存地址寄存器递增
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  //外设数据宽度为16位
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //内存数据宽度为16位
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;  //工作在循环缓存模式
    DMA_InitStructure.DMA_Priority = DMA_Priority_High; //DMA通道 x拥有中优先级
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMA通道x没有设置为内存到内存传输
    DMA_Init(DMA_CHx, &DMA_InitStructure);  //根据DMA_InitStruct中指定的参数初始化DMA的通道USART1_Tx_DMA_Channel所标识的寄存器
}

void  Adc_Init(void)
{
    ADC_InitTypeDef ADC_InitStructure;
//    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC |RCC_APB2Periph_ADC1| RCC_APB2Periph_AFIO,ENABLE );//使能端口1的时钟和ADC1的时钟，因为ADC1的通道1在PA10上

    RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //设置ADC分频因子6 72M/6=12M,ADC最大时间不能超过14M，也就是ADC的时钟频率为12MHz

	#if 0
    //PC5 作为模拟通道输入引脚
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//模拟输入引脚
    GPIO_Init(GPIOC, &GPIO_InitStructure);
	#endif

    ADC_DeInit(ADC1);  //复位ADC1,将外设 ADC1 的全部寄存器重设为缺省值
    ADC_TempSensorVrefintCmd(ENABLE);//使能内部温度传感器
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//本次实验使用的是ADC1，并ADC1工作在独立模式ADC_CR1的位19:16
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;	//ADC_ScanConvMode 用来设置是否开启扫描模式，本实验开启扫面模式.ADC_CR1的位8
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;	//ADC_ContinuousConvMode 用来设置是否开启连续转换模式 模数转换工作在连续转换模式，ADC_CR2的位1
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//转换由软件而不是外部触发启动 ADC_CR2的位19:17
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC数据右对齐ADC_CR2的位11
    ADC_InitStructure.ADC_NbrOfChannel = 2;	//顺序进行规则转换的ADC通道的数目ADC_SQR1位23:20
    ADC_Init(ADC1, &ADC_InitStructure);	//根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器

    ADC_RegularChannelConfig(ADC1, ADC_Channel_15, 1, ADC_SampleTime_239Cycles5 );//ADC1；ADC1通道1；第1转换；采样时间为239.5周期
    ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 2, ADC_SampleTime_239Cycles5 );//ADC1；ADC1通道1；第2转换；采样时间为239.5周期

    ADC_DMACmd(ADC1, ENABLE); //使能ADC1的DMA传输
    ADC_Cmd(ADC1, ENABLE);	//使能的ADC1,ADC_CR2位0

    ADC_ResetCalibration(ADC1);	//使能复位校准，ADC_CR2位3
    while(ADC_GetResetCalibrationStatus(ADC1));	//等待复位校准结束

    ADC_StartCalibration(ADC1);	 //开启AD校准，ADC_CR2位2
    while(ADC_GetCalibrationStatus(ADC1));	 //等待校准结束

    MYDMA_Config(DMA1_Channel1,(u32)&ADC1->DR,(u32)&AD_Value,ADC_SAMPLE_COUNT*ADC_CHANNAL_COUNT);//DMA1通道1；外设为ADC1；存储器为AD_DATA；通道数为N个.
    DMA_Cmd(DMA1_Channel1, ENABLE);//启动DMA通道
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);//软件启动AD转换
}

u16 Get_Adc_Average(u16 nChannel)     //
{
    unsigned int adc=0;
    char i=0;

    for(i=0; i<ADC_SAMPLE_COUNT; i++)
    {
        if(nChannel==ADC_Channel_10)
        {
            adc+= AD_Value[i][0];
        }
        else if(nChannel==ADC_Channel_16)
        {
            adc+= AD_Value[i][1];
        }
    }
    return (adc/ADC_SAMPLE_COUNT);
}

//得到温度值
//返回值:温度值(单位:℃.)
float Get_CPU_Temprate(void)
{
    u32 adcx;
    float temperate;
    adcx=Get_Adc_Average(ADC_Channel_16);	//读取通道16内部温度传感器通道,10次取平均
    temperate=(float)adcx*(3.3/4096);		//电压值
    temperate=(temperate-1.43)/0.0043 + 25; //转换为温度值
    return temperate;
}

