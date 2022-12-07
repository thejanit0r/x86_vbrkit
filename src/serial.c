/*

    This file is part of x86_vbrkit.

    Copyright 2017 / the`janitor / < email: base64dec(dGhlLmphbml0b3JAcHJvdG9ubWFpbC5jb20=) >

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

*/
#include <shared.h>
#include <libc.h>
#include <types.h>
#include <stdarg.h>
#include <serial.h>

/******************************************************************************/

#define SERIAL_PORT SERIAL_PORT1

/******************************************************************************/

void 
serial_port_init(int port)
{
    /* 
        More info @ linux/serial_reg.h
        Alternatively, could use BIOS' serial services: INT 14H / AH=00h, 01h, 02h
    */
    outportb(port + UART_IER, 0x00); /* disable all interrupts */
    outportb(port + UART_LCR, UART_LCR_DLAB); /* change config mode */

    /* 
        Baud rate divisor values 
    
        0x01 : 115200 bps 
        0x02 :  57600 bps 
        0x06 :  19200 bps 
        0x0C :   9600 bps 
        0x18 :   4800 bps 
        0x30 :   2400 bps   
    */
    outportb(port + UART_DLL, 0x01); /* 115200 bps */
    outportb(port + UART_DLM, 0x00);
    outportb(port + UART_LCR, UART_LCR_WLEN8); /* 8-bit, no parity, 1 stop bit */
    outportb(port + UART_FCR, 0x00); /* no FIFO */

    // outportb(port + UART_FCR, 0xC7); 
    // outportb(port + UART_MCR, UART_MCR_DTR | UART_MCR_RTS);
}

bool_t
serial_port_initialized(int port)
{
    /* loopback mode */
    outportb(port + UART_MCR, UART_MCR_LOOP | 
        UART_MCR_OUT1 | 
        UART_MCR_OUT1 | 
        UART_MCR_RTS);
    
    /* write a byte to the serial... */
    outportb(port + UART_TX, 0xAE);

    /* ...and read it back */
    if(inportb(port + UART_RX) != 0xAE)
    {
        return false;
    }

    /* set normal operation mode */
    outportb(port + UART_MCR, 0x00);

    return true;
}

char 
serial_getch(int port)
{
    while((inportb(port + UART_LSR) & UART_LSR_DR) == 0)
        cpu_relax();
 
    /* receive */
    return inportb(port + UART_RX);
}

void
serial_putch(int port, char c)
{
    while(true) 
    {
        /* check if we can transmit */
        if((inportb(port + UART_LSR) & BOTH_EMPTY) == BOTH_EMPTY)
            break;

        cpu_relax();
    }

    /* transmit */
    outportb(port + UART_TX, c);
}

void 
serial_puts(int port, char* str)
{
    while(*str != 0) 
    {
        serial_putch(port, *str++);
    }       
}