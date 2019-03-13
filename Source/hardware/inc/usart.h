#ifndef __USART_H
#define __USART_H

#include "sys.h" 
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include "PUBLIC.H"


#define ENABLE_USART1 			1		
#define ENABLE_USART2 			1		
#define ENABLE_USART3 			1		
#define ENABLE_USART4 			0
#define ENABLE_USART5 			0

extern Pipe_t  uart_pipe1;
extern Pipe1_t uart_pipe2;
extern Pipe1_t uart_pipe2_log;
extern Pipe_t  uart_pipe3;
extern Pipe_t  uart_send_pipe3;
extern Pipe1_t usart_pipe4;
extern Pipe1_t uart_pipe4_log;
extern Pipe1_t uart_pipe5;


void uart1_printf(char * string, ...);
void uart2_printf(char * string, ...);
void uart3_printf(char * string, ...);
void uart4_printf(char * string, ...);
void uart5_printf(char * string, ...);

void uart1_output(const unsigned char * buf,int len);
void uart2_output(const unsigned char * buf,int len);
void uart3_output(const unsigned char * buf,int len);
void uart4_output(const unsigned char * buf,int len);
void uart5_output(const unsigned char * buf,int len);

void reset_uart_pipe1(void);
void reset_uart_pipe2(void);
void reset_uart_pipe3(void);
void reset_uart_pipe4(void);
void reset_uart_pipe5(void);

void *get_txBuffer(void);
void uart_init(USART_TypeDef* USARTx,u32 bound);


#endif


