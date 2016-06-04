//*****************************************************************************
//
//      COPYRIGHT (c) ATMEL Norway, 1996-2001
//
//      The copyright to the document(s) herein is the property of
//      ATMEL Norway, Norway.
//
//      The document(s) may be used  and/or copied only with the written
//      permission from ATMEL Norway or in accordance with the terms and
//      conditions stipulated in the agreement/contract under which the
//      document(s) have been supplied.
//
//*****************************************************************************
//
//  File........: DATAFLASH.C
//
//  Author(s)...: ATMEL Norway
//
//  Target(s)...: All AVRs with built-in HW SPI
//
//  Description.: Functions to access the Atmel AT45Dxxx Dataflash series
//                Supports 512Kbit - 64Mbit
//
//  Revisions...:
//
//  YYYYMMDD - VER. - COMMENT                                       - SIGN.
//
//  20011017 - 1.00 - Beta release                                  -  RM
//  20011017 - 0.10 - Generated file                                -  RM
//  20031009          port to avr-gcc/avr-libc                      - M.Thomas
//  20040121          added compare and erase function              - M.Thomas
//
//*****************************************************************************
/*
   remark mthomas: If you plan to use the Dataflash functions in own code
   for (battery powered) devices: disable the "chip select" after accessing
   the Dataflash. The current draw with cs enabled is "very" high. You can
   simply use the macro _at45_DF_CS_inactive already defined by Atmel after every
   DF access

   The coin-cell battery on the Butterfly is not a reliable power-source if data
   in the flash-array should be changed (write/erase).
   See the Dataflash datasheet for the current needed during write-accesses.
 */
//*****************************************************************************
/*
  Dirk Spaanderman: changed the dataflash code to a c++ library for arduino
 */


/*****************************************************************************

Filename:    DataFlash.cpp
Description: DataFlash library file for the FlashShield 1.0

******************************************************************************

DataFlash library for the FlashShield 1.0

Copyright(c) 2011 Async Labs Inc. All rights reserved.

This program is free software; you can redistribute it and/or modify it
under the terms of version 2 of the GNU General Public License as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59
Temple Place - Suite 330, Boston, MA 02111-1307, USA.

Contact Information:
<asynclabs@asynclabs.com>

-----------------------------------------------------------------------------
Author    Date       Comment
-----------------------------------------------------------------------------
AsyncLabs 03/17/2010 Initial version
AsyncLabs 01/10/2011 Clean up

*****************************************************************************/

#include "at45.h"
#include "periph/gpio.h"
#include "periph/spi.h"
#include "xtimer.h"

// configuration for the Atmel AT45DB161D device
#define PAGE_BITS    10
#define PAGE_SIZE    528

static uint8_t _at45_DF_SPI_RW(at45_t *dev, uint8_t byte);
static uint16_t _at45_Read_DF_status(at45_t *dev);

int at45_init(at45_t *dev, spi_t spi, gpio_t cs, gpio_t rst, gpio_t wp, gpio_t pwr)
{
    dev->cs = cs;
    dev->rst = rst;
    dev->wp = wp;
    dev->pwr = pwr;
    dev->spi = spi;
    int ret;

    gpio_set(dev->cs);
    gpio_set(dev->wp);
    gpio_clear(dev->pwr); //on
    gpio_clear(dev->rst);

    gpio_init(dev->pwr, GPIO_OUT);
    gpio_init(dev->rst, GPIO_OUT);
    gpio_init(dev->wp, GPIO_OUT);
    gpio_init(dev->cs, GPIO_OUT);

    xtimer_usleep(100*1000);

    gpio_set(dev->rst);

    xtimer_usleep(100*1000);


    if ((ret = spi_init_master(dev->spi, SPI_CONF_FIRST_RISING, SPI_SPEED_100KHZ)))
        return ret;

    uint16_t status = _at45_Read_DF_status(dev);
    printf("flash status: 0x%04x\n", status);

    uint8_t id[4];
    at45_Read_DF_ID(dev, id);

    return 0;
}

static uint8_t _at45_DF_SPI_RW(at45_t *dev, uint8_t byte)
{
    char receive;
    spi_transfer_byte(dev->spi, byte, &receive);
    return (uint8_t)receive;
}

uint16_t _at45_Read_DF_status(at45_t *dev)
{
    uint16_t result;

    gpio_clear(dev->cs);
    _at45_DF_SPI_RW(dev, StatusReg);
    result = _at45_DF_SPI_RW(dev, 0x00);
    result += _at45_DF_SPI_RW(dev, 0x00) << 8;
    gpio_set(dev->cs);

    return result;
}

void at45_Read_DF_ID(at45_t *dev, uint8_t *id)
{
    gpio_clear(dev->cs);
    _at45_DF_SPI_RW(dev, ReadMfgID);
    id[0] = _at45_DF_SPI_RW(dev, 0x00);
    id[1] = _at45_DF_SPI_RW(dev, 0x00);
    id[2] = _at45_DF_SPI_RW(dev, 0x00);
    id[3] = _at45_DF_SPI_RW(dev, 0x00);
    gpio_set(dev->cs);
}

/*****************************************************************************
 *
 *  Function name : Page_To_Buffer
 *
 *  Returns :       None
 *
 *  Parameters :    BufferNo    ->  Decides usage of either buffer 1 or 2
 *
 *                  PageAdr     ->  Address of page to be transferred to buffer
 *
 *  Purpose :   Transfers a page from flash to Dataflash SRAM buffer
 *
 *
 ******************************************************************************/
void at45_Page_To_Buffer(at45_t *dev, unsigned int PageAdr, unsigned char BufferNo)
{
    gpio_clear(dev->cs);
    if (1 == BufferNo)                                              //transfer flash page to buffer 1
    {
        _at45_DF_SPI_RW(dev, FlashToBuf1Transfer);
        _at45_DF_SPI_RW(dev, (unsigned char)(PageAdr >> (16 - PAGE_BITS)));    //upper part of page address
        _at45_DF_SPI_RW(dev, (unsigned char)(PageAdr << (PAGE_BITS - 8)));     //lower part of page address
        _at45_DF_SPI_RW(dev, 0x00);                                            //don't cares
    }
#ifdef USE_BUFFER2
    else if (2 == BufferNo)                                         //transfer flash page to buffer 2
    {
        _at45_DF_SPI_RW(dev, FlashToBuf2Transfer);
        _at45_DF_SPI_RW(dev, (unsigned char)(PageAdr >> (16 - PAGE_BITS)));    //upper part of page address
        _at45_DF_SPI_RW(dev, (unsigned char)(PageAdr << (PAGE_BITS - 8)));     //lower part of page address
        _at45_DF_SPI_RW(dev, 0x00);                                            //don't cares
    }
#endif
    gpio_set(dev->cs);

    while(!(_at45_Read_DF_status(dev) & 0x80));      //monitor the status register, wait until busy-flag is high
}

/*****************************************************************************
 *
 *  Function name : Buffer_Read_Byte
 *
 *  Returns :       One read byte (any value)
 *
 *  Parameters :    BufferNo    ->  Decides usage of either buffer 1 or 2
 *
 *                  IntPageAdr  ->  Internal page address
 *
 *  Purpose :       Reads one byte from one of the Dataflash
 *
 *                  internal SRAM buffers
 *
 ******************************************************************************/
unsigned char at45_Buffer_Read_Byte(at45_t *dev, unsigned char BufferNo, unsigned int IntPageAdr)
{
    unsigned char data = 0;

    gpio_clear(dev->cs);
    if (1 == BufferNo)      //read byte from buffer 1
    {
        _at45_DF_SPI_RW(dev, Buf1Read);            //buffer 1 read op-code
        _at45_DF_SPI_RW(dev, 0x00);                //don't cares
        _at45_DF_SPI_RW(dev, (unsigned char)(IntPageAdr>>8));  //upper part of internal buffer address
        _at45_DF_SPI_RW(dev, (unsigned char)(IntPageAdr));     //lower part of internal buffer address
        _at45_DF_SPI_RW(dev, 0x00);                //don't cares
        data = _at45_DF_SPI_RW(dev, 0x00);         //read byte
    }
#ifdef USE_BUFFER2
    else if (2 == BufferNo)  //read byte from buffer 2
    {
        _at45_DF_SPI_RW(dev, Buf2Read);            //buffer 2 read op-code
        _at45_DF_SPI_RW(dev, 0x00);                //don't cares
        _at45_DF_SPI_RW(dev, (unsigned char)(IntPageAdr>>8));  //upper part of internal buffer address
        _at45_DF_SPI_RW(dev, (unsigned char)(IntPageAdr));     //lower part of internal buffer address
        _at45_DF_SPI_RW(dev, 0x00);                //don't cares
        data = _at45_DF_SPI_RW(dev, 0x00);         //read byte
    }
#endif
    gpio_set(dev->cs);

    return data;    //return the read data byte
}

/*****************************************************************************
 *
 *  Function name : Buffer_Write_Byte
 *
 *  Returns :       None
 *
 *  Parameters :    IntPageAdr  ->  Internal page address to write byte to
 *
 *          BufferAdr   ->  Decides usage of either buffer 1 or 2
 *
 *          Data        ->  Data byte to be written
 *
 *  Purpose :       Writes one byte to one of the Dataflash
 *
 *                  internal SRAM buffers
 *
 ******************************************************************************/
void at45_Buffer_Write_Byte(at45_t *dev, unsigned char BufferNo, unsigned int IntPageAdr, unsigned char Data)
{
    gpio_clear(dev->cs);
    if (1 == BufferNo)              //write byte to buffer 1
    {
        _at45_DF_SPI_RW(dev, Buf1Write);       //buffer 1 write op-code
        _at45_DF_SPI_RW(dev, 0x00);            //don't cares
        _at45_DF_SPI_RW(dev, (unsigned char)(IntPageAdr>>8));  //upper part of internal buffer address
        _at45_DF_SPI_RW(dev, (unsigned char)(IntPageAdr));     //lower part of internal buffer address
        _at45_DF_SPI_RW(dev, Data);            //write data byte
    }
#ifdef USE_BUFFER2
    else if (2 == BufferNo)         //write byte to buffer 2
    {
        _at45_DF_SPI_RW(dev, Buf2Write);       //buffer 2 write op-code
        _at45_DF_SPI_RW(dev, 0x00);            //don't cares
        _at45_DF_SPI_RW(dev, (unsigned char)(IntPageAdr>>8));  //upper part of internal buffer address
        _at45_DF_SPI_RW(dev, (unsigned char)(IntPageAdr));     //lower part of internal buffer address
        _at45_DF_SPI_RW(dev, Data);            //write data byte
    }
#endif
    gpio_set(dev->cs);
}

/*****************************************************************************
 *
 *
 *  Function name : Buffer_To_Page
 *
 *  Returns :       None
 *
 *  Parameters :    BufferAdr   ->  Decides usage of either buffer 1 or 2
 *
 *          PageAdr     ->  Address of flash page to be programmed
 *
 *  Purpose :   Transfers a page from Dataflash SRAM buffer to flash
 *
 *
 ******************************************************************************/
void at45_Buffer_To_Page(at45_t *dev, unsigned char BufferNo, unsigned int PageAdr)
{
    gpio_clear(dev->cs);
    if (1 == BufferNo)
    {
        _at45_DF_SPI_RW(dev, Buf1ToFlashWE);           //buffer 1 to flash with erase op-code
        _at45_DF_SPI_RW(dev, (unsigned char)(PageAdr >> (16 - PAGE_BITS))); //upper part of page address
        _at45_DF_SPI_RW(dev, (unsigned char)(PageAdr << (PAGE_BITS - 8)));  //lower part of page address
        _at45_DF_SPI_RW(dev, 0x00);                    //don't cares
    }
#ifdef USE_BUFFER2
    else if (2 == BufferNo)
    {
        _at45_DF_SPI_RW(dev, Buf2ToFlashWE);           //buffer 2 to flash with erase op-code
        _at45_DF_SPI_RW(dev, (unsigned char)(PageAdr >> (16 - PAGE_BITS)));    //upper part of page address
        _at45_DF_SPI_RW(dev, (unsigned char)(PageAdr << (PAGE_BITS - 8)));     //lower part of page address
        _at45_DF_SPI_RW(dev, 0x00);                    //don't cares
    }
#endif
    gpio_set(dev->cs);

    while(!(_at45_Read_DF_status(dev) & 0x80));      //monitor the status register, wait until busy-flag is high
}