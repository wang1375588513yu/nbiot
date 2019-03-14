#include "PUBLIC.H"
#include <string.h>
#include "timer.h"

bool bluetoothautoswitch=false;
bool bluetoothhandshake = false;

unsigned char _txBuffer[UART_TX_LEN]= {0};


/*****************************************PUBLIC INTERFACE************************************************/

void SoftReset(void)
{
    NVIC_SystemReset();      // ??
}

char* _ptxBuffer(void)
{
	return (char*)_txBuffer;
}


//设计接口的时候一定要注意这里使用的MCU是STM32F103CB,其SRAM只有20K,地址区间为0x20000000~0x2000FFFF
unsigned char CheckSum(const void *buf,int len)
{

    unsigned char cka=0;
    unsigned char *p=NULL;
    if(buf==NULL || len<=0)return NULL;
    if((unsigned int)buf<SRAM_BASE)return NULL;
    if((unsigned int)buf+len>=SRAM_END_ADDR)return NULL;


    p=(unsigned char *)buf;

    while(len--)
    {
        cka+=*p++;
    }
    return cka;
}

unsigned char CheckXOR(const void *buf,int len)
{

    unsigned char cka=0;
    unsigned char *p=NULL;
    if(buf==NULL || len<=0)return NULL;
    if((unsigned int)buf<SRAM_BASE)return NULL;
    if((unsigned int)buf+len>=SRAM_END_ADDR)return NULL;


    p=(unsigned char *)buf;

    while(len--)
    {
        cka=cka^*p++;
    }
    return cka;
}

void Pipe_Reset(Pipe_t *pipe)
{
    pipe->in_pos=0;
    pipe->out_pos=0;
    pipe->sum=0;
}

#ifdef pipe1

bool remote_keep_control_enable=false;
void Pipe1_Reset(Pipe1_t *pipe)
{
    if(pipe==NULL)return;
	memset((void *)pipe->buf, 0, pipe->size);
    pipe->pin=0;
    pipe->pout=0;
    pipe->sum=0;
}

void Pipe1_Remove(Pipe1_t *pipe,unsigned int len)
{
    if(pipe==NULL || pipe->sum<len || len==0 || len>pipe->size)return;
    pipe->sum -= len;
    memmove((void*)pipe->buf,(const void*)(pipe->buf+len),pipe->sum);
    pipe->pin=pipe->sum;
}

bool Pipe_Block_Input(Pipe1_t *pipe,const char*packet,unsigned short packetlen)
{
    if(pipe==NULL||pipe->buf==NULL||pipe->size==0||packet==NULL||packetlen==0)return false;
    if(pipe->sum+packetlen>pipe->size)return false;
    if(pipe->pin+packetlen<=pipe->size)
    {
        memcpy((void*)&pipe->buf[pipe->pin],(const void*)packet,packetlen);
    }
    else
    {
        memcpy((void*)&pipe->buf[pipe->pin],(const void*)packet,(pipe->size-pipe->pin));
        memcpy((void*)&pipe->buf[0],(const void*)(packet+(pipe->size-pipe->pin)),packetlen-(pipe->size-pipe->pin));
    }
    pipe->pin+=packetlen;
    pipe->pin%=pipe->size;
    pipe->sum+=packetlen;
    pipe->time=Get_SysmilliTick();

    return true;
}

bool Pipe_Check(Pipe1_t *pipe)
{
    if(pipe==NULL)return false;
    if(pipe->buf==NULL)return false;
    if(pipe->size==NULL)return false;
    if(pipe->pin==pipe->pout)
    {
        if(pipe->sum==0||pipe->sum==pipe->size)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        if(pipe->pin>pipe->pout)
        {
            if(pipe->pin-pipe->pout==pipe->sum)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            if(pipe->pout-pipe->pin+pipe->sum==pipe->size)
            {
                return true;
            }
            else
            {
                return false;
            }
        }

    }

    //return true;
}


#endif


void Pipe_Remove(Pipe_t *pipe,unsigned int len)
{
    if(pipe==NULL || pipe->sum<len || len==0 || len>PIPE_BUF_LEN)return;
    pipe->sum -= len;
    memmove((void*)pipe->pipe_buf,(const void*)(pipe->pipe_buf+len),pipe->sum);
    pipe->in_pos=pipe->sum;
}

long long NMEA_Pow(unsigned char m,unsigned char n)
{
    long long result=1;
    while(n--)result*=m;
    return result;
}

long long NMEA_Str2num(const unsigned char *buf,unsigned char jinzhi,unsigned char *dx)
{

    unsigned char *p=(unsigned char *)buf;
    long long ires=0,fres=0;
    unsigned char ilen=0,flen=0,i=0;
    unsigned char mask=0;
    long long res=0;
    if(buf==NULL||dx==NULL)return 0;

    if(*p=='-') {
        mask|=0X02;
        p++;
    }
    while(1)
    {
        if(mask&0X01 && *p=='.') break;
        if(*p=='.') {
            mask|=0X01;
            p++;
        }

        if( (jinzhi<=10) && ((*p<'0') ||((jinzhi-1+'0')<*p)) )break;
        if( (10<jinzhi && jinzhi<=16) && ((*p<'0') ||('9'<*p && *p<'A')||((jinzhi-11+'A')<*p)))break;
        if(mask&0X01)flen++;
        else ilen++;
        p++;
    }
    if(mask&0X02)buf++;
    for(i=0; i<ilen; i++)
    {

        if(jinzhi<=10)ires+=(long long)((buf[i]-'0')*NMEA_Pow(jinzhi,ilen-1-i));
        if(jinzhi>10)
        {
            if('0'<=buf[i] && buf[i]<='9')ires+=(long long)((buf[i]-'0')*NMEA_Pow(jinzhi,ilen-1-i));
            else if('A'<=buf[i] && buf[i]<='F')ires+=(long long)((buf[i]-'A'+10)*NMEA_Pow(jinzhi,ilen-1-i));
        }
    }
    if(flen>5)flen=5;
    *dx=flen;
    for(i=0; i<flen; i++)
    {
        if(jinzhi<=10)fres+=(long long)((buf[ilen+1+i]-'0')*NMEA_Pow(jinzhi,flen-1-i));
        if(jinzhi>10)
        {
            if('0'<=buf[ilen+1+i] && buf[ilen+1+i]<='9')fres+=(long long)((buf[ilen+1+i]-'0')*NMEA_Pow(jinzhi,flen-1-i));
            if('A'<=buf[ilen+1+i] && buf[ilen+1+i]<='F')fres+=(long long)((buf[ilen+1+i]-'A'+10)*NMEA_Pow(jinzhi,flen-1-i));
        }
    }
    res=ires*NMEA_Pow(jinzhi,flen)+fres;
    if(mask&0X02)res=-res;
    return res;
}

unsigned long long HEX2DEC(unsigned long long BCDHEX)
{
    unsigned long long DEC10=0;
    unsigned long long temp=0;
    unsigned char BCDHEXlen=0;
    unsigned char BCDbuff=0;
    temp=BCDHEX;
    while(temp)
    {
        BCDbuff=temp%16;

        DEC10+=BCDbuff*NMEA_Pow(10,BCDHEXlen);
        temp /=16;
        BCDHEXlen++;
    }

    return DEC10;
}

unsigned long long DEC2HEX(unsigned long long DEC10)
{
    unsigned long long BCDHEX=0;
    unsigned long long temp=0;
    unsigned char DEC10len=0;
    unsigned char Datebuff=0;
    temp=DEC10;
    while(temp)
    {
        Datebuff=temp%10;

        BCDHEX+=Datebuff*NMEA_Pow(16,DEC10len);
        temp /=10;
        DEC10len++;
    }

    return BCDHEX;
}

unsigned short Caculate_CRC16(unsigned char *DAT, int Lenth)
{

    unsigned short CRC16 = 0xFFFF;
    unsigned char i;
    unsigned char j;

    if(DAT==NULL || Lenth<=0)return NULL;
    if((unsigned int)DAT+Lenth>=SRAM_END_ADDR)return NULL;

    for(i=0; i<Lenth; i++)
    {
        CRC16 = CRC16 ^ DAT[i];
        for(j=0; j<8; j++)
        {

            if(CRC16 & 0x01)
            {
                CRC16 = CRC16 >> 1;
                CRC16 = CRC16 ^ 0xA001;
            }
            else
            {
                CRC16 = CRC16 >> 1;
            }
        }
    }
    return(CRC16);
}

bool isBCDencodeing(unsigned long long BCDHEX)
{
    unsigned char BCDbuff=0;
    if(BCDHEX==0)return true;
    while(BCDHEX)
    {
        BCDbuff=BCDHEX%16;
        if(BCDbuff>=0xA)return false;
        BCDHEX/=16;
    }
    return true;
}

#define CRC32_POLYNOMIAL 0xEDB88320L
/* --------------------------------------------------------------------------
Calculate a CRC value to be used by CRC calculation functions.
-------------------------------------------------------------------------- */
unsigned long CRC32Value(int i)
{
    int j;
    unsigned long ulCRC;
    ulCRC = i;
    for ( j = 8 ; j > 0; j-- )
    {
        if ( ulCRC & 1 )
            ulCRC = ( ulCRC >> 1 ) ^ CRC32_POLYNOMIAL;
        else
            ulCRC >>= 1;
    }
    return ulCRC;
}
/* --------------------------------------------------------------------------
Calculates the CRC-32 of a block of data all at once
-------------------------------------------------------------------------- */
unsigned long CalculateBlockCRC32(
    unsigned long ulCount, /* Number of bytes in the data block */
    unsigned char *ucBuffer ) /* Data block */
{
    unsigned long ulTemp1;
    unsigned long ulTemp2;
    unsigned long ulCRC = 0;
    while ( ulCount-- != 0 )
    {
        ulTemp1 = ( ulCRC >> 8 ) & 0x00FFFFFFL;
        ulTemp2 = CRC32Value( ((int) ulCRC ^ *ucBuffer++ ) & 0xff );
        ulCRC = ulTemp1 ^ ulTemp2;
    }
    return( ulCRC );
}

bool IPaddressPhare(const char *string,unsigned char (*ip)[4],unsigned int len)
{
    unsigned int i=0,data=0;
    unsigned char dotcnt=0,iptemp[4]= {0};

    if(string==NULL||ip==NULL||len<=7)return false;//0.0.0.0

    for(i=0; i<len; i++)
    {
        if(*string=='.')
        {
            iptemp[dotcnt]=data;
            dotcnt++;
            string++;
            data=0;
            if(dotcnt>3)return false;
        }
        else
        {
            if('0'<=*string && *string<='9')
            {
                data*=10;
                data+=*string-'0';
                if(data>256)return false;
                string++;
            }
            else return false;
        }
    }
    if(dotcnt==3)
    {
        iptemp[dotcnt]=data;
        memcpy((void*)ip,(const void*)iptemp,4);
        return true;
    }
    else return false;
}




char PublicSendBuffer[PUBLIC_SEND_BUFSIZE]= {0};

char* GetBuffer(void)
{
    return PublicSendBuffer;
}


