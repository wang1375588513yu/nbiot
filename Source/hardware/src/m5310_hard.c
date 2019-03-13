#include "m5310_hard.h"
#include "usart.h"
#include "timer.h"

#define  hardware_init

#define nbiot_SEND_PIPE_MAXSIZE 300
unsigned char nbiot_SEND_PIPE_BUF[nbiot_SEND_PIPE_MAXSIZE];
Pipe1_t nbiot_send_pipe= {nbiot_SEND_PIPE_BUF,nbiot_SEND_PIPE_MAXSIZE,0,0,0,0};

#ifdef hardware_init
bool NBIOT_send_pipe_input(const char*packet,unsigned short packetlen)
{
    if(packet==NULL||packetlen==0)return false;
    if(nbiot_send_pipe.sum+packetlen>nbiot_send_pipe.size)return false;
    if(nbiot_send_pipe.pin+packetlen<=nbiot_send_pipe.size)
    {
        memcpy((void*)&nbiot_send_pipe.buf[nbiot_send_pipe.pin],(const void*)packet,packetlen);
    }
    else
    {
        memcpy((void*)&nbiot_send_pipe.buf[nbiot_send_pipe.pin],(const void*)packet,(nbiot_send_pipe.size-nbiot_send_pipe.pin));
        memcpy((void*)&nbiot_send_pipe.buf[0],(const void*)(packet+(nbiot_send_pipe.size-nbiot_send_pipe.pin)),packetlen-(nbiot_send_pipe.size-nbiot_send_pipe.pin));
    }
    nbiot_send_pipe.pin+=packetlen;
    nbiot_send_pipe.pin%=nbiot_send_pipe.size;
    nbiot_send_pipe.sum+=packetlen;
    return true;
}

void NBIOT_Send_Packet(const void* data,int datalen)
{
    if(data==NULL||datalen==0)return;
    NBIOT_send_pipe_input((const char*)data,datalen);
}

void NBIOT_normal_output(void)
{
    if(nbiot_send_pipe.sum>0)
    {
        if(USART_GetFlagStatus(UART4,USART_FLAG_TC)==SET ||(Get_SysmilliTick() - nbiot_send_pipe.time >10))
        {   //如果可以发送就发送
            if(Get_MillisecTimer_NBIOT_delay())return;

            nbiot_send_pipe.time=Get_SysmilliTick();
            USART_SendData(UART4,(uint8_t)nbiot_send_pipe.buf[nbiot_send_pipe.pout++]);
            nbiot_send_pipe.pout %= nbiot_send_pipe.size;
            nbiot_send_pipe.sum--;
            if(nbiot_send_pipe.pout%1000==0)
            {
                Set_MillisecTimer_NBIOT_delay(100);
            }
        }
    }
}

#endif
