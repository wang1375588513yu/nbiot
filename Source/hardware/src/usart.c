
#include "sys.h"
#include "usart.h"
#include "timer.h"

//////////////////////////////////////////////////////////////////////////////////
//���ʹ��ucos,����������ͷ�ļ�����.
#if SYSTEM_SUPPORT_UCOS
#include "includes.h"					//ucos ʹ��	  
#endif

#if ENABLE_USART1   //���ʹ���˽���
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���

//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ

Pipe_t uart_pipe1;

void reset_uart_pipe1()
{
    memset((char*)&uart_pipe1,0,sizeof(Pipe_t));
}

//��ʼ��IO ����1
//bound:������
void uart1_init(u32 bound)
{
    //GPIO�˿�����
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��USART1��GPIOAʱ��
    USART_DeInit(USART1);  //��λ����1
    //USART1_TX   PA.9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
    GPIO_Init(GPIOA, &GPIO_InitStructure); //��ʼ��PA9

    //USART1_RX	  PA.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
    GPIO_Init(GPIOA, &GPIO_InitStructure);  //��ʼ��PA10

    //USART ��ʼ������
    USART_InitStructure.USART_BaudRate = bound;//һ������Ϊ9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
    USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
    USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

    USART_Init(USART1, &USART_InitStructure); //��ʼ������
#if ENABLE_USART1		  //���ʹ���˽���  
    //Usart1 NVIC ����
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
    NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//�����ж�
#endif
    USART_Cmd(USART1, ENABLE);                    //ʹ�ܴ���
    reset_uart_pipe1();
}

void USART1_IRQHandler(void)                	//����1�жϷ������
{
    u8 Res;
#ifdef OS_TICKS_PER_SEC	 	//���ʱ�ӽ�����������,˵��Ҫʹ��ucosII��.
    OSIntEnter();
#endif
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
    {
        Res =USART_ReceiveData(USART1);//(USART1->DR);	//��ȡ���յ�������
        uart_pipe1.pipe_buf[uart_pipe1.in_pos]=Res;
        uart_pipe1.in_pos++;
        uart_pipe1.in_pos%=PIPE_BUF_LEN;
        if(PIPE_BUF_LEN > uart_pipe1.sum)
            uart_pipe1.sum++;
        else uart_pipe1.out_pos=uart_pipe1.in_pos;
        uart_pipe1.time=Get_SysmilliTick();
    }
#ifdef OS_TICKS_PER_SEC	 	//���ʱ�ӽ�����������,˵��Ҫʹ��ucosII��.
    OSIntExit();
#endif
}

void uart1_printf(char * string, ...)
{
    __va_list arg;
    int len=0;
    if(string==NULL)  return;

    if(strlen(string)>=UART_TX_LEN)return;
    va_start(arg, string);
    memset(_txBuffer,0,UART_TX_LEN);
    len=vsprintf((char *)_txBuffer, string, arg); /*must use "vsprintf" */
    va_end(arg);
    uart1_output((const unsigned char *)_txBuffer,len);
}

void uart1_output(const unsigned char * buf,int len)
{
    unsigned int __i;

    if(buf==NULL ||len==0)  return;
    for(__i = 0; __i < len; __i++)
    {
        while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET) {};
        USART_SendData(USART1,(uint8_t) buf[__i]);
    }
}

#endif

#if ENABLE_USART2   //���ʹ���˽���
#define USART2_RX_BUFER_SIZE 1024
unsigned char USART2_RX_BUFER[USART2_RX_BUFER_SIZE];
Pipe1_t uart_pipe2={USART2_RX_BUFER,USART2_RX_BUFER_SIZE,0,0,0,0};

#define USART2_RX_LOG_BUFER_SIZE 200
unsigned char USART2_RX_LOG_BUFER[USART2_RX_LOG_BUFER_SIZE];
Pipe1_t uart_pipe2_log={USART2_RX_LOG_BUFER,USART2_RX_LOG_BUFER_SIZE,0,0,0,0};

void reset_uart_pipe2()
{
	Pipe1_Reset(&uart_pipe2_log);
	Pipe1_Reset(&uart_pipe2);
}

void uart2_init(u32 bound)
{
    //GPIO�˿�����
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��USART2��GPIOAʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    USART_DeInit(USART2);  //��λ����2

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA.2
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;//GPIO_Mode_Out_OD;	//�����������
    GPIO_Init(GPIOA, &GPIO_InitStructure); //��ʼ��PA2

    //USART2_RX	  PA.3
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
    GPIO_Init(GPIOA, &GPIO_InitStructure);  //��ʼ��PA3

    //USART ��ʼ������
    USART_InitStructure.USART_BaudRate = bound;//һ������Ϊ9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
    USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
    USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

    USART_Init(USART2, &USART_InitStructure); //��ʼ������
#if ENABLE_USART2		  //���ʹ���˽���  
    //Usart1 NVIC ����
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
    NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���

    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//�����ж�
#endif
    USART_Cmd(USART2, ENABLE);                    //ʹ�ܴ���
    reset_uart_pipe2();
}

void uart2_output(const unsigned char * buf,int len)
{
    unsigned int __i;

    if(buf==NULL ||len==0)  return;
    for(__i = 0; __i < len; __i++)
    {
        while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET) {};
        USART_SendData(USART2,(uint8_t) buf[__i]);
    }
}

void uart2_printf(char * string, ...)
{
    va_list arg;
    int len=0;
    if(string==NULL)  return;
    if(strlen(string)>=UART_TX_LEN)return;

    va_start(arg, string);
    memset(_txBuffer,0,UART_TX_LEN+1);
    len=vsprintf((char *)_txBuffer, string, arg); /*must use "vsprintf" */
    va_end(arg);
    uart2_output((const unsigned char *)_txBuffer,len);
}

void USART2_IRQHandler(void)                	//����2�жϷ������
{
    u8 Res;
#ifdef OS_TICKS_PER_SEC	 	//���ʱ�ӽ�����������,˵��Ҫʹ��ucosII��.
    OSIntEnter();
#endif
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
    {
        Res =USART_ReceiveData(USART2);//(USART2->DR);	//��ȡ���յ�������
        Pipe_Block_Input(&uart_pipe2,(const char*)&Res,1);
		Pipe_Block_Input(&uart_pipe2_log,(const char*)&Res,1);
    }
#ifdef OS_TICKS_PER_SEC	 	//���ʱ�ӽ�����������,˵��Ҫʹ��ucosII��.
    OSIntExit();
#endif
}
#endif

#if ENABLE_USART3   //���ʹ���˽���
Pipe_t uart_pipe3;
Pipe_t uart_send_pipe3;

void reset_uart_pipe3(void)
{
    memset((char*)&uart_pipe3,0,sizeof(Pipe_t));
	memset((char*)&uart_send_pipe3,0,sizeof(Pipe_t));
}
//��ʼ��IO ����1
//bound:������
void uart3_init(u32 bound)
{
    //GPIO�˿�����
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//ʹ��USART2��GPIOAʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    USART_DeInit(USART3);  //��λ����1

//USART3_TX	  PB.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PA.2
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//��������//GPIO_Mode_AF_OD;	//�����������GPIO_Mode_AF_OD;//
    GPIO_Init(GPIOB, &GPIO_InitStructure); //��ʼ��PA2

    //USART3_RX	  PB.11
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
    GPIO_Init(GPIOB, &GPIO_InitStructure);  //��ʼ��PA3
    //USART ��ʼ������

    USART_InitStructure.USART_BaudRate = bound;//һ������Ϊ9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
    USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
    USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

    USART_Init(USART3, &USART_InitStructure); //��ʼ������
#if ENABLE_USART3		  //���ʹ���˽���  
    //Usart1 NVIC ����
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
    NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���

    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//�����ж�
#endif
    USART_Cmd(USART3, ENABLE);                    //ʹ�ܴ���
    reset_uart_pipe3();
}

void uart3_printf(char * string, ...)
{
    va_list arg;
    int len=0;
    if(string==NULL)  return;
    if(strlen(string)>=UART_TX_LEN)
        return;

    va_start(arg, string);
    memset(_txBuffer,0,UART_TX_LEN+1);
    len=vsprintf((char *)_txBuffer, string, arg); /*must use "vsprintf" */
    va_end(arg);
    uart3_output((const unsigned char *)_txBuffer,len);
}

void uart3_output(const unsigned char * buf,int len)
{
    unsigned int __i;

    if(buf==NULL ||len==0)  return;
    for(__i = 0; __i < len; __i++)
    {
        while(USART_GetFlagStatus(USART3,USART_FLAG_TC)==RESET)
        {};
        USART_SendData(USART3,(uint8_t) buf[__i]);
    }
}

void USART3_IRQHandler(void)                	//����1�жϷ������
{
    u8 Res;
#ifdef OS_TICKS_PER_SEC	 	//���ʱ�ӽ�����������,˵��Ҫʹ��ucosII��.
    OSIntEnter();
#endif
    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
    {
        Res =USART_ReceiveData(USART3);//(USART2->DR);	//��ȡ���յ�������

        uart_pipe3.pipe_buf[uart_pipe3.in_pos]=Res;
        uart_pipe3.in_pos++;
        uart_pipe3.in_pos%=PIPE_BUF_LEN;
        if(PIPE_BUF_LEN > uart_pipe3.sum)
            uart_pipe3.sum++;
        else uart_pipe3.out_pos=uart_pipe3.in_pos;
        uart_pipe3.time=Get_SysmilliTick();

    }
#ifdef OS_TICKS_PER_SEC	 	//���ʱ�ӽ�����������,˵��Ҫʹ��ucosII��.
    OSIntExit();
#endif
}

#endif


#if ENABLE_USART4
#define USART4_RX_BUFER_SIZE 10240
unsigned char USART4_RX_BUFER[USART4_RX_BUFER_SIZE];
Pipe1_t usart_pipe4={USART4_RX_BUFER,USART4_RX_BUFER_SIZE,0,0,0,0};

#define USART4_RX_LOG_BUFER_SIZE 200
unsigned char USART4_RX_LOG_BUFER[USART4_RX_LOG_BUFER_SIZE];
Pipe1_t uart_pipe4_log={USART4_RX_LOG_BUFER,USART4_RX_LOG_BUFER_SIZE,0,0,0,0};

void reset_uart_pipe4()
{
	Pipe1_Reset(&uart_pipe4_log);
	Pipe1_Reset(&usart_pipe4);
}

void uart4_init(u32 bound)
{
    //GPIO�˿�����
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	//ʹ��USART2��GPIOAʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);

    USART_DeInit(UART4);  //��λ����1

//UART4_TX	  PC.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PA.2
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//��������//GPIO_Mode_AF_OD;	//�����������GPIO_Mode_AF_OD;//
    GPIO_Init(GPIOC, &GPIO_InitStructure); //��ʼ��PA2

    //UART4_RX	  PC.11
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
    //GPIO_InitStructure.GPIO_Mode =GPIO_Mode_IPU;
    GPIO_Init(GPIOC, &GPIO_InitStructure);  //��ʼ��PA3

    //USART ��ʼ������
    USART_InitStructure.USART_BaudRate = bound;//һ������Ϊ9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
    USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
    USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

    USART_Init(UART4, &USART_InitStructure); //��ʼ������
 
    //Usart1 NVIC ����
    NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
    NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���

    USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);//�����ж�
    USART_Cmd(UART4, ENABLE);                    //ʹ�ܴ���
	reset_uart_pipe4();
}


void uart4_printf(char * string, ...)
{
    __va_list arg;
    int len=0;
    if(string==NULL)  return;

	memset((void*)GetBuffer(),0,PUBLIC_SEND_BUFSIZE);
    va_start(arg, string);
    
    len=vsnprintf((char *)GetBuffer(),PUBLIC_SEND_BUFSIZE, string, arg); /*must use "vsprintf" */
    va_end(arg);
    uart4_output((const unsigned char *)GetBuffer(),len);
}
void uart4_output(const unsigned char * buf,int len)
{
    unsigned int __i,timespan;
    timespan=Get_SysmilliTick();
    if(buf==NULL ||len==0)  return;
    for(__i = 0; __i < len; __i++)
    {
        while(USART_GetFlagStatus(UART4,USART_FLAG_TC)==RESET)
        {
            //��ʱ�����������
            if(Get_SysmilliTick()-timespan>10)
            {
                break;
            }
        }
        timespan=Get_SysmilliTick();
        USART_SendData(UART4,(uint8_t) buf[__i]);
    }
}


void UART4_IRQHandler(void)                	//����1�жϷ������
{
    u8 Res;
#ifdef OS_TICKS_PER_SEC	 	//���ʱ�ӽ�����������,˵��Ҫʹ��ucosII��.
    OSIntEnter();
#endif
    if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
    {

        Res =USART_ReceiveData(UART4);//(USART1->DR);	//��ȡ���յ�������
        Pipe_Block_Input(&usart_pipe4,(const char*)&Res,1);
		Pipe_Block_Input(&uart_pipe4_log,(const char*)&Res,1);
    }
#ifdef OS_TICKS_PER_SEC	 	//���ʱ�ӽ�����������,˵��Ҫʹ��ucosII��.
    OSIntExit();
#endif
}
#endif


#if ENABLE_USART5
#define USART5_RX_BUFER_SIZE 10250
unsigned char USART5_RX_BUFER[USART5_RX_BUFER_SIZE];
Pipe1_t uart_pipe5={USART5_RX_BUFER,USART5_RX_BUFER_SIZE,0,0,0,0};

#define USART5_RX_LOG_BUFER_SIZE 200
unsigned char USART5_RX_LOG_BUFER[USART5_RX_LOG_BUFER_SIZE];
Pipe1_t uart_pipe5_log={USART5_RX_LOG_BUFER,USART5_RX_LOG_BUFER_SIZE,0,0,0,0};

void reset_uart_pipe5()
{
	Pipe1_Reset(&uart_pipe5_log);
	Pipe1_Reset(&uart_pipe5);
}

void uart5_init(u32 bound)
{
    //GPIO�˿�����
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	//ʹ��USART2��GPIOAʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);

    USART_DeInit(UART5);  //��λ����1

//UART5_TX	  PC.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//��������//GPIO_Mode_AF_OD;	//�����������GPIO_Mode_AF_OD;//
    GPIO_Init(GPIOC, &GPIO_InitStructure); //��ʼ��PA2

    //UART5_RX	  PC.11
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
    GPIO_Init(GPIOD, &GPIO_InitStructure);  //��ʼ��PA3

    //USART ��ʼ������
    USART_InitStructure.USART_BaudRate = bound;//һ������Ϊ9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
    USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
    USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

    USART_Init(UART5, &USART_InitStructure); //��ʼ������
 
    //Usart1 NVIC ����
    NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
    NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���

    USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);//�����ж�
    USART_Cmd(UART5, ENABLE);                    //ʹ�ܴ���
	reset_uart_pipe5();
}

void uart5_printf(char * string, ...)
{
    __va_list arg;
    int len=0;
    if(string==NULL)  return;

	memset((void*)GetBuffer(),0,PUBLIC_SEND_BUFSIZE);
    va_start(arg, string);
    len=vsnprintf((char *)GetBuffer(),PUBLIC_SEND_BUFSIZE, string, arg); /*must use "vsprintf" */
    va_end(arg);
    uart5_output((const unsigned char *)GetBuffer(),len);
}
void uart5_output(const unsigned char * buf,int len)
{
    unsigned int __i,timespan;
    timespan=Get_SysmilliTick();
    if(buf==NULL ||len==0)  return;
    for(__i = 0; __i < len; __i++)
    {
        while(USART_GetFlagStatus(UART5,USART_FLAG_TC)==RESET)
        {//��ʱ�����������            
            if(Get_SysmilliTick()-timespan>10)
            {
                break;
            }
        }
        timespan=Get_SysmilliTick();
        USART_SendData(UART5,(uint8_t) buf[__i]);
    }
}


void UART5_IRQHandler(void)                	//����1�жϷ������
{
    u8 Res;
#ifdef OS_TICKS_PER_SEC	 	//���ʱ�ӽ�����������,˵��Ҫʹ��ucosII��.
    OSIntEnter();
#endif
    if(USART_GetITStatus(UART5, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
    {
        Res =USART_ReceiveData(UART5);//(USART1->DR);	//��ȡ���յ�������
        Pipe_Block_Input(&uart_pipe5,(const char*)&Res,1);
		Pipe_Block_Input(&uart_pipe5_log,(const char*)&Res,1);
    }
#ifdef OS_TICKS_PER_SEC	 	//���ʱ�ӽ�����������,˵��Ҫʹ��ucosII��.
    OSIntExit();
#endif
}
#endif

void uart_init(USART_TypeDef* USARTx,u32 bound)
{
#if ENABLE_USART1
    if(USARTx == USART1)
    {
        uart1_init(bound);
    }
#endif

#if ENABLE_USART2
    if(USARTx == USART2)
    {
        uart2_init(bound);
    }
#endif
#if ENABLE_USART3
    if(USARTx == USART3)
    {
        uart3_init(bound);
    }
#endif

#if ENABLE_USART4
    if(USARTx == UART4)
    {
        uart4_init(bound);
    }
#endif
#if ENABLE_USART5
    if(USARTx == UART5)
    {
        uart5_init(bound);
    }
#endif
}

void *get_txBuffer(void)
{
    return &_txBuffer;
}

