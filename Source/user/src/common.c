#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include "common.h"

Union8Byte_t myunion= {0};

void *my_memmem(const void *start, unsigned int s_len,const void *find, unsigned int f_len)
{
    char    *p, *q;
    unsigned int  len;
    if(start==NULL || find==NULL || s_len==0 || f_len==0 || s_len<f_len)return NULL;
    p = (char *)start, q = (char *)find;
    len = 0;
    while((p - (char *)start + f_len) <= s_len)
    {
        while(*p++ == *q++)
        {
            len++;
            if(len == f_len)
                return(p - f_len);
        };
        p=p-len;
        q = (char *)find;
        len = 0;
    };

    return(NULL);
}

char *my_memstr(const char *str1,unsigned int len1,const char *str2)
{
    char *p1;
    char find=0;
    int i,len2,j,k;

    if(str1==NULL || str2==NULL)   return NULL;

    p1=(char *)str2;
    len2=0;
    while(*p1++!=0) len2++;
    if(len2>len1)  return NULL;

    for(i=len2-1; i<len1; i++)
    {
        k=0;
        p1=(char *)&str1[i-(len2-1)];
        for(j=i-(len2-1); j<=i; j++)
        {
            if(str1[j]!=str2[k]) break;
            k++;
        }
        if(k==len2)
        {
            find=1;
            break;
        }
    }

    if(find==1)  return p1;
    else return NULL;
}


char num_to_char(unsigned char num)
{
    if(num>0xf)  return ' ';
    else if(num<=9) return '0'+num;
    else return ('A'+(num-10));
}

unsigned char char_to_num(char ch)
{
    unsigned char v_c=0x10;

    if((ch>='A') && (ch<='F'))
        v_c=10+ch-'A';
    else   if((ch>='a') && (ch<='f'))
        v_c=10+ch-'a';
    else   if((ch>='0') && (ch<='9'))
        v_c=ch-'0';

    return v_c;
}

int hexbytestr_to_num(char *strbyte,int len,unsigned char* num)
{
    unsigned char v_c1,v_c2;
    if(strbyte==NULL || num==NULL||len!=2)  return -1;

    v_c1=char_to_num(strbyte[0]);
    if(v_c1&0x10)   return -1;

    v_c2=char_to_num(strbyte[1]);
    if(v_c2&0x10)   return -1;

    *num=(v_c1<<4)|v_c2;
    return 0;
}

unsigned short htons(unsigned short n)
{
    return ((n&0xff)<<8)|(n>>8);
}
unsigned short  ntohs(unsigned short m)
{
    return ((m&0xff)<<8)|(m>>8);
}


unsigned int htonl(unsigned int n)
{
    return ((n&0xff)<<24)|((n&0xff00)<<8)|((n>>8)&0xff00)|((n>>24)&0xff);
}
unsigned int  ntohl(unsigned int m)
{
    return ((m&0xff)<<24)|((m&0xff00)<<8)|((m>>8)&0xff00)|((m>>24)&0xff);
}

void Reverse_Stream_Order_8byte(void)
{
    myunion.byte[0] ^= myunion.byte[7];
    myunion.byte[7] ^= myunion.byte[0];
    myunion.byte[0] ^= myunion.byte[7];

    myunion.byte[1] ^= myunion.byte[6];
    myunion.byte[6] ^= myunion.byte[1];
    myunion.byte[1] ^= myunion.byte[6];

    myunion.byte[2] ^= myunion.byte[5];
    myunion.byte[5] ^= myunion.byte[2];
    myunion.byte[2] ^= myunion.byte[5];

    myunion.byte[3] ^= myunion.byte[4];
    myunion.byte[4] ^= myunion.byte[3];
    myunion.byte[3] ^= myunion.byte[4];
}

unsigned long long Reverse_Stream_Order_ULL(unsigned long long data)
{
    myunion.vull=data;
    Reverse_Stream_Order_8byte();
    return myunion.vull;
}

long long Reverse_Stream_Order_LL(long long data)
{
    myunion.vll=data;
    Reverse_Stream_Order_8byte();
    return myunion.vll;
}

double Reverse_Stream_Order_DOU(double data)
{
    myunion.vd=data;
    Reverse_Stream_Order_8byte();
    return myunion.vd;
}

void Reverse_Stream_Order_4byte(void)
{
    myunion.byte[0] ^= myunion.byte[3];
    myunion.byte[3] ^= myunion.byte[0];
    myunion.byte[0] ^= myunion.byte[3];

    myunion.byte[1] ^= myunion.byte[2];
    myunion.byte[2] ^= myunion.byte[1];
    myunion.byte[1] ^= myunion.byte[2];
}

long Reverse_Stream_Order_Long(long data)
{
    myunion.vl=data;
    Reverse_Stream_Order_4byte();
    return myunion.vl;
}
unsigned long Reverse_Stream_Order_ULong(unsigned long data)
{
    myunion.vul=data;
    Reverse_Stream_Order_4byte();
    return myunion.vul;
}
int Reverse_Stream_Order_Int(int data)
{
    return Reverse_Stream_Order_Long(data);
}

unsigned int Reverse_Stream_Order_UInt(unsigned int data)
{
    return Reverse_Stream_Order_ULong(data);
}

float Reverse_Stream_Order_Float(float data)
{
    myunion.vf=data;
    Reverse_Stream_Order_4byte();
    return myunion.vf;
}

void Reverse_Stream_Order_2byte(void)
{
    myunion.byte[0] ^= myunion.byte[1];
    myunion.byte[1] ^= myunion.byte[0];
    myunion.byte[0] ^= myunion.byte[1];
}

short Reverse_Stream_Order_Short(short data)
{
    myunion.vs=data;
    Reverse_Stream_Order_2byte();
    return myunion.vs;
}
unsigned short Reverse_Stream_Order_Ushort(unsigned short data)
{
    myunion.vus=data;
    Reverse_Stream_Order_2byte();
    return myunion.vus;
}

#define POSITIVE_SIGNAL   0
#define NEGATIVE_SIGNAL  1

int decode_number(char *buf,unsigned char len,double *num)
{
    double value=0.0,fac=1.0;
    char *p=buf,dir=0,dot_sum=0;
    char sign;

    if(buf==NULL ||num==NULL || len==0)  return -1;

    if(*p=='-')
    {
        sign=NEGATIVE_SIGNAL;
        p++;
        len--;
    }
    else if(*p=='+')
    {
        sign=POSITIVE_SIGNAL;
        p++;
        len--;
    }
    else  if(*p>='0' && *p <='9')
    {
        sign=POSITIVE_SIGNAL;
    }
    else  return -1;

    while(len--)
    {
        if(*p!='.')
        {
            if(dir==0)    //整数部分
            {
                if(*p>='0' && *p <='9')
                {
                    value*=10;
                    value+=(*p-'0');
                }
                else
                    return -1;
            }
            else //   if(dir==0)  小数部分
            {
                if(*p>='0' && *p <='9')
                {
                    fac*=0.1;
                    value+=(*p-'0')*fac;
                }
                else
                    return -1;
            }
        }
        else  //if(*p!='.')
        {
            if(dot_sum==0)  //整数部分结束 小数部分开始
            {
                dir++;
                dot_sum++;
            }
            else
                return -1;
        }
        p++;
    }

    if(sign==NEGATIVE_SIGNAL)
        *num=value*(-1);
    else
        *num=value;
    return 0;
}

int hexuintstr_to_num(char *strbyte,int len,unsigned int* num)
{
    unsigned char v_c;
    unsigned int uintnum=0;
    if(strbyte==NULL || num==NULL||len==0)  return -1;

    while(len--)
    {
        v_c=char_to_num(*strbyte++);
        if(v_c&0x10)   return -1;
        else
        {
            uintnum<<=4;
            uintnum|=v_c;
        }
    }

    *num=uintnum;
    return 0;
}
