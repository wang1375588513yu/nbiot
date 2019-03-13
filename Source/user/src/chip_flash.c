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
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "stmflash.h"
#include "stm32f10x_flash.h"

#include "delay.h"
#include "config.h"
#include "oscrc32.h"






unsigned char chip_read_app_header(flash_header_t* pheader)
{

    if(pheader==NULL)  return 1;

    return STMFLASH_Read(APP_HEADER_START_ADDR,(u16 *)pheader,sizeof(flash_header_t)/2);
}

unsigned char chip_write_app_header(flash_header_t* pheader)
{

    if(pheader==NULL)  return 1;


    return STMFLASH_Write(APP_HEADER_START_ADDR,(u16 *)pheader,sizeof(flash_header_t)/2);
}


unsigned char chip_read_flash(unsigned int ReadAddr,unsigned short *pBuffer,unsigned short NumToRead)
{

    if(ReadAddr<STM32_FLASH_BASE ||((ReadAddr+2*NumToRead)>=(STM32_FLASH_BASE+1024*STM32_FLASH_SIZE))||pBuffer==NULL)  return 1;


    return STMFLASH_Read(ReadAddr,pBuffer,NumToRead);
}

unsigned char chip_write_flash(unsigned int writeAddr,unsigned short *pBuffer,unsigned short NumToWrite)
{

    if(writeAddr<STM32_FLASH_BASE ||((writeAddr+2*NumToWrite)>(STM32_FLASH_BASE+1024*STM32_FLASH_SIZE))||pBuffer==NULL)  return 1;


    return STMFLASH_Write(writeAddr,pBuffer,NumToWrite);
}


unsigned char chip_erase_flash(unsigned int startAddr,unsigned short pagesum)
{
    unsigned short i;

    if((startAddr-STM32_FLASH_BASE)%FLASH_PAGE_SIZE==0)  return 1;

    if((startAddr<STM32_FLASH_BASE)||(startAddr>=STM32_FLASH_BASE+1024*STM32_FLASH_SIZE)) return 1;

    if((startAddr-STM32_FLASH_BASE+pagesum*FLASH_PAGE_SIZE)>(1024*STM32_FLASH_SIZE))  return 1;

    for(i=0; i<pagesum; i++)
    {
        FLASH_ErasePage(startAddr+i*FLASH_PAGE_SIZE);//²Á³ýÕâ¸öÉÈÇ
        delay_ms(10);
    }
    return 0;
}

unsigned char chip_crc_flash( unsigned int startAddr,unsigned int len,unsigned int *crc)
{
    unsigned int length=len,data_size;
    unsigned char buf[64]= {0};
    unsigned int local_prevcrc=0,local_addr;
    if(crc==NULL)   return 1;
    if((startAddr<STM32_FLASH_BASE)||(startAddr>=STM32_FLASH_BASE+1024*STM32_FLASH_SIZE) ||((startAddr+len)>(STM32_FLASH_BASE+1024*STM32_FLASH_SIZE)))return 1;


    local_addr=startAddr;
    do
    {
        memset(buf,0,64);
        if(length>64) data_size=64;
        else data_size=length;
        if(chip_read_flash(local_addr,(unsigned short *)buf,data_size/2)!=0)
        {
            return 1;
        }
        local_prevcrc=u4_oscrc32(local_prevcrc,buf,data_size);
        length-=data_size;
        local_addr+=data_size;
    } while(length>0);

    *crc=local_prevcrc;
    return 0;
}

