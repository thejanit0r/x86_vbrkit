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
#ifndef _FAT_H_
#define _FAT_H_

#include <stdint.h>

typedef struct
{
    uint16_t    bytes_per_sector; /* bytes per sector (512, 1024, 2048, 4096) */
    uint8_t     sectors_per_cluster; /* sectors per cluster (1, 2, 4, 8, 16, 32, 64, 128) */
    uint16_t    reserved_sectors; /* number of reserved sectors */
    uint8_t     fat_count; /* number of FATs (usually 2) */
    uint16_t    root_entry_count; /* number of root directory entries */
    uint16_t    total_sectors_16; /* total number of sectors */
    uint8_t     media_type; /* media descriptor */
    uint16_t    sectors_per_fat_16; /* number of sectors per FAT */
    uint16_t    sectors_per_track; /* sectors per track */
    uint16_t    head_side_count; /* number of heads */
    uint32_t    hidden_sector_count; /* # of hidden sectors */
    uint32_t    total_sectors_32; /* # of sectors if total_sectors_16 == 0 */
 
} __attribute__((packed)) fat_bpb_t;

typedef struct
{
    uint8_t     bios_drive_num; /* int 0x13 drive number (e.g. 0x80) */
    uint8_t     nt_flags;   /* flags for Windows NT (reserved) */
    uint8_t     nt_signature; /* WinNT: 0x29 if next 3 fields are present */
    uint8_t     volume_id[4]; /* Volume serial number */
    uint8_t     volume_label[11]; /* Volume label */
    uint8_t     fat_type_label[8]; /* "FAT12   ", "FAT16   ", "FAT     " */
 
} __attribute__((packed)) fat16_ebpb_t;

typedef struct
{
    uint32_t    sectors_per_fat_32;
    uint16_t    ext_flags;
    uint16_t    fs_version;
    uint32_t    root_cluster;
    uint16_t    fs_info;
    uint16_t    backup;
    uint8_t     reserved[12];
    
    uint8_t     bios_drive_num; /* int 0x13 drive number (e.g. 0x80) */
    uint8_t     nt_flags;   /* flags for Windows NT (reserved) */
    uint8_t     nt_signature; /* WinNT: 0x29 if next 3 fields are present */
    uint8_t     volume_id[4]; /* Volume serial number */
    uint8_t     volume_label[11]; /* Volume label */
    uint8_t     fat_type_label[8]; /* "FAT12   ", "FAT16   ", "FAT     " */
 
} __attribute__((packed)) fat32_ebpb_t;

typedef struct
{
    uint8_t         bootjmp[3]; /* jump to bootstrap (eb xx 90, or e9 xx xx) */
    uint8_t         oem_name[8];

    /* BIOS Parameter Block (BPB) and Extended BIOS Parameter Block (EBPB) */
    fat_bpb_t       bpb;
    fat16_ebpb_t    ebpb;

    uint8_t         bootcode[448];
    uint16_t        signature; /* AA55h */

} __attribute__((packed)) fat16_bs_t;

typedef struct
{
    uint8_t         bootjmp[3]; /* jump to bootstrap (eb xx 90, or e9 xx xx) */
    uint8_t         oem_name[8];

    /* BIOS Parameter Block (BPB) and Extended BIOS Parameter Block (EBPB) */
    fat_bpb_t       bpb;
    fat32_ebpb_t    ebpb;

    uint8_t         bootcode[448];
    uint16_t        signature; /* AA55h */

} __attribute__((packed)) fat32_bs_t;

typedef struct
{
    uint8_t     name[8]; /* file name */
    uint8_t     ext[3]; /* file extension */
    uint8_t     attrib; /* attribute (hidden, dir, read only, ...) */
    uint8_t     reserved_0;
    uint8_t     creation_ms;
    uint16_t    creation_time;
    uint16_t    creation_date;
    uint16_t    last_access_date;
    uint16_t    reserved_1;
    uint16_t    last_write_time;
    uint16_t    last_write_date;
    uint16_t    starting_cluster;
    uint32_t    file_size;

} __attribute__((packed)) fat_direntry_t;

#endif //_FAT16_H_ 
