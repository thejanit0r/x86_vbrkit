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
#include <video.h>
#include <console.h>
#include <serial.h>
#include <bios.h>

/******************************************************************************/

static int console_serial_port = 0;

/******************************************************************************/

void
console_init(int serial_port)
{
    if(serial_port == 0)
    {
        video_init();
    }
    else
    {
        serial_port_init(serial_port);
    }

    console_serial_port = serial_port;
}

char 
getch(void)
{
    if(console_serial_port == 0)
    {
        rmode_ctx_t ctx;

        /* wait for keystroke and read */
        ctx.ah = BIOS_SVC_KEYBOARD_GETCHAR;

        ldr_bios_call(BIOS_SVC_KEYBOARD, &ctx);

        return (char)ctx.al;
    }
    else
    {
        return serial_getch(console_serial_port);
    }
}

void
putch(char c, uint8_t color)
{
    if(console_serial_port == 0)
    {
        rmode_ctx_t ctx;

        /* write text in teletype mode */
        if(c == '\n') 
        {
            putch('\r', color);
        }

        ctx.ah = BIOS_SVC_VIDEO_PUTCHAR;
        ctx.al = c;
        ctx.bh = 0x00; /* video page number */
        ctx.bl = ((color >> 0) & 0x0F);

        ldr_bios_call(BIOS_SVC_VIDEO, &ctx);
    }
    else
    {
        serial_putch(console_serial_port, c);
    }
}

void 
puts(char* str, uint8_t color)
{
    while(*str != 0) 
    {
        putch(*str++, color);
    }       
}

void 
putsw(wchar_t* str, uint8_t color)
{
    while(*str != 0) 
    {
        putch((char)*str++, color);
    }       
}

void
get_cursor(unsigned int* row, unsigned int* col)
{
    rmode_ctx_t ctx;

    ctx.ah = BIOS_SVC_VIDEO_GET_CURSOR_POS;
    ctx.bh = 0x00; /* video page number */

    ldr_bios_call(BIOS_SVC_VIDEO, &ctx);

    *row = ctx.dh;
    *col = ctx.dl;
}

void
set_cursor(unsigned int row, unsigned int col)
{
    rmode_ctx_t ctx;

    ctx.ah = BIOS_SVC_VIDEO_SET_CURSOR_POS;
    ctx.bh = 0x00; /* video page number */
    ctx.dh = row;
    ctx.dl = col;

    ldr_bios_call(BIOS_SVC_VIDEO, &ctx);
}

void 
printf(uint8_t color, char* fmt, ...)
{
    char c;
    char tmp[32];

    va_list args;
    va_start(args, fmt);

    while((c = *fmt++) != 0)
    {
        if(c != '%') 
        {
            putch(c, color);
        } 
        else 
        {
            /* get the format specifier */
            c = *fmt++;

            switch(c)
            {
                case 'p':
                {
                    c = 'x';
                }
                case 'x':
                case 'u':
                case 'd':
                {
                    int n = va_arg(args, int);

                    itoa(n, tmp, sizeof(tmp), ((c == 'x') ? 16 : 10));
                    puts(tmp, color);
                    break;    
                }    
                case 'c':
                {
                    int n = va_arg(args, int);

                    putch((n & 0xFF), color);
                    break; 
                }
                case 's':
                {
                    char* p = va_arg(args, char *);
                    
                    if(p != 0) 
                    {
                        puts(p, color);
                    }  
                    else 
                    {
                        puts("(null)", color);
                    }

                    break;
                }
                case 'S':
                {
                    wchar_t* p = va_arg(args, wchar_t *);
                    
                    if(p != 0) 
                    {
                        putsw(p, color);
                    }  
                    else 
                    {
                        puts("(null)", color);
                    }

                    break;
                }
                default:
                {
                    putch(c, color);
                    break;
                }
            }
        }
    }

    va_end(args);
}
