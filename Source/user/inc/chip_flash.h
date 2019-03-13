//==========================================================================================
//      Copyright (c) 2015 nothdoo.  All Rights Reserved                
//==========================================================================================
//  Model Name   : bdyzj0001                                                                                                       
//==========================================================================================  
//  MCU Type     :            stm32f103x   
//  Compile Tool :           keil 5.0                
//  System clock :            HF 72M, HXTAL Type,    
//  Power Supply :            
//          
//  Start Date   :            2015.07.03
//  End Date     :                 
//  Author       :            Brank Zhu 
//==========================================================================================

#ifndef __USER_FLASH_H__
#define __USER_FLASH_H__
#include "config.h"



unsigned char chip_read_app_header(flash_header_t* pheader);
unsigned char chip_write_app_header(flash_header_t* pheader);
unsigned char chip_read_flash(unsigned int ReadAddr,unsigned short *pBuffer,unsigned short NumToRead);
unsigned char chip_write_flash(unsigned int writeAddr,unsigned short *pBuffer,unsigned short NumToWrite);
unsigned char chip_erase_flash(unsigned int startAddr,unsigned short pagesum);
unsigned char chip_crc_flash( unsigned int startAddr,unsigned int len,unsigned int *crc);
#endif
