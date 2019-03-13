#ifndef __M5310_ONENET_H__
#define __M5310_ONENET_H__

#include "sys.h"
#include "PUBLIC.h"

typedef struct
{
	uint8_t boot;
	uint8_t encrypt;
	uint8_t debug;
	uint16_t port;
	uint32_t keep_alive;
	size_t uri_len;
	const char* uri;
	size_t ep_len;
//	const char IMEI[15];
	const char* ep;
//	const char IMSI[15];
	uint8_t block1;	//COAP option BLOCK1(PUT or POST),0-6. 2^(4+n)  bytes
	uint8_t block2;	//COAP option BLOCK2(GET),0-6. 2^(4+n)  bytes
	uint8_t block2th;	//max size to trigger block-wise operation,0-2. 2^(8+n) bytes
}MIPL_T;

int nbiot_strncpy(char *dest,const char *src,int size);
int nbiot_itoa( int   val,char *str,int   size );
void hex_to_hexstring(char *src,char *dst,int len);
char* mipl_generate(char *buf, size_t buflen, MIPL_T *mipl);
char* init_miplconf(void);
unsigned int get_command_len(void);

#endif
