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
#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#include <types.h>

/******************************************************************************/

#define FG_BLACK            (0)
#define FG_BLUE             (1)
#define FG_GREEN            (2)
#define FG_CYAN             (3)
#define FG_RED              (4)
#define FG_MAGENTA          (5)
#define FG_BROWN            (6)
#define FG_LGRAY            (7)
#define FG_DGRAY            (8) 
#define FG_LBLUE            (9)
#define FG_LGREEN           (10)
#define FG_LCYAN            (11)
#define FG_LRED             (12)
#define FG_LMAGENTA         (13)
#define FG_YELLOW           (14)
#define FG_WHITE            (15)

#define BG_BLACK            (FG_BLACK       << 4)
#define BG_BLUE             (FG_BLUE        << 4)
#define BG_GREEN            (FG_GREEN       << 4)
#define BG_CYAN             (FG_CYAN        << 4)
#define BG_RED              (FG_RED         << 4)
#define BG_MAGENTA          (FG_MAGENTA     << 4)
#define BG_BROWN            (FG_BROWN       << 4)
#define BG_LGRAY            (FG_LGRAY       << 4)
#define BG_DGRAY            (FG_DGRAY       << 4)   
#define BG_LBLUE            (FG_LBLUE       << 4)
#define BG_LGREEN           (FG_LGREEN      << 4)
#define BG_LCYAN            (FG_LCYAN       << 4)
#define BG_LRED             (FG_LRED        << 4)
#define BG_LMAGENTA         (FG_LMAGENTA    << 4)
#define BG_YELLOW           (FG_YELLOW      << 4)
#define BG_WHITE            (FG_WHITE       << 4)

#define SERIAL_PORT1        0x3F8 /* COM 1, ttyS0 */
#define SERIAL_PORT2        0x2F8 /* COM 2, ttyS1 */
#define SERIAL_PORT3        0x3E8 /* COM 3, ttyS2 */
#define SERIAL_PORT4        0x2E8 /* COM 4, ttyS3 */

/******************************************************************************/

void console_init(int serial_port);

char getch(void);

void putch(char c, uint8_t color);

void puts(char* str, uint8_t color);

void get_cursor(unsigned int* row, unsigned int* col);

void set_cursor(unsigned int row, unsigned int col);

void printf(uint8_t color, char* fmt, ...);

#endif //_CONSOLE_H_ 
 
