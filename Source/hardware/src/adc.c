#include "adc.h"

#define ADC_SAMPLE_COUNT 10
#define ADC_CHANNAL_COUNT 2

vu16 AD_Value[ADC_SAMPLE_COUNT][ADC_CHANNAL_COUNT];

//��ʼ��ADC
void MYDMA_Config(DMA_Channel_TypeDef* DMA_CHx,u32 cpar,u32 cmar,u16 cndtr)
{
    DMA_InitTypeDef DMA_InitStructure;
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//ʹ��DMAʱ��
    DMA_DeInit(DMA_CHx);   //��DMA��ͨ��1�Ĵ�������Ϊȱʡֵ

    DMA_InitStructure.DMA_PeripheralBaseAddr = cpar;  //DMA�������ַ
    DMA_InitStructure.DMA_MemoryBaseAddr = cmar;  //DMA�ڴ����ַ
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //���ݴ��䷽�򣬴����跢�͵��ڴ�  DMA_CCRXλ4
    DMA_InitStructure.DMA_BufferSize = cndtr;  //DMAͨ����DMA����Ĵ�С
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //�����ַ�Ĵ�������
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //�ڴ��ַ�Ĵ�������
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  //�������ݿ��Ϊ16λ
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //�ڴ����ݿ��Ϊ16λ
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;  //������ѭ������ģʽ
    DMA_InitStructure.DMA_Priority = DMA_Priority_High; //DMAͨ�� xӵ�������ȼ�
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��
    DMA_Init(DMA_CHx, &DMA_InitStructure);  //����DMA_InitStruct��ָ���Ĳ�����ʼ��DMA��ͨ��USART1_Tx_DMA_Channel����ʶ�ļĴ���
}

void  Adc_Init(void)
{
    ADC_InitTypeDef ADC_InitStructure;
//    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC |RCC_APB2Periph_ADC1| RCC_APB2Periph_AFIO,ENABLE );//ʹ�ܶ˿�1��ʱ�Ӻ�ADC1��ʱ�ӣ���ΪADC1��ͨ��1��PA10��

    RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //����ADC��Ƶ����6 72M/6=12M,ADC���ʱ�䲻�ܳ���14M��Ҳ����ADC��ʱ��Ƶ��Ϊ12MHz

	#if 0
    //PC5 ��Ϊģ��ͨ����������
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//ģ����������
    GPIO_Init(GPIOC, &GPIO_InitStructure);
	#endif

    ADC_DeInit(ADC1);  //��λADC1,������ ADC1 ��ȫ���Ĵ�������Ϊȱʡֵ
    ADC_TempSensorVrefintCmd(ENABLE);//ʹ���ڲ��¶ȴ�����
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//����ʵ��ʹ�õ���ADC1����ADC1�����ڶ���ģʽADC_CR1��λ19:16
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;	//ADC_ScanConvMode ���������Ƿ���ɨ��ģʽ����ʵ�鿪��ɨ��ģʽ.ADC_CR1��λ8
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;	//ADC_ContinuousConvMode ���������Ƿ�������ת��ģʽ ģ��ת������������ת��ģʽ��ADC_CR2��λ1
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//ת��������������ⲿ�������� ADC_CR2��λ19:17
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC�����Ҷ���ADC_CR2��λ11
    ADC_InitStructure.ADC_NbrOfChannel = 2;	//˳����й���ת����ADCͨ������ĿADC_SQR1λ23:20
    ADC_Init(ADC1, &ADC_InitStructure);	//����ADC_InitStruct��ָ���Ĳ�����ʼ������ADCx�ļĴ���

    ADC_RegularChannelConfig(ADC1, ADC_Channel_15, 1, ADC_SampleTime_239Cycles5 );//ADC1��ADC1ͨ��1����1ת��������ʱ��Ϊ239.5����
    ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 2, ADC_SampleTime_239Cycles5 );//ADC1��ADC1ͨ��1����2ת��������ʱ��Ϊ239.5����

    ADC_DMACmd(ADC1, ENABLE); //ʹ��ADC1��DMA����
    ADC_Cmd(ADC1, ENABLE);	//ʹ�ܵ�ADC1,ADC_CR2λ0

    ADC_ResetCalibration(ADC1);	//ʹ�ܸ�λУ׼��ADC_CR2λ3
    while(ADC_GetResetCalibrationStatus(ADC1));	//�ȴ���λУ׼����

    ADC_StartCalibration(ADC1);	 //����ADУ׼��ADC_CR2λ2
    while(ADC_GetCalibrationStatus(ADC1));	 //�ȴ�У׼����

    MYDMA_Config(DMA1_Channel1,(u32)&ADC1->DR,(u32)&AD_Value,ADC_SAMPLE_COUNT*ADC_CHANNAL_COUNT);//DMA1ͨ��1������ΪADC1���洢��ΪAD_DATA��ͨ����ΪN��.
    DMA_Cmd(DMA1_Channel1, ENABLE);//����DMAͨ��
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);//�������ADת��
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

//�õ��¶�ֵ
//����ֵ:�¶�ֵ(��λ:��.)
float Get_CPU_Temprate(void)
{
    u32 adcx;
    float temperate;
    adcx=Get_Adc_Average(ADC_Channel_16);	//��ȡͨ��16�ڲ��¶ȴ�����ͨ��,10��ȡƽ��
    temperate=(float)adcx*(3.3/4096);		//��ѹֵ
    temperate=(temperate-1.43)/0.0043 + 25; //ת��Ϊ�¶�ֵ
    return temperate;
}

