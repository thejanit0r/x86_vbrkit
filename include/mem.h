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
#ifndef _MEMORY_H_
#define _MEMORY_H_

#include <types.h>

/******************************************************************************/

/*
    MEMORY LAYOUT

    1000h to 7C00h: bootkit code
    7C00h to 7E00h: mbr/vbr
    10000h to 100000h: misc memory (compat. with real mode addressing)
*/

/******************************************************************************/

#define MEM_FREE 	0
#define MEM_USED 	1

typedef struct 
{
    uint8_t 	status;
    size_t 		size;

} malloc_blk_t;

/******************************************************************************/

void mem_init(void);

bool_t mem_is_initialized(void);

void* malloc(size_t size);

void free(void* mem);

#endif //_MEMORY_H_ 
