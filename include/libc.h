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
#ifndef _LIBC_H_
#define _LIBC_H_

#include <types.h>

/******************************************************************************/

typedef struct 
{
    uint16_t limit;

    union
    {
        uint32_t base32;
        uint64_t base64;
    };
    
} __attribute__((packed)) idt_t;

typedef struct 
{
    uint16_t limit;

    union
    {
        uint32_t base32;
        uint64_t base64;
    };
    
} __attribute__((packed)) gdt_t;

/******************************************************************************/

void __sidt(idt_t* dst);

void __sgdt(gdt_t* dst);

uint64_t __rdmsr(uint64_t msr);

uint64_t __rflags();

uint16_t __cs();

void memcpy(void* dst, void* src, size_t size);

void memset(void* dst, uint32_t data, size_t size);

int memcmp(void* a, void* b, size_t n);

void outportb(uint16_t port, uint8_t val);

void outportw(uint16_t port, uint16_t val);

uint8_t inportb(uint16_t port);

uint16_t inportw(uint16_t port);

uint64_t rdtsc(void);

void io_delay(void);

void cpu_relax(void);

uint32_t get_rtc_seconds(void);

char tolower(char c);

int isdigit(char c);

int strlen(char* s);

int wcslen(wchar_t* s);

void itoa(unsigned int n, char* s, int len, int base);

int32_t atoi(char* s, char** end);

char* strcpy(char* a, char* b);

char* strcat(char* a, char* b);

char* strcat_wtoa(char* a, wchar_t* b);

char* strncat(char* a, char* b, int n);

char* strncpy(char* a, char* b, int n);

int strcmp(char* a, char* b);

void sprintf(char* buffer, char* fmt, ...);

void* findpattern(void* start, size_t size, char* ptrn);

void* findstr_a(void* start, size_t size, char* str);

void* findstr_w(void* start, size_t size, wchar_t* str);

#endif //_LIBC_H_ 