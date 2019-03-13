#ifndef __COMMON__H__
#define __COMMON__H__

typedef union Union8Byte
{
	char byte[8];
	
	short vs;
	unsigned short vus;
	
	int vi;
	unsigned int vui;
	long vl;
	unsigned long vul;
	float vf;
	
	long long vll;
	unsigned long long vull;
	double vd;

}Union8Byte_t;

void *my_memmem(const void *start, unsigned int s_len,const void *find, unsigned int f_len);
char *my_memstr(const char *str1,unsigned int len1,const char *str2);
char num_to_char(unsigned char num);
unsigned char char_to_num(char ch);
int hexbytestr_to_num(char *strbyte,int len,unsigned char* num);

unsigned short htons(unsigned short n);
unsigned short  ntohs(unsigned short m); 
unsigned int htonl(unsigned int n);
unsigned int  ntohl(unsigned int m); 

void Reverse_Stream_Order_8byte(void);
unsigned long long Reverse_Stream_Order_ULL(unsigned long long data);
long long Reverse_Stream_Order_LL(long long data);
double Reverse_Stream_Order_DOU(double data);

void Reverse_Stream_Order_4byte(void);
long Reverse_Stream_Order_Long(long data);
unsigned long Reverse_Stream_Order_ULong(unsigned long data);
int Reverse_Stream_Order_Int(int data);
unsigned int Reverse_Stream_Order_UInt(unsigned int data);
float Reverse_Stream_Order_Float(float data);

void Reverse_Stream_Order_2byte(void);
short Reverse_Stream_Order_Short(short data);
unsigned short Reverse_Stream_Order_Ushort(unsigned short data);

int decode_number(char *buf,unsigned char len,double *num);
int hexuintstr_to_num(char *strbyte,int len,unsigned int* num);



#endif
