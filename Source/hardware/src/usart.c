
#include "sys.h"
#include "usart.h"
#include "timer.h"

//////////////////////////////////////////////////////////////////////////////////
//如果使用ucos,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_UCOS
#include "includes.h"					//ucos 使用	  
#endif

#if ENABLE_USART1   //如果使能了接收
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误

//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目

Pipe_t uart_pipe1;

void reset_uart_pipe1()
{
    memset((char*)&uart_pipe1,0,sizeof(Pipe_t));
}

//初始化IO 串口1
//bound:波特率
void uart1_init(u32 bound)
{
    //GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟
    USART_DeInit(USART1);  //复位串口1
    //USART1_TX   PA.9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化PA9

    //USART1_RX	  PA.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);  //初始化PA10

    //USART 初始化设置
    USART_InitStructure.USART_BaudRate = bound;//一般设置为9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
    USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

    USART_Init(USART1, &USART_InitStructure); //初始化串口
#if ENABLE_USART1		  //如果使能了接收  
    //Usart1 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启中断
#endif
    USART_Cmd(USART1, ENABLE);                    //使能串口
    reset_uart_pipe1();
}

void USART1_IRQHandler(void)                	//串口1中断服务程序
{
    u8 Res;
#ifdef OS_TICKS_PER_SEC	 	//如果时钟节拍数定义了,说明要使用ucosII了.
    OSIntEnter();
#endif
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
    {
        Res =USART_ReceiveData(USART1);//(USART1->DR);	//读取接收到的数据
        uart_pipe1.pipe_buf[uart_pipe1.in_pos]=Res;
        uart_pipe1.in_pos++;
        uart_pipe1.in_pos%=PIPE_BUF_LEN;
        if(PIPE_BUF_LEN > uart_pipe1.sum)
            uart_pipe1.sum++;
        else uart_pipe1.out_pos=uart_pipe1.in_pos;
        uart_pipe1.time=Get_SysmilliTick();
    }
#ifdef OS_TICKS_PER_SEC	 	//如果时钟节拍数定义了,说明要使用ucosII了.
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

#if ENABLE_USART2   //如果使能了接收
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
    //GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//使能USART2，GPIOA时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    USART_DeInit(USART2);  //复位串口2

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA.2
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;//GPIO_Mode_Out_OD;	//复用推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化PA2

    //USART2_RX	  PA.3
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);  //初始化PA3

    //USART 初始化设置
    USART_InitStructure.USART_BaudRate = bound;//一般设置为9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
    USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

    USART_Init(USART2, &USART_InitStructure); //初始化串口
#if ENABLE_USART2		  //如果使能了接收  
    //Usart1 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器

    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启中断
#endif
    USART_Cmd(USART2, ENABLE);                    //使能串口
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

void USART2_IRQHandler(void)                	//串口2中断服务程序
{
    u8 Res;
#ifdef OS_TICKS_PER_SEC	 	//如果时钟节拍数定义了,说明要使用ucosII了.
    OSIntEnter();
#endif
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
    {
        Res =USART_ReceiveData(USART2);//(USART2->DR);	//读取接收到的数据
        Pipe_Block_Input(&uart_pipe2,(const char*)&Res,1);
		Pipe_Block_Input(&uart_pipe2_log,(const char*)&Res,1);
    }
#ifdef OS_TICKS_PER_SEC	 	//如果时钟节拍数定义了,说明要使用ucosII了.
    OSIntExit();
#endif
}
#endif

#if ENABLE_USART3   //如果使能了接收
Pipe_t uart_pipe3;
Pipe_t uart_send_pipe3;

void reset_uart_pipe3(void)
{
    memset((char*)&uart_pipe3,0,sizeof(Pipe_t));
	memset((char*)&uart_send_pipe3,0,sizeof(Pipe_t));
}
//初始化IO 串口1
//bound:波特率
void uart3_init(u32 bound)
{
    //GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//使能USART2，GPIOA时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    USART_DeInit(USART3);  //复位串口1

//USART3_TX	  PB.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PA.2
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽//GPIO_Mode_AF_OD;	//复用推挽输出GPIO_Mode_AF_OD;//
    GPIO_Init(GPIOB, &GPIO_InitStructure); //初始化PA2

    //USART3_RX	  PB.11
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(GPIOB, &GPIO_InitStructure);  //初始化PA3
    //USART 初始化设置

    USART_InitStructure.USART_BaudRate = bound;//一般设置为9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
    USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

    USART_Init(USART3, &USART_InitStructure); //初始化串口
#if ENABLE_USART3		  //如果使能了接收  
    //Usart1 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器

    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//开启中断
#endif
    USART_Cmd(USART3, ENABLE);                    //使能串口
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

void USART3_IRQHandler(void)                	//串口1中断服务程序
{
    u8 Res;
#ifdef OS_TICKS_PER_SEC	 	//如果时钟节拍数定义了,说明要使用ucosII了.
    OSIntEnter();
#endif
    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
    {
        Res =USART_ReceiveData(USART3);//(USART2->DR);	//读取接收到的数据

        uart_pipe3.pipe_buf[uart_pipe3.in_pos]=Res;
        uart_pipe3.in_pos++;
        uart_pipe3.in_pos%=PIPE_BUF_LEN;
        if(PIPE_BUF_LEN > uart_pipe3.sum)
            uart_pipe3.sum++;
        else uart_pipe3.out_pos=uart_pipe3.in_pos;
        uart_pipe3.time=Get_SysmilliTick();

    }
#ifdef OS_TICKS_PER_SEC	 	//如果时钟节拍数定义了,说明要使用ucosII了.
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
    //GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	//使能USART2，GPIOA时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);

    USART_DeInit(UART4);  //复位串口1

//UART4_TX	  PC.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PA.2
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽//GPIO_Mode_AF_OD;	//复用推挽输出GPIO_Mode_AF_OD;//
    GPIO_Init(GPIOC, &GPIO_InitStructure); //初始化PA2

    //UART4_RX	  PC.11
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    //GPIO_InitStructure.GPIO_Mode =GPIO_Mode_IPU;
    GPIO_Init(GPIOC, &GPIO_InitStructure);  //初始化PA3

    //USART 初始化设置
    USART_InitStructure.USART_BaudRate = bound;//一般设置为9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
    USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

    USART_Init(UART4, &USART_InitStructure); //初始化串口
 
    //Usart1 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器

    USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);//开启中断
    USART_Cmd(UART4, ENABLE);                    //使能串口
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
            //有时候会死在这里
            if(Get_SysmilliTick()-timespan>10)
            {
                break;
            }
        }
        timespan=Get_SysmilliTick();
        USART_SendData(UART4,(uint8_t) buf[__i]);
    }
}


void UART4_IRQHandler(void)                	//串口1中断服务程序
{
    u8 Res;
#ifdef OS_TICKS_PER_SEC	 	//如果时钟节拍数定义了,说明要使用ucosII了.
    OSIntEnter();
#endif
    if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
    {

        Res =USART_ReceiveData(UART4);//(USART1->DR);	//读取接收到的数据
        Pipe_Block_Input(&usart_pipe4,(const char*)&Res,1);
		Pipe_Block_Input(&uart_pipe4_log,(const char*)&Res,1);
    }
#ifdef OS_TICKS_PER_SEC	 	//如果时钟节拍数定义了,说明要使用ucosII了.
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
    //GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	//使能USART2，GPIOA时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);

    USART_DeInit(UART5);  //复位串口1

//UART5_TX	  PC.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽//GPIO_Mode_AF_OD;	//复用推挽输出GPIO_Mode_AF_OD;//
    GPIO_Init(GPIOC, &GPIO_InitStructure); //初始化PA2

    //UART5_RX	  PC.11
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(GPIOD, &GPIO_InitStructure);  //初始化PA3

    //USART 初始化设置
    USART_InitStructure.USART_BaudRate = bound;//一般设置为9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
    USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

    USART_Init(UART5, &USART_InitStructure); //初始化串口
 
    //Usart1 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器

    USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);//开启中断
    USART_Cmd(UART5, ENABLE);                    //使能串口
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
        {//有时候会死在这里            
            if(Get_SysmilliTick()-timespan>10)
            {
                break;
            }
        }
        timespan=Get_SysmilliTick();
        USART_SendData(UART5,(uint8_t) buf[__i]);
    }
}


void UART5_IRQHandler(void)                	//串口1中断服务程序
{
    u8 Res;
#ifdef OS_TICKS_PER_SEC	 	//如果时钟节拍数定义了,说明要使用ucosII了.
    OSIntEnter();
#endif
    if(USART_GetITStatus(UART5, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
    {
        Res =USART_ReceiveData(UART5);//(USART1->DR);	//读取接收到的数据
        Pipe_Block_Input(&uart_pipe5,(const char*)&Res,1);
		Pipe_Block_Input(&uart_pipe5_log,(const char*)&Res,1);
    }
#ifdef OS_TICKS_PER_SEC	 	//如果时钟节拍数定义了,说明要使用ucosII了.
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

