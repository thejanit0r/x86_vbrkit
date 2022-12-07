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
#include <mem.h>
#include <libc.h>
#include <console.h>
#include <types.h>
#include <stdarg.h>

/******************************************************************************/

static uint8_t*     mem_base = NULL;
static size_t       mem_size = 0;
static size_t       mem_allocated;
static uint32_t     mem_newly_freed_count;

/******************************************************************************/

void
mem_init(void)
{
    mem_base = (uint8_t *)0x10000;
    mem_size = 0x80000;

    mem_allocated = 0;
    mem_newly_freed_count = 0;

    memset(mem_base, 0, mem_size);
}

bool_t
mem_is_initialized(void)
{
    return (mem_size != 0 && mem_base != NULL);
}

void
malloc_merge_free_blks(void)
{
    uint8_t* mem = mem_base;

    /* some kind of optimization, merge only on threshold reached */
    if(mem_newly_freed_count > 1)
    {
        return;
    }

    while(mem < mem_base + mem_size)
    {
        malloc_blk_t* blk = (malloc_blk_t *)mem;

        if(blk->size == 0 && blk->status == MEM_FREE)
        {
            /* reached the end of the chain */
            break;
        }
        else 
        if(blk->size != 0 && blk->status == MEM_FREE)
        {
            /* 
                found a free block, check if the next block is 
                also free and merge it
            */
            malloc_blk_t* tmp = (malloc_blk_t *)
                    (mem + blk->size + sizeof(malloc_blk_t));

            if(tmp->size != 0 && tmp->status == MEM_FREE)
            {
#ifdef _DEBUG

                printf(FG_LRED, "Merging free blocks\n");

#endif

                /* block is also free, so merge it */
                blk->size += (tmp->size + sizeof(malloc_blk_t));
                continue;
            }
        }

        mem += (blk->size + sizeof(malloc_blk_t));
    }

    mem_newly_freed_count = 0;
}

void*
malloc(size_t size)
{
    if(!mem_is_initialized())
    {
        mem_init();
    }

    uint8_t* mem = mem_base;

    /* nothing to allocate */
    if(size == 0)
    {
        return NULL;
    }

    /* find a block */
    while(mem + size + sizeof(malloc_blk_t) < mem_base + mem_size)
    {
        malloc_blk_t* blk = (malloc_blk_t *)mem;

        if(blk->size == 0 && blk->status == MEM_FREE)
        {
            /* reached the end of the chain */
            blk->status = MEM_USED;
            blk->size = size;

#ifdef _DEBUG

            printf(FG_LRED, "Allocating to end of chain\n");

#endif

            goto malloc_blk_allocated;
        }
        else 
        if(blk->size >= size && blk->status == MEM_FREE)
        {
            /* the block is free and big enough */

            /* 
                if it is bigger than the threshold, mark the rest of the 
                available space as a free "new" block 
            */
            if((blk->size - size) >= 8 * sizeof(malloc_blk_t))
            {
                malloc_blk_t* tmp = (malloc_blk_t *)
                    (mem + size + sizeof(malloc_blk_t));

                tmp->status = MEM_FREE;
                tmp->size = blk->size - size - sizeof(malloc_blk_t);

                blk->status = MEM_USED;
                blk->size = size;

#ifdef _DEBUG

                printf(FG_LRED, "Allocating and splitting existing block\n");

#endif
            }
            else
            {
                blk->status = MEM_USED;

#ifdef _DEBUG

                printf(FG_LRED, "Allocating to existing block\n");

#endif
            }

            goto malloc_blk_allocated;
        }
        
        /* 
            if the current block is being used or free but 
            not big enough, skip it 
        */
        mem += (blk->size + sizeof(malloc_blk_t));
    }

    return NULL;

malloc_blk_allocated:

    mem_allocated += ((malloc_blk_t *)mem)->size + sizeof(malloc_blk_t);

    /* zero out the allocated memory */
    memset(mem + sizeof(malloc_blk_t), 0, ((malloc_blk_t *)mem)->size);

#ifdef _DEBUG

    printf(FG_LRED, "Allocated block at %p (%x)\n", mem, ((malloc_blk_t *)mem)->size);

#endif

    return (void *)(mem + sizeof(malloc_blk_t));
}

void
free(void* mem)
{
    if(!mem_is_initialized())
    {
        return;
    }

    malloc_blk_t* blk = (malloc_blk_t *)(
        (uint8_t *)mem - sizeof(malloc_blk_t));

    blk->status = MEM_FREE;

    mem_allocated -= (blk->size + sizeof(malloc_blk_t));
    mem_newly_freed_count++;

#ifdef _DEBUG

    printf(FG_LRED, "Block freed at %p (%x)\n", blk, blk->size);

#endif

    /* merge free blocks */
    malloc_merge_free_blks();
}
