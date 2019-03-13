#ifndef __M5310_HARD_H__
#define __M5310_HARD_H__

#include "sys.h"
#include "PUBLIC.H"

void NBIOT_Send_Packet(const void* data,int datalen);
bool NBIOT_send_pipe_input(const char*packet,unsigned short packetlen);
void NBIOT_normal_output(void);

#endif
