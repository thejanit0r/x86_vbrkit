/*

    This file is part of x86_vbrkit.

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
#ifndef _SERIAL_H_
#define _SERIAL_H_

#include <types.h>

/******************************************************************************/

#define SERIAL_PORT1    0x3F8 /* COM 1, ttyS0 */
#define SERIAL_PORT2    0x2F8 /* COM 2, ttyS1 */
#define SERIAL_PORT3    0x3E8 /* COM 3, ttyS2 */
#define SERIAL_PORT4    0x2E8 /* COM 4, ttyS3 */

/* DLAB=0 */
#define UART_RX                 0       /* In:  Receive buffer */
#define UART_TX                 0       /* Out: Transmit buffer */

#define UART_IER                0x01    /* Out: Interrupt Enable Register */
#define UART_IER_MSI            0x08    /* Enable Modem status interrupt */
#define UART_IER_RLSI           0x04    /* Enable receiver line status interrupt */
#define UART_IER_THRI           0x02    /* Enable Transmitter register int. */
#define UART_IER_RDI            0x01    /* Enable receiver data interrupt */

#define UART_FCR                0x02    /* Out: FIFO Control Register */
#define UART_FCR_ENABLE_FIFO    0x01    /* Enable the FIFO */
#define UART_FCR_CLEAR_RCVR     0x02    /* Clear the RCVR FIFO */
#define UART_FCR_CLEAR_XMIT     0x04    /* Clear the XMIT FIFO */
#define UART_FCR_DMA_SELECT     0x08    /* For DMA applications */
#define UART_FCR_R_TRIG_00      0x00
#define UART_FCR_R_TRIG_01      0x40
#define UART_FCR_R_TRIG_10      0x80
#define UART_FCR_R_TRIG_11      0xC0
#define UART_FCR_T_TRIG_00      0x00
#define UART_FCR_T_TRIG_01      0x10
#define UART_FCR_T_TRIG_10      0x20
#define UART_FCR_T_TRIG_11      0x30

#define UART_LCR                0x03    /* Out: Line Control Register */
#define UART_LCR_DLAB           0x80    /* Divisor latch access bit */
#define UART_LCR_SBC            0x40    /* Set break control */
#define UART_LCR_SPAR           0x20    /* Stick parity (?) */
#define UART_LCR_EPAR           0x10    /* Even parity select */
#define UART_LCR_PARITY         0x08    /* Parity Enable */
#define UART_LCR_STOP           0x04    /* Stop bits: 0=1 bit, 1=2 bits */
#define UART_LCR_WLEN5          0x00    /* Wordlength: 5 bits */
#define UART_LCR_WLEN6          0x01    /* Wordlength: 6 bits */
#define UART_LCR_WLEN7          0x02    /* Wordlength: 7 bits */
#define UART_LCR_WLEN8          0x03    /* Wordlength: 8 bits */

/* Access to some registers depends on register access / configuration mode */
#define UART_LCR_CONF_MODE_A    UART_LCR_DLAB   /* Configutation mode A */
#define UART_LCR_CONF_MODE_B    0xBF            /* Configutation mode B */

#define UART_MCR                0x04    /* Out: Modem Control Register */
#define UART_MCR_CLKSEL         0x80    /* Divide clock by 4 */
#define UART_MCR_TCRTLR         0x40    /* Access TCR/TLR */
#define UART_MCR_XONANY         0x20    /* Enable Xon Any */
#define UART_MCR_AFE            0x20    /* Enable auto-RTS/CTS */
#define UART_MCR_LOOP           0x10    /* Enable loopback test mode */
#define UART_MCR_OUT2           0x08    /* Out2 complement */
#define UART_MCR_OUT1           0x04    /* Out1 complement */
#define UART_MCR_RTS            0x02    /* RTS complement */
#define UART_MCR_DTR            0x01    /* DTR complement */

#define UART_LSR                0x05    /* In:  Line Status Register */
#define UART_LSR_FIFOE          0x80    /* FIFO error */
#define UART_LSR_TEMT           0x40    /* Transmitter empty */
#define UART_LSR_THRE           0x20    /* Transmit-hold-register empty */
#define UART_LSR_BI             0x10    /* Break interrupt indicator */
#define UART_LSR_FE             0x08    /* Frame error indicator */
#define UART_LSR_PE             0x04    /* Parity error indicator */
#define UART_LSR_OE             0x02    /* Overrun error indicator */
#define UART_LSR_DR             0x01    /* Receiver data ready */
#define UART_LSR_BRK_ERROR_BITS 0x1E    /* BI, FE, PE, OE bits */
#define BOTH_EMPTY              (UART_LSR_TEMT | UART_LSR_THRE)

#define UART_MSR                0x06    /* In:  Modem Status Register */
#define UART_MSR_DCD            0x80    /* Data Carrier Detect */
#define UART_MSR_RI             0x40    /* Ring Indicator */
#define UART_MSR_DSR            0x20    /* Data Set Ready */
#define UART_MSR_CTS            0x10    /* Clear to Send */
#define UART_MSR_DDCD           0x08    /* Delta DCD */
#define UART_MSR_TERI           0x04    /* Trailing edge ring indicator */
#define UART_MSR_DDSR           0x02    /* Delta DSR */
#define UART_MSR_DCTS           0x01    /* Delta CTS */
#define UART_MSR_ANY_DELTA      0x0F    /* Any of the delta bits! */

/* DLAB=1 */
#define UART_DLL                0       /* Out: Divisor Latch Low */
#define UART_DLM                1       /* Out: Divisor Latch High */
#define UART_DIV_MAX            0xFFFF  /* Max divisor value */

#define UART_EFR                2       /* I/O: Extended Features Register */
#define UART_EFR_CTS            0x80    /* CTS flow control */
#define UART_EFR_RTS            0x40    /* RTS flow control */
#define UART_EFR_SCD            0x20    /* Special character detect */
#define UART_EFR_ECB            0x10    /* Enhanced control bit */

/******************************************************************************/

void serial_port_init(int port);

bool_t serial_port_initialized(int port);

char serial_getch(int port);

void serial_putch(int port, char c);

void serial_puts(int port, char* str);

#endif //_SERIAL_H_ 
 
