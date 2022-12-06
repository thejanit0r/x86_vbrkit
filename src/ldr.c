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
#include <ldr.h>
#include <shared.h>
#include <libc.h>
#include <console.h>
#include <disk.h>
#include <mem.h>
#include <video.h>
#include <types.h>
#include <pff.h>
#include <pe.h>
#include <serial.h>
#include <bios.h>

/******************************************************************************/

static void
print_drives(void)
{
    printf(FG_LCYAN, "****************\n");
    printf(FG_LCYAN, "* DRIVES       *\n");
    printf(FG_LCYAN, "****************\n");

    for(int i = DRIVE_HDD_START; i <= DRIVE_HDD_END; i++)
    {
        bool_t is_bootable;

        if(disk_is_valid(i, &is_bootable))
        {
            printf(FG_LGREEN, "   Drive %xh found!", i);

            if(is_bootable)
            {
                printf(FG_LMAGENTA, " (bootable)");
            }

            printf(FG_LMAGENTA, "\n");
        }
    }

    printf(0, "\n");
}

static void
print_serial_ports(void)
{
    int ports[] = {
        SERIAL_PORT1,
        SERIAL_PORT2,
        SERIAL_PORT3,
        SERIAL_PORT4
    };

    printf(FG_LCYAN, "****************\n");
    printf(FG_LCYAN, "* SERIAL PORTS *\n");
    printf(FG_LCYAN, "****************\n");
    
    for(int i = 0; i < countof(ports); i++)
    {
        int port = ports[i];

        /* try to initialize the port */
        serial_port_init(port);

        if(serial_port_initialized(port))
        {
            printf(FG_GREEN, "> Port 0x%x initialized/available\n", port);
        }
        else
        {
            printf(FG_RED, "> Port 0x%x not initialized/available\n", port);
        }
    }

    printf(0, "\n");
}

static void
print_mmap(void)
{
    rmode_ctx_t ctx;

    mmap_addr_desc_t mmap_addr_desc;

    unsigned int col;
    unsigned int row;

    printf(FG_LCYAN, "****************\n");
    printf(FG_LCYAN, "* MEMORY MAP   *\n");
    printf(FG_LCYAN, "****************\n");

    /* start address */
    ctx.ebx = 0;

    while(true)
    {
        ctx.eax = BIOS_SVC_SYSTEM_QUERY_MEM_MAP;
        ctx.edx = 0x534D4150; /* 'SMAP' */
        ctx.ecx = sizeof(mmap_addr_desc_t);
        
        ctx.es = (((uint32_t)&mmap_addr_desc >> 4) & 0xF000);
        ctx.di = (((uint32_t)&mmap_addr_desc >> 0) & 0xFFFF);

        ldr_bios_call(BIOS_SVC_SYSTEM, &ctx);

        /* exit on fail */
        if((ctx.efl & 1) || ctx.eax != 0x534D4150)
        {
            break;
        }

        printf(FG_LMAGENTA, "Base: %x`%x | ", 
            (uint32_t)(mmap_addr_desc.base_address >> 32),
            (uint32_t)(mmap_addr_desc.base_address >> 0));

        printf(FG_LMAGENTA, "Size: %x`%x | ",
            (uint32_t)(mmap_addr_desc.size >> 32),
            (uint32_t)(mmap_addr_desc.size >> 0));

        printf(FG_LGREEN, "Type: ");

        switch(mmap_addr_desc.type)
        {
            case 0x01: 	printf(FG_LMAGENTA, "FREE RAM"); break;
            case 0x02: 	printf(FG_LMAGENTA, "RESERVED"); break;
            case 0x03: 	printf(FG_LMAGENTA, "ACPI RECLAIM MEMORY"); break;
            case 0x04: 	printf(FG_LMAGENTA, "ACPI NVS MEMORY"); break;
            default: 	printf(FG_LMAGENTA, "RESERVED"); break;
        }

        printf(FG_LGREEN, "\n");

        /* exit on completion */
        if(ctx.ebx == 0)
        {
            break;
        }
    }

    printf(0, "\n");
}

static void
print_pe_info(void* base, size_t size)
{
    struct debug_dir* dd;
    struct cv_pdb70* pdb_info;

    if(!pe_is_valid(base))
        return;

    printf(FG_LCYAN, "PE EXECUTABLE! ");

    if(pe_get_magic(base) == PE_OPT_MAGIC_PE32PLUS)
    {
        printf(FG_LCYAN, "[64] EntryPoint: %x, SizeOfCode: %x\n",
            pe64_get_optional_header(base)->entry_point,
            pe64_get_optional_header(base)->text_size);
    }
    else
    {
        printf(FG_LCYAN, "[32] EntryPoint: %x, SizeOfCode: %x\n",
            pe32_get_optional_header(base)->entry_point,
            pe32_get_optional_header(base)->text_size);
    }

#if 0

    dd = pe_get_debug_dir(base, NULL, true);

    /* check for IMAGE_DEBUG_TYPE_CODEVIEW */
    if(dd->type != 2)
        return;

    pdb_info = (struct cv_pdb70 *)((uint8_t *)base + dd->ptr_to_raw_data);

    if(pdb_info->magic != 'SDSR')
        return;

    printf(FG_WHITE, "    %s\n", pdb_info->filename);

#endif
}

static void
print_rsrc_info(void* base, size_t size)
{
    void* internal_name = pe_get_version_info_field(base, size, L"InternalName");
    void* original_name = pe_get_version_info_field(base, size, L"OriginalFilename");
    
    if(internal_name || original_name)
    {
        printf(FG_WHITE, "> ");
    }
    
    if(internal_name)
    {
        printf(FG_WHITE, "%S", internal_name);
    }

    if(original_name && 0)
    {
        if(internal_name)
        {
            printf(FG_WHITE, ", ");
        }

        printf(FG_WHITE, "%S", original_name);
    }

    if(internal_name || original_name)
    {

        printf(FG_WHITE, ": %S\n", pe_get_version_info_field(base, size, L"FileDescription"));
    }
}

/******************************************************************************/

void
ldr_isr_rm_callback(void* p)
{
    isr_rm_ctx_t* isr_ctx = (isr_rm_ctx_t *)((uint8_t *)p + 4 /* sp, ss */); 

#if _DEBUG

    /* log the interrupt service routine */
    printf(0, "[INT %xH]: AX=%x (from %x:%x)\n", 
        isr_ctx->isr, isr_ctx->ax, isr_ctx->ret_seg, isr_ctx->ret_offs);
    
#endif

    /*
        Called services by Microsoft Windows

        - INT 13H / AH=02H: Read Sectors
        - INT 13H / AH=08H: Get Drive Parameters
        - INT 13H / AH=15H: Get Drive Type
        - INT 13H / AH=41H: Extended Disk Drive (EDD) Installation Check
        - INT 13H / AH=42H: Extended Read Sectors
        - INT 13H / AH=43H: Extended Write Sectors
        - INT 13H / AH=48H: Extended Get Drive Parameters
        
        - INT 15H / AX=E820H: Query System Address Map
        - INT 15H / AH=C0H: Get System Parameters
        - INT 15H / AH=C1H: Get Extended BIOS Data Area Segment
    */

    /* 13H: disk services */
    if(isr_ctx->isr == BIOS_SVC_DISK)
    {
        /* disk address packet, for AH=42h */
        dap_t* dap = (dap_t *)(((uint32_t)(isr_ctx->ds) << 4) + 
            (isr_ctx->saved_regs.esi & 0xFFFF));

        /* data buffer */
        uint8_t* buffer;
        uint16_t sectors;

        if((((isr_ctx->ax) >> 8) & 0xFF) == BIOS_SVC_DISK_READ && !(isr_ctx->efl & 1))
        {
            buffer = (uint8_t *)(((uint32_t)(isr_ctx->es) << 4) + 
                (isr_ctx->saved_regs.ebx & 0xFFFF));

            sectors = isr_ctx->saved_regs.eax & 0xFF;

            /* ... */
        }
        else
        if((((isr_ctx->ax) >> 8) & 0xFF) == BIOS_SVC_DISK_EXTENDED_READ && !(isr_ctx->efl & 1))
        {
            buffer = (uint8_t *)(((uint32_t)dap->dst_seg << 4) + dap->dst_offs);

            /* assuming sector size is 512 bytes */
            uint32_t size = dap->sectors_to_transfer * 512;

#if _DEBUG

            /* debug */
            printf(FG_LRED, "Reading sector(s) to: %x (%x)\n", buffer, size);

#endif

            print_pe_info(buffer, size);
            print_rsrc_info(buffer, size);

            /* Windows 10 - 21H2 - 10.0.19041.1288 */
            if(findpattern(buffer, size, "E9 D5 01 EB") != NULL)
            {
                printf(FG_LRED, "Bootmgr.exe detected! Disk read called from %x:%x\n\n",
                    isr_ctx->ret_seg, isr_ctx->ret_offs);
            }
            
            /*
                Patch, hook and deploy the next stage(s)...

                E.g. PatchGuard/KPP,  Driver Signature Enforcement (DSE), etc.
            */
        }
        else
        {
            /* nothing */
        }
    }
    /* 15H: system services */
    else if(isr_ctx->isr == BIOS_SVC_SYSTEM)
    {
        /* get system memory map */
        if(isr_ctx->ax == BIOS_SVC_SYSTEM_QUERY_MEM_MAP)
        {
            /* spoof/manipulate returned data; e.g. reserve/hide memory for the vbrkit */
        }
    }
}

static void
print_root_directory()
{
    FATFS fs;
    DIR dir;

    if(pf_mount(&fs) != FR_OK)
    {
        printf(FG_LRED, "Failed to mount the filesystem!\n");
        return;
    }
    
    /* scan the root directory of the FAT32 volume */
    if(pf_opendir(&dir, "/") == FR_OK)
    {
        FILINFO file_info;
        char 	file_name[16];

        while(pf_readdir(&dir, &file_info) == FR_OK && file_info.fname[0] != 0)
        {
            memset(file_name, 0, sizeof(file_name));
            memcpy(file_name, file_info.fname, sizeof(file_info.fname));

            /* print the file info */
            printf(FG_WHITE, "/%s (%d bytes) (%s)\n", file_name, file_info.fsize,
                ((file_info.fattrib & AM_DIR) ? "folder" : "file"));
        }
    }
    else
    {
        printf(FG_LRED, "Failed to open the filesystem!\n");
    }
}

/******************************************************************************/

void
ldr_main(int boot_drive)
{
    /* default debug serial port */
    serial_port_init(SERIAL_PORT1);

    /* heap / memory allocation */
    mem_init();

    /* current disk */
    disk_init(boot_drive);

    /* console video or serial */
    console_init(SERIAL_PORT1);

    printf(FG_LRED, "*******************************\n");
    printf(FG_LRED, "*         x86_vbrkit          *\n");
    printf(FG_LRED, "*******************************\n\n");

    printf(FG_WHITE, "> Booting from drive %xh\n", boot_drive);

#ifdef _DEBUG

    print_drives();
    print_serial_ports();
    print_mmap();

#endif

    /* find another bootable drive to boot from */
    int drive_to_boot;

    if(disk_get_bootable_drives(&boot_drive, 1, &drive_to_boot, 1) < 1)
    {
        printf(FG_RED, "Failed to find a bootable drive!");

        while(1);
    }

    printf(FG_WHITE, "> Next drive to boot: %xh\n", drive_to_boot);

    getch();

    /* load the first sector and jump to it */
    if(disk_io(READ, drive_to_boot, 0, 1, (uint8_t *)0x7C00))
    {
        rmode_ctx_t ctx;

        ctx.eax = 0;
        ctx.ecx = 0;
        ctx.ebx = 0;
        ctx.ebp = 0;
        ctx.esi = 0;
        ctx.edi = 0;
        ctx.edx = drive_to_boot;

        ldr_jmp_to_rmode(0, 0x7C00, &ctx);
    }

    while(1);
}

/******************************************************************************/




