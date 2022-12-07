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
#include <console.h>
#include <types.h>
#include <stdarg.h>

/******************************************************************************/

void 
__sidt(idt_t* dst)
{
    __asm__ __volatile__ (
        "sidt %0" 
        : /* output operands */
        "=m"(*dst)
    );
}

void 
__sgdt(gdt_t* dst)
{
    __asm__ __volatile__ (
        "sgdt %0" 
        : /* output operands */
        "=m"(*dst)
    );
}

uint32_t
__cr0()
{
    unsigned long val;

    __asm__ __volatile__ ( "mov %%cr0, %0" : "=r"(val) );

    return val;
}

uint16_t 
__cs()
{
    uint16_t seg = 0;

    __asm__ __volatile__ ("movw %%cs,%0" : "=rm" (seg));

    return seg;
}

/******************************************************************************/

static void
__movsb(void* dst, void* src, size_t size)
{
    __asm__ volatile (
        "cld;"
        "rep movsb;"
        : /* output operands - none */
        : /* input operands */
        "D" (dst),  /* edi/di */
        "S" (src),  /* esi/si */
        "c" (size)  /* ecx/cx/cl */
        : /* clobbers */
        "memory" );
}

static void
__movsd(void* dst, void* src, size_t size)
{
    __asm__ volatile (
        "cld;"
        "rep movsd;"
        : /* output operands - none */
        : /* input operands */
        "D" (dst),  /* edi/di */
        "S" (src),  /* esi/si */
        "c" (size)  /* ecx/cx/cl */
        : /* clobbers */
        "memory" );
}

static void
__stosb(void* dst, uint8_t data, size_t size)
{
    __asm__ volatile (
        "cld;"
        "rep stosb;"
        : /* output operands - none */
        : /* input operands */
        "D" (dst),  /* edi/di */
        "a" (data), /* eax/ax/al */
        "c" (size)  /* ecx/cx/cl */
        : /* clobbers */
        "memory" );
}

static void
__stosd(void* dst, uint32_t data, size_t size)
{
    __asm__ volatile (
        "cld;"
        "rep stosl;"
        : /* output operands - none */
        : /* input operands */
        "D" (dst),  /* edi/di */
        "a" (data), /* eax/ax/al */
        "c" (size)  /* ecx/cx/cl */
        : /* clobbers */
        "memory" );
}

static uint8_t
actohex(char c)
{
    uint8_t r = 0;

    r |= (c >= '0' && c <= '9' ? c - '0' : 0);
    r |= (c >= 'A' && c <= 'F' ? 10 + c - 'A' : 0);
    r |= (c >= 'a' && c <= 'f' ? 10 + c - 'a' : 0);

    return r;
}

/******************************************************************************/

void 
outportb(uint16_t port, uint8_t val)
{
    __asm__ volatile (
        "outb %0, %1" 
        : /* output operands - none */
        : /* input operands */
        "a" (val),  /* eax/ax/al */
        "d" (port)  /* edx/dx/dl */
    );
}

void 
outportw(uint16_t port, uint16_t val)
{
    __asm__ volatile (
        "outw %0, %1" 
        : /* output operands - none */
        : /* input operands */
        "a" (val),  /* eax/ax/al */
        "d" (port)  /* edx/dx/dl */
    );
}

uint8_t 
inportb(uint16_t port)
{
    uint8_t val;

    __asm__ volatile (
        "inb %1, %0" 
        : /* output operands */
        "=a" (val)  /* eax/ax/al */
        : /* input operands */
        "d" (port)  /* edx/dx/dl */
    );

    return val;
}

uint16_t 
inportw(uint16_t port)
{
    uint16_t val;

    __asm__ volatile (
        "inw %1, %0" 
        : /* output operands */
        "=a" (val)  /* eax/ax/al */
        : /* input operands */
        "d" (port)  /* edx/dx/dl */
    );

    return val;
}

void 
io_delay(void)
{
    const uint16_t DELAY_PORT = 0x80;

    __asm__ __volatile__("outb %%al,%0" : : "dN" (DELAY_PORT));
}

void
cpu_relax(void)
{
    __asm__ __volatile__("rep; nop" ::: "memory");
}

uint64_t 
rdtsc(void)
{
    uint32_t eax;
    uint32_t edx;
    
    __asm__ volatile(
        "rdtsc;"
        : /* output operands */
        "=a" (eax), 
        "=d" (edx) );
    
    return (uint64_t)eax | (uint64_t)edx << 32;
}

void
memcpy(void* dst, void* src, size_t size)
{
    if(!(size & 3))
    {
        /* if size is a multiple of 4 */
        __movsd(dst, src, size >> 2);
    }
    else
    {
        /* move byte-by-byte */
        __movsb(dst, src, size);
    }
}

void
memset(void* dst, uint32_t data, size_t size)
{
#if 1

    if(!(size & 3))
    {
        /* if size is a multiple of 4 */
        __stosd((uint32_t *)dst, data, size >> 2);
    }
    else
    {
        /* store byte-by-byte */
        __stosb((uint8_t *)dst, (uint8_t)data, size);
    }

#else

    if(!(size & 3))
    {
        for(int i = 0; i < (size >> 2); i++)
        {
            ((uint32_t *)dst)[i] = data;
        }
    }
    else
    {
        for(int i = 0; i < size; i++)
        {
            ((uint8_t *)dst)[i] = data;
        }
    }

#endif
}

int
memcmp(void* a, void* b, size_t n)
{
    if(n != 0) 
    {
        uint8_t* tmp_a = (uint8_t *)a;
        uint8_t* tmp_b = (uint8_t *)b;

        do 
        {
            if(*tmp_a++ != *tmp_b++)
            {
                return (*(--tmp_a) - *(--tmp_b));
            }
        
        } while(--n != 0);
    }

    return 0;
}

uint32_t
get_rtc_seconds(void)
{
    #define bcd_to_int(a) (((a & 0xF0) >> 4) * 10 + (a & 0x0F))

#if 1

    rmode_ctx_t ctx;

    ctx.ah = 0x02;
    ctx.al = 0;

    ldr_bios_call(0x1A, &ctx);

    return ( bcd_to_int(ctx.ch) * 3600 + 
        bcd_to_int(ctx.cl) * 60 + 
        bcd_to_int(ctx.dh) );

#else

    uint8_t bcd_fmt;

    /* CMOS RTC I/O mapped registers */

    /* read the status register b: data BCD formatted? */
    outportb(0x70, 0x0B);
    bcd_fmt = !(inportb(0x71) & 0x04);

    /* seconds */
    outportb(0x70, 0x00);

    if(bcd_fmt)
    {
        return bcd_to_int(inportb(0x71));
    }
    else
    {
        return inportb(0x71);
    }

#endif

    #undef bcd_to_int
}

/******************************************************************************/

char
tolower(char c)
{
    /* ansi char to lowercase */
    return ((c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c);
}

int 
isdigit(char c)
{
    return (c >= '0' && c <= '9');
}

int 
isspace(char c)
{
    return (c == '\n' || c == '\r' || c == ' ' || c == '\t');
}

int
strlen(char* s)
{
    int len = 0;

    for(/* */; *s++ != 0; len++);
        
    return len;
}

int
wcslen(wchar_t* s)
{
    int len = 0;

    for(/* */; *s++ != 0; len++);
        
    return len;
}

char*
strcpy(char* a, char* b)
{
    char* tmp = a;

    for(/* */; (*a = *b) != 0; a++, b++);
    
    return tmp;
}

char*
strcat(char* a, char* b)
{
    char* tmp = a;

    /* go to the end of the string */
    for(/* */; *a != 0; ++a);

    /* concat the string pointed by b */
    while((*a++ = *b++) != 0);
    
    return tmp;
}

char*
strcat_wtoa(char* a, wchar_t* b)
{
    char* tmp = a;

    /* go to the end of the string */
    for(/* */; *a != 0; ++a);

    /* concat the string pointed by b */
    while((*a++ = (char)*b++) != 0);
    
    return tmp;
}

wchar_t*
wcscat(wchar_t* a, wchar_t* b)
{
    char* tmp = (char *)a;

    /* go to the end of the string */
    for(/* */; *a != 0; ++a);

    /* concat the string pointed by b */
    while((*a++ = *b++) != 0);
    
    return (wchar_t *)tmp;
}

char*
strncat(char* a, char* b, int n)
{
    char* tmp = a;

    /* go to the end of the string */
    for(/* */; *a != 0; ++a);

    /* concat the string pointed by b */
    while(n-- && (*a++ = *b++) != 0);
    
    return tmp;
}

char* 
strncpy(char* a, char* b, int n)
{
    char* tmp = a;

    while(n-- && *b != 0)
    {
        *(a++) = *(b++);
    }

    /* null terminate */
    *a = 0;

    return tmp;
}

int
strcmp(char* a, char* b)
{
    while(*a == *b++)
    {
        if(*a++ == 0)
        {
            /* reached end of string a */
            return 0;
        }
    }
    
    return (*a - *(b - 1));
}

void 
strrev(char* s)
{
    char tmp;

    int i;
    int j;

    for(i = 0, j = strlen(s) - 1; i < j; i++, j--) 
    {
        tmp = s[i];
        s[i] = s[j];
        s[j] = tmp;
    }
}  

void
itoa(unsigned int n, char* s, int len, int base)
{
    unsigned int neg;
    unsigned int digit;
    unsigned int sum;

    int i = 0;

    /* check for buffer length */
    if(len == 0)
    {
        return;
    }

    /* mark sign and make it positive */
    neg = (((int)n) < 0 && base == 10 ? 1 : 0);
    sum = (neg ? (unsigned int)(-((int)n)) : n);

    do
    {
        digit = sum % base;

        /* print out the digit in ascii */
        if(digit < 10)
        {
            s[i++] = '0' + digit;
        }
        else
        {
            s[i++] = 'A' + digit - 0xA;
        }

        sum /= base;

    } while(sum != 0 && (i < (len - 2)));

    /* check if there wasnt enough room in the buffer */
    if(i == (len - 2) && sum != 0)
    {
        return;
    }

    /* mark the sign */
    if(neg)
    {
        s[i++] = '-';
    }

    /* null-terminate the string */
    s[i] = '\0';

    /* reverse the string */
    strrev(s);
}

void 
sprintf(char* buffer, char* fmt, ...)
{
    char c;
    char tmp[32];

    va_list args;
    va_start(args, fmt);

    while((c = *fmt++) != 0)
    {
        if(c != '%') 
        {
            tmp[0] = c;
            tmp[1] = 0;

            strcat(buffer, tmp);
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
                    itoa(va_arg(args, int), tmp, sizeof(tmp), 
                        ((c == 'x') ? 16 : 10));

                    strcat(buffer, tmp);
                    break;    
                }    
                case 'c':
                {
                    tmp[0] = (char)(va_arg(args, int) & 0xFF);
                    tmp[1] = 0;
            
                    strcat(buffer, tmp);
                    break; 
                }
                case 's':
                {
                    char* p = va_arg(args, char *);
                    
                    if(p != 0) 
                    {
                        strcat(buffer, p);
                    }  
                    else 
                    {
                        strcat(buffer, "(null)");
                    }

                    break;
                }
                default:
                {
                    tmp[0] = c;
                    tmp[1] = 0;

                    strcat(buffer, tmp);
                    break;
                }
            }
        }
    }

    va_end(args);
}

int32_t
atoi(char* s, char** end)
{
    char        c;
    char*       tmp = s;
    bool_t      neg = false;
    
    int32_t     any = 0;
    uint32_t    acc = 0;
    int32_t     cutlim;
    uint32_t    cutoff;

    /* find the first char */
    for(/* */; (c = *s) == ' '; s++);

    /* check the sign */
    if(c == '-')
    {
        neg = true;
        c = *s++;
    }
    else
    {
        /* assume positive */
        if(c == '+')
        {
            c = *s++;
        }
    }

    #define LONG_MAX    2147483647L
    #define LONG_MIN    (-LONG_MAX - 1L)

    cutoff = (neg ? -(uint32_t)LONG_MIN : LONG_MAX);
    cutlim = cutoff % 10;
    cutoff = cutoff / 10;

    while(true) 
    {
        c = *s++;

        if(!(c >= '0' && c <= '9'))
        {
            break;
        }

        /* ASCII to number */
        c -= '0';

        if( (any < 0) || 
            (acc > cutoff) || 
            (acc == cutoff && c > cutlim) )
        {
            /* overflow */
            any = -1;
        }
        else 
        {
            /* digits consumed */
            any = 1;
            acc *= 10;
            acc += c;
        }
    }

    if(any < 0) 
    {
        /* overflow happened: out of range */
        acc = (neg ? LONG_MIN : LONG_MAX);
    } 
    else 
    if(neg)
    {
        /* value is negative */
        acc = -acc;
    }

    if(end)
    {
        /* set pointer at the end of the number */
        *end = (any ? s - 1 : tmp);
    }

    return acc;
}

void*
findpattern(void* start, size_t size, char* ptrn)
{
    size_t      pos = 0;
    uint8_t     hex;
    uint8_t*    match = NULL;
    uint8_t*    curr = (uint8_t *)start;

    for(/* */; size--; curr++)
    {
        if(!ptrn[pos])
        {
            /* pattern found */
            return match;
        }

        for(/* */; ptrn[pos] == ' '; pos++);

        hex = 0;
        hex |= (actohex(ptrn[pos]) & 0x0f) << 4;
        hex |= (actohex(ptrn[pos+1]) & 0x0f);
    
        if(ptrn[pos] == '?' || ptrn[pos+1] == '?')
        {
            /* wildcards, logical and */
            pos = ((*curr & hex) == hex ? pos + 2 : 0);
        }
        else
        {
            pos = (*curr == hex ? pos + 2 : 0);
        }

        /* first match */
        match = (pos == 2 ? curr : match);

        /* previous pattern didnt match, so reset */
        match = (pos == 0 ? NULL : match);
    }

    return 0;
}

void*
findstr_a(void* start, size_t size, char* str)
{
    size_t  pos = 0;
    char*   match = NULL;
    char*   curr = (char *)start;

    for(/* */; size--; curr++)
    {
        if(!str[pos])
        {
            return match;
        }

        /* check */
        pos = (*curr == str[pos] ? pos + 1 : 0);
        match = (pos == 1 ? curr : 
                (pos == 0 ? NULL : match));
    }

    return 0;
}

void*
findstr_w(void* start, size_t size, wchar_t* str)
{
    size_t      pos = 0;
    wchar_t*    match = NULL;
    wchar_t*    curr = (wchar_t *)start;

    for(/* */; size -= 2; curr++)
    {
        if(!str[pos])
        {
            return match;
        }

        /* check */
        pos = (*curr == str[pos] ? pos + 1 : 0);
        match = (pos == 1 ? curr : 
                (pos == 0 ? NULL : match));
    }

    return 0;
}

