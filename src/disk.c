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
#include <disk.h>
#include <mem.h>
#include <bios.h>

/******************************************************************************/

static int boot_drive;

/******************************************************************************/

void
disk_init(int drive)
{
    disk_set_booting_drive(drive);
}

int
disk_get_booting_drive(void)
{
    return boot_drive;
}

void 
disk_set_booting_drive(int drive)
{
    boot_drive = drive;
}

bool_t
disk_is_valid(int drive, bool_t* is_bootable)
{
    bool_t is_valid = false; 
    uint8_t* buffer = (uint8_t *)malloc(512);

    if(is_bootable != NULL)
    {
        *is_bootable = false;
    }
    
    if(buffer != NULL)
    {
        if(disk_io(READ, drive, 0, 1, buffer))
        {
            is_valid = true;

            /* check if the drive is bootable */
            if(*(uint16_t *)&buffer[510] == 0xAA55)
            {
                if(is_bootable != NULL)
                {
                    *is_bootable = true;
                }
            }
        }

        free(buffer);
    }

    return is_valid;
}

int
disk_get_bootable_drives(int* except, int except_count, int* out, int out_max_count)
{
    int out_count = 0;

    if(out == NULL || out_max_count <= 0)
    {
        return 0;
    }

    for(int i = DRIVE_HDD_START; i <= DRIVE_HDD_END; i++)
    {
        bool_t is_bootable;
        bool_t do_add = true;

        if(disk_is_valid(i, &is_bootable) && is_bootable)
        {
            if(except_count > 0 && except != NULL)
            {
                for(int j = 0; j < except_count; j++)
                {
                    if(except[j] == i)
                    {
                        do_add = false;
                    }
                }
            }
            
            if(out_count < out_max_count)
            {
                if(do_add)
                {
                    out[out_count++] = i;
                }
            }
            else
            {
                break;
            }
        }
    }

    return out_count;
}

bool_t
disk_io(int mode, int drive_num, uint64_t sector_start, int sectors_to_transfer, 
    uint8_t* dst_buffer)
{
    rmode_ctx_t ctx;

    /* setup disk address packet */
    ctx.dap.packet_size = sizeof(dap_t);
    ctx.dap.reserved = 0;
    ctx.dap.sector_start = sector_start;
    ctx.dap.sectors_to_transfer = sectors_to_transfer;
    ctx.dap.dst_seg = (((uint32_t)dst_buffer >> 4) & 0xF000);
    ctx.dap.dst_offs = (((uint32_t)dst_buffer >> 0) & 0xFFFF);

    /* 
        setup the bios call 
        note: using LBA mode (CHS won't be supported!)
    */
    ctx.dl = drive_num;
    
    ctx.ah = (mode == READ ? 
        BIOS_SVC_DISK_EXTENDED_READ : 
        BIOS_SVC_DISK_EXTENDED_WRITE);

    ctx.al = (mode == READ ? 0x00 : 0x00); /* write without verify */
    ctx.esi = ((uint32_t)(&ctx.dap)) & 0xFFFF;

    ldr_bios_call(BIOS_SVC_DISK, &ctx);

    return (!(ctx.efl & 1) /* && ctx.ah == 0 */);
}
