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
#include <pe.h>

/******************************************************************************/

struct section_header* 
pe_get_section_header(void* base, ulong_t i, ulong_t* num_of_sec)
{
    uint8_t* tmp = (uint8_t *)base;

    struct section_header* sec_hdr;
    struct mz_hdr* dos = (struct mz_hdr *)tmp;
    struct pe_hdr* pe = (struct pe_hdr *)(tmp + dos->peaddr);

    /* get the number of sections and sanity check */
    if(i >= pe->sections)
    {
        return NULL;
    }

    if(num_of_sec != NULL)
    {
        *num_of_sec = pe->sections;
    }

    /* get the first section header */
    sec_hdr = (struct section_header *)((uint8_t *)pe + 
            sizeof(struct pe_hdr) + pe->opt_hdr_size);

    return (sec_hdr + i);
}

bool_t 
pe_is_valid(void* base)
{
    ulong_t magic = pe_get_magic(base);

    if( magic != PE_OPT_MAGIC_PE32 &&
        magic != PE_OPT_MAGIC_PE32PLUS )
    {
        return false;
    }

    return true;
}

ulong_t 
pe_get_magic(void* base)
{
    uint8_t* tmp = (uint8_t *)base;

    struct pe_hdr* pe;
    struct mz_hdr* dos = (struct mz_hdr *)tmp;

    if(dos->magic != MZ_MAGIC)
        return 0;

    pe = (struct pe_hdr *)(tmp + dos->peaddr);

    if(pe->magic != PE_MAGIC)
        return 0;

    return (*(uint16_t *)((uint8_t *)pe + sizeof(struct pe_hdr)));
}

struct pe32_opt_hdr*
pe32_get_optional_header(void* base)
{
    uint8_t* tmp = (uint8_t *)base;

    return (struct pe32_opt_hdr *)(tmp + 
        ((struct mz_hdr *)tmp)->peaddr + sizeof(struct pe_hdr));
}

struct pe32plus_opt_hdr*
pe64_get_optional_header(void* base)
{
    uint8_t* tmp = (uint8_t *)base;

    return (struct pe32plus_opt_hdr *)(tmp + 
        ((struct mz_hdr *)tmp)->peaddr + sizeof(struct pe_hdr));
}

uint32_t 
pe_rva_to_raw(void* base, ulong_t rva)
{
    ulong_t num_of_sec;

    struct section_header* sec_hdr;

    /* get the first section header */
    if((sec_hdr = pe_get_section_header(base, 0, &num_of_sec)) == NULL)
    {
        return 0;
    }

    /* calculate the raw address */
    for(ulong_t i = 0; i < num_of_sec; i++)
    {
        if( (sec_hdr->virtual_address <= rva) &&
            (sec_hdr->virtual_address + sec_hdr->virtual_size) > rva )
        {
            rva -= sec_hdr->virtual_address;
            rva += sec_hdr->data_addr;

            return rva;
        }

        sec_hdr++;
    }

    return 0;
}

uint32_t 
pe_raw_to_rva(void* base, ulong_t raw)
{
    ulong_t num_of_sec;

    struct section_header* sec_hdr;

    /* get the first section header */
    if((sec_hdr = pe_get_section_header(base, 0, &num_of_sec)) == NULL)
    {
        return 0;
    }

    /* calculate the raw address */
    for(ulong_t i = 0; i < num_of_sec; i++)
    {
        if( (sec_hdr->data_addr <= raw) &&
            (sec_hdr->data_addr + sec_hdr->raw_data_size) > raw )
        {
            raw -= sec_hdr->data_addr;
            raw += sec_hdr->virtual_address;

            return raw;
        }

        sec_hdr++;
    }

    return 0;
}

struct debug_dir*
pe_get_debug_dir(void* base, ulong_t* size, bool_t file_aligned)
{
    uint32_t offs;

    /* get the debug directory */
    if(pe_get_magic(base) == PE_OPT_MAGIC_PE32PLUS)
    {
        offs = pe64_get_optional_header(base)->data_directory.debug.virtual_address;

        if(size != NULL)
        {
            *size = pe64_get_optional_header(base)->data_directory.debug.size;
        }
    }
    else
    {
        offs = pe32_get_optional_header(base)->data_directory.debug.virtual_address;

        if(size != NULL)
        {
            *size = pe32_get_optional_header(base)->data_directory.debug.size;
        }
    }	

    /* convert to file aligned offset */
    if(file_aligned)
    {
        offs = pe_rva_to_raw(base, offs);
    }

    return (struct debug_dir *)((uint8_t *)base + offs);
}

void*
pe_get_version_info_field(void* base, size_t size, wchar_t* field_name)
{
    uint8_t* field_value = NULL;
    uint8_t* verinfo = findstr_w(base, size, L"VS_VERSION_INFO");

    if(verinfo == NULL)
    {
        return NULL;
    }

    field_value = findstr_w(verinfo, size - (verinfo - (uint8_t *)base), field_name);

    if(field_value != NULL)
    {
        field_value += (2 * wcslen(field_name));

        while(*field_value == 0) 
        {
            field_value++;
        }
    }

    return field_value;
}