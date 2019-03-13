#include "PUBLIC.H"
#include <string.h>
#include "timer.h"

/*****************************************PUBLIC INTERFACE************************************************/
bool allneedtosleep=false;

unsigned char needtestcount=0;

bool remote_keep_control_enable=false;
bool bluetoothautoswitch=false;
unsigned int dtuA_inbytecount=0;
unsigned int dtuB_inbytecount=0;



char PublicSendBuffer[PUBLIC_SEND_BUFSIZE]= {0};


char* GetBuffer(void)
{
    return PublicSendBuffer;
}

void SoftReset(void)
{
// __set_FAULTMASK(1);      // 关闭所有中断
    NVIC_SystemReset();      // 复位
}

//当从EEPROM读出来的角色都不是这些类型的时候,数传应该不工作
bool is_dtu_role(unsigned char role)
{
    if(role==DTU_ROLE_COORDINATOR)return true;
    if(role==DTU_ROLE_TERMINAL)return true;
    if(role==DTU_ROLE_REPEATOR)return true;
    if(role==DTU_ROLE_TERMINAL_AND_REPEATER)return true;

    return false;
}





unsigned char CheckSum(const void *buf,int len)
{

    unsigned char cka=0;
    unsigned char *p=NULL;
    if(buf==NULL || len<=0)return NULL;
    //if(!SafeRegion_For_STM32F407ZG(buf,len))return NULL;


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
    //if(!SafeRegion_For_STM32F407ZG(buf,len))return NULL;


    p=(unsigned char *)buf;

    while(len--)
    {
        cka=cka^*p++;
    }
    return cka;
}

void Pipe_Reset(Pipe_t *pipe)
{
    if(pipe==NULL)return;
    pipe->pin=0;
    pipe->pout=0;
    pipe->sum=0;
}
void Pipe_Remove(Pipe_t *pipe,unsigned int len)
{
    if(pipe==NULL || pipe->sum<len || len==0 || len>pipe->size)return;
    pipe->sum -= len;
    memmove((void*)pipe->buf,(const void*)(pipe->buf+len),pipe->sum);
    pipe->pin=pipe->sum;
}

bool Pipe_Block_Input(Pipe_t *pipe,const char*packet,unsigned short packetlen)
{
    if(pipe==NULL||pipe->buf==NULL||pipe->size==NULL||packet==NULL||packetlen==0)return false;
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

bool Pipe_Check(Pipe_t *pipe)
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
    unsigned char len=0;
    DEC10=0;
    len=0;

    while(BCDHEX)
    {
        DEC10+=(BCDHEX%16)*NMEA_Pow(10,len);
        BCDHEX /=16;
        len++;
    }

    return DEC10;
}

unsigned long long DEC2HEX(unsigned long long DEC10)
{
    unsigned long long BCDHEX=0;
    unsigned char len=0;
    BCDHEX=0;
    len=0;

    while(DEC10)
    {
        BCDHEX+=(DEC10%10)*NMEA_Pow(16,len);
        DEC10 /=10;
        len++;
    }

    return BCDHEX;
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
unsigned short Caculate_CRC16(unsigned char *DAT, int Lenth)
{

    unsigned short CRC16 = 0xFFFF;
    unsigned char i;
    unsigned char j;

    if(DAT==NULL || Lenth<=0)return NULL;
    if((unsigned int)DAT<SRAM_BASE)return NULL;
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

#define CRC8INIT    0x00
#define CRC8POLY    0x18              //0X18 = X^8+X^5+X^4+X^0   

unsigned char CRC8 ( uint8_t *data_in, uint16_t number_of_bytes_to_read )
{
    uint8_t   crc;
    uint16_t loop_count;
    uint8_t  bit_counter;
    uint8_t  data;
    uint8_t  feedback_bit;

    crc = CRC8INIT;

    for (loop_count = 0; loop_count != number_of_bytes_to_read; loop_count++)
    {   data = data_in[loop_count];

        bit_counter = 8;
        do
        {   feedback_bit = (crc ^ data) & 0x01;
            if ( feedback_bit == 0x01 )
                crc = crc ^ CRC8POLY;

            crc = (crc >> 1) & 0x7F;
            if ( feedback_bit == 0x01 )
                crc = crc | 0x80;

            data = data >> 1;
            bit_counter--;
        }
        while (bit_counter > 0);
    }
    return crc;
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

bool Wifi_Config_Info_Check(Wifi_Config_Info_t *wifi)
{
    //unsigned char *checkcode=NULL;
    int len=0;

    if(wifi==NULL)return false;

    if(wifi->wifi_onOff!=0&&wifi->wifi_onOff!=1)return false;

    if(wifi->wifi_authtype!=0&&wifi->wifi_authtype!=1&&wifi->wifi_authtype!=2&&wifi->wifi_authtype!=3&&wifi->wifi_authtype!=4&&wifi->wifi_authtype!=5)return false;

    if(wifi->wifi_encryptmod!=0&&wifi->wifi_encryptmod!=1&&wifi->wifi_encryptmod!=2&&wifi->wifi_encryptmod!=3&&wifi->wifi_encryptmod!=4)return false;

    if(wifi->wifi_authtype==0||wifi->wifi_authtype==1)
    {
        if(wifi->wifi_encryptmod!=0&&wifi->wifi_encryptmod!=1)return false;
    }
    else if(wifi->wifi_authtype==2)
    {
        if(wifi->wifi_encryptmod!=1)return false;
    }
    else if(wifi->wifi_authtype>=3)
    {
        if(wifi->wifi_encryptmod<2)return false;
    }

    if(wifi->wifi_mod!=1&&wifi->wifi_mod!=2&&wifi->wifi_mod!=3&&wifi->wifi_mod!=4)return false;

    if(wifi->wifi_channel>13)
    {
        if(wifi->wifi_channel!=149&&wifi->wifi_channel!=153&&wifi->wifi_channel!=157&&wifi->wifi_channel!=161&&wifi->wifi_channel!=165)return false;
    }

    if(wifi->wifi_mod==1)
    {
        if(wifi->wifi_channel>=1&&wifi->wifi_channel<=13)return false;
    }
    else
    {
        if(wifi->wifi_channel>13)return false;
    }

    if(wifi->wifi_broadcast_onoff!=0&&wifi->wifi_broadcast_onoff!=1)return false;

    len=strlen((const char*)wifi->wifi_ssid);

    if(!(len>=1&&len<=32))return false;

    len=strlen((const char*)wifi->wifi_passwd);

    if(!(len>=8&&len<=64))return false;

    return true;
}

bool isDevice_data_check_list_t(const Device_data_check_list_t *plist)
{
    if(plist==NULL)return false;
    if(!isBCDencodeing(plist->imei))return false;
    if(plist->onenet_data_base>2)return false;
    return true;
}



