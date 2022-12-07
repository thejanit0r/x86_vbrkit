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
#ifndef _SHARED_H_
#define _SHARED_H_

#include <types.h>

/******************************************************************************/

/* Disk Address Packet (DAP) */
typedef struct 
{
    uint8_t     packet_size;
    uint8_t     reserved;
    uint16_t    sectors_to_transfer;
    uint16_t    dst_offs;
    uint16_t    dst_seg;
    uint64_t    sector_start;

} __attribute__((packed)) dap_t;

/* System memory map address range descriptor */
typedef struct 
{
    uint64_t    base_address;
    uint64_t    size;
    uint32_t    type;

} __attribute__((packed)) mmap_addr_desc_t;

/* PUSHAD regs */
typedef struct 
{
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

} __attribute__((packed)) pushad_regs_t;

/* real mode isr callback context */
typedef struct 
{
    pushad_regs_t   saved_regs;
    uint32_t        efl;
    uint16_t        ds;
    uint16_t        es;
    uint16_t        ax;
    uint16_t        isr;
    uint16_t        ret_offs;
    uint16_t        ret_seg;

} __attribute__((packed)) isr_rm_ctx_t;

/******************************************************************************/

typedef struct
{
    union
    {
        uint32_t eax;

        union 
        {
            uint16_t ax;

            struct 
            {
                uint8_t al;
                uint8_t ah;
            };
        };
    };

    union
    {
        uint32_t ecx;
        
        union 
        {
            uint16_t cx;

            struct 
            {
                uint8_t cl;
                uint8_t ch;
            };
        };
    };
    
    union
    {
        uint32_t edx;

        union 
        {
            uint16_t dx;

            struct 
            {
                uint8_t dl;
                uint8_t dh;
            };
        };
    };

    union
    {
        uint32_t ebx;

        union 
        {
            uint16_t bx;

            struct 
            {
                uint8_t bl;
                uint8_t bh;
            };
        };
    };

    union 
    {
        uint32_t esp;
        uint16_t sp;
    };

    union 
    {
        uint32_t ebp;
        uint16_t bp;
    };
    
    union 
    {
        uint32_t esi;
        uint16_t si;
    };
    
    union 
    {
        uint32_t edi;
        uint16_t di;
    };

    uint32_t    efl;
    uint16_t    es;
    uint16_t    ds;
    
    dap_t       dap;

} __attribute__((packed)) rmode_ctx_t;

/******************************************************************************/

extern void ldr_bios_call(int n, volatile rmode_ctx_t* rmode_ctx);

extern void ldr_jmp_to_rmode(uint32_t seg, uint32_t offs, volatile rmode_ctx_t* rmode_ctx);

/******************************************************************************/

#endif //_SHARED_H_ 

