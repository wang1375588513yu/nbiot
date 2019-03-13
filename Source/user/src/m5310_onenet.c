#include "m5310_onenet.h"


static char cmd_buff[512];
static char tmp_buff[512];
static char reg_buff[512];

int nbiot_strncpy(char *dest,const char *src,int size)
{
    char *eos;
    eos = dest;
    while ( size && *src )
    {
        *eos = *src;
        ++eos;
        ++src;
        --size;
    }
    *eos = '\0';

    return (eos - dest);
}

int nbiot_itoa( int   val,char *str,int   size )
{
    if (str)
    {
        char ch;
        char tmp[11];
        char *dst = tmp;
        char *eos = tmp;

        if ( val < 0 )
        {
            dst++;
            val = -val;
            *eos++ = '-';
        }
        do
        {
            *eos++ = val % 10 + '0';
            val = val / 10;
        } while( val );

        /* reverse */
        *eos-- = '\0';
        while ( dst < eos )
        {
            ch = *dst;
            *dst = *eos;
            *eos = ch;

            ++dst;
            --eos;
        }
        return nbiot_strncpy(str, tmp, size);/* copy */
    }

    return 0;
}

void hex_to_hexstring(char *src,char *dst,int len)
{
    int i = 0,j = 0;
    for(i=0,j=0; j<len*2; i++,j++)
    {
        if(src[i]<=9)
        {
            dst[j] =  '0';
            dst[++j] = src[i] + '0';
        }
        else  if(src[i]>=0x0A&&src[i]<=0x0F)
        {
            dst[j] =  '0';
            dst[++j] = src[i] + 55;
        }
        else if (src[i]>=0x0F)
        {
            dst[j] =  src[i]/16 +'0';
            switch (dst[j])
            {
            case '0':
                dst[j] = '0';
                break;
            case '1':
                dst[j] = '1';
                break;
            case '2':
                dst[j] = '2';
                break;
            case '3':
                dst[j] = '3';
                break;
            case '4':
                dst[j] = '4';
                break;
            case '5':
                dst[j] = '5';
                break;
            case '6':
                dst[j] = '6';
                break;
            case '7':
                dst[j] = '7';
                break;
            case '8':
                dst[j] = '8';
                break;
            case '9':
                dst[j] = '9';
                break;
            case ':':
                dst[j] = 'A';
                break;
            case ';':
                dst[j] = 'B';
                break;
            case '<':
                dst[j] = 'C';
                break;
            case '=':
                dst[j] = 'D';
                break;
            case '>':
                dst[j] = 'E';
                break;
            case '?':
                dst[j] = 'F';
                break;
            default :
                break;
            }
            dst[++j] =  src[i]%16 +'0';
            switch (dst[j])
            {
            case '0':
                dst[j] = '0';
                break;
            case '1':
                dst[j] = '1';
                break;
            case '2':
                dst[j] = '2';
                break;
            case '3':
                dst[j] = '3';
                break;
            case '4':
                dst[j] = '4';
                break;
            case '5':
                dst[j] = '5';
                break;
            case '6':
                dst[j] = '6';
                break;
            case '7':
                dst[j] = '7';
                break;
            case '8':
                dst[j] = '8';
                break;
            case '9':
                dst[j] = '9';
                break;
            case ':':
                dst[j] = 'A';
                break;
            case ';':
                dst[j] = 'B';
                break;
            case '<':
                dst[j] = 'C';
                break;
            case '=':
                dst[j] = 'D';
                break;
            case '>':
                dst[j] = 'E';
                break;
            case '?':
                dst[j] = 'F';
                break;
            default :
                break;
            }
        }
    }
    return;
}
unsigned int command_length = 0;
char* mipl_generate(char *buf, size_t buflen, MIPL_T *mipl)
{
    //strcpy(buf,"AT+MIPLCONF=46,000000002C0100001900636F61703A2F2F3138332E3233302E34302E34303A353638330900313233343B6675636B,1,1\r\n");
//	SendCmd(buf,"OK",300);
    uint32_t offset = 0;
    char mode,temp[5];
    memset(buf, 0, buflen);
    mode = ((mipl->boot & 0x1) << 4) | (mipl->encrypt & 0x1);
    memcpy(buf + offset, &mode, 1);
    offset++;
    memcpy(buf + offset, &mipl->debug, 1);
    offset++;
    memcpy(buf + offset, &mipl->port, 2);
    offset += 2;
    memcpy(buf+offset, &mipl->keep_alive, 4);
    offset += 4;
    memcpy(buf + offset, &mipl->uri_len, 2);
    offset += 2;
    memcpy(buf + offset, mipl->uri, mipl->uri_len);
    offset += mipl->uri_len;
    memcpy(buf + offset, &mipl->ep_len, 2);
    offset += 2;
    memcpy(buf + offset, mipl->ep, mipl->ep_len);
    offset += mipl->ep_len;
    *(buf + offset) = mipl->block1;
    offset++;
    *(buf + offset) = mipl->block2;
    offset++;
    *(buf + offset) = mipl->block2th;
    offset++;

    strcpy(cmd_buff,"AT+MIPLCONF=");//创建一个通信实例设备，该指令目前只允许拥有1个实例
    nbiot_itoa(offset,temp,5);
    strcat(cmd_buff,temp);
    strcat(cmd_buff,",");
    hex_to_hexstring(buf,cmd_buff+strlen(cmd_buff),offset);
    strcat(cmd_buff,",1,1\r\n");
    strcpy(tmp_buff,cmd_buff);
//    memset(cmd_buff,0,1024*sizeof(char));//清空cmd_buff
	command_length = offset + 6;	//该命令长度
    return cmd_buff;
}

unsigned int get_command_len(void)
{
	return command_length;
}


char* init_miplconf(void)
{
    MIPL_T mipl;
    char buffer[512];
	//char *cmmd = "AT+MIPLCONF=71,100000002C0100001900636F61703A2F2F3138332E3233302E34302E33393A353638331F003836393937363033303136333435363B343630303430343237303038333130050501,1,1\r\n";
    mipl.boot=1;		//Bootstrap 模式设置， 1 为打开
    mipl.encrypt=0;		//加密模式设置   0为关闭
    mipl.debug=0;		//Debug等级设置  0是关闭debug模式
    mipl.port=0;		//本地端口号，设置缺省，模块自动搜索
    mipl.keep_alive=300;	//设备存活时间，标示终端和 OneNET 平台之间连接的存活周期， 设置范围为 10s~86400s
    mipl.uri="coap://183.230.40.39:5683";	//重庆平台 IP和端口
    mipl.uri_len=strlen(mipl.uri);			//IP和端口的长度
    mipl.ep="869976030163456;460040427008310";//鉴权参数，即模块的IMEI号和SIM卡的IMSI号
    mipl.ep_len=strlen(mipl.ep);			//长度	
    mipl.block1 = 5,	//设置 PUT 和 POST 指令分片长度	默认设置为5就好
    mipl.block2 = 5,	//设置 GET 指令分片长度	默认设置为5就好
    mipl.block2th = 1,	//设置出发分片操作的最大长度
    strcpy(reg_buff,mipl_generate(buffer,sizeof(buffer),&mipl));
	
    return reg_buff;
}


