
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <linux/fs.h>

#include "types.h"
#include "fat.h"

#define ROUND_UP(a, b) ((a) + (b) - 1) / (b);

/******************************************************************************/

static void
dump_hex(uint8_t* data, size_t size)
{
    for(int i = 0; i < size; i++)
    {
        printf("%02X ", data[i]);

        if((i + 1) % 16 == 0)
            printf("\n");
    }
}

static bool_t
is_volume_fat16(uint8_t* bs)
{
    fat16_bs_t* fat16_bs = (fat16_bs_t *)bs;

    /* get the total sectors in volume (including VBR) */
    uint32_t total_sectors = (fat16_bs->bpb.total_sectors_16 == 0) ? 
        fat16_bs->bpb.total_sectors_32 : fat16_bs->bpb.total_sectors_16;

    /* size of fat_direntry_t is 32 bytes */
    uint32_t root_dir_sectors = ROUND_UP(
        fat16_bs->bpb.root_entry_count * sizeof(fat_direntry_t), 
        fat16_bs->bpb.bytes_per_sector);

    uint32_t data_sectors = total_sectors - 
        (fat16_bs->bpb.reserved_sectors + 
        (fat16_bs->bpb.fat_count * fat16_bs->bpb.sectors_per_fat_16) + 
        root_dir_sectors);

    uint32_t total_clusters = data_sectors / 
        fat16_bs->bpb.sectors_per_cluster;

    printf(
        "Total sectors:            %i\n"
        "Root Directory sectors:   %i\n"
        "Data sectors:             %i\n"
        "Total clusters:           %i\n\n",
        total_sectors,
        root_dir_sectors,
        data_sectors,
        total_clusters);

    if( total_clusters > 4085 && total_clusters < 65525 && 
        fat16_bs->bpb.root_entry_count != 0 ) 
    {
        return true;
    } 

    return false;
}

static bool_t
is_volume_fat32(uint8_t* bs)
{
    fat32_bs_t* fat32_bs = (fat32_bs_t *)bs;

    /* get the total sectors in volume (including VBR) */
    uint32_t total_sectors = (fat32_bs->bpb.total_sectors_16 == 0) ? 
        fat32_bs->bpb.total_sectors_32 : fat32_bs->bpb.total_sectors_16;

    /* size of fat_direntry_t is 32 bytes */
    uint32_t root_dir_sectors = ROUND_UP(
        fat32_bs->bpb.root_entry_count * sizeof(fat_direntry_t), 
        fat32_bs->bpb.bytes_per_sector);

    uint32_t data_sectors = total_sectors - 
        (fat32_bs->bpb.reserved_sectors + 
        (fat32_bs->bpb.fat_count * fat32_bs->bpb.sectors_per_fat_16) + 
        root_dir_sectors);

    uint32_t total_clusters = data_sectors / 
        fat32_bs->bpb.sectors_per_cluster;

    printf(
        "*********************************************************\n"
        "* Total sectors                %08i (hex: %08X) *\n"
        "* Root Directory sectors       %08i (hex: %08X) *\n"
        "* Data sectors                 %08i (hex: %08X) *\n"
        "* Total clusters               %08i (hex: %08X) *\n"
        "*********************************************************\n\n",
        total_sectors, total_sectors,
        root_dir_sectors, root_dir_sectors,
        data_sectors, data_sectors,
        total_clusters, total_clusters );

    if(total_clusters > 65525 && fat32_bs->bpb.root_entry_count == 0) 
    {
        return true;
    } 

    return false;
}

static void
print_fat32_info(int fd, uint8_t* bs)
{
    fat32_bs_t* fat32_bs = (fat32_bs_t *)bs;

    printf(
        "*********************************************************\n"
        "* Bytes per sector             %08i (hex: %08X) *\n"
        "* Sectors per cluster          %08i (hex: %08X) *\n"
        "* Reserved sectors             %08i (hex: %08X) *\n"
        "* FAT count                    %08i (hex: %08X) *\n"
        "* Total sectors 32-bit         %08i (hex: %08X) *\n"
        "* Sectors per FAT              %08i (hex: %08X) *\n"
        "* Root cluster                 %08i (hex: %08X) *\n"
        "*********************************************************\n\n",
        fat32_bs->bpb.bytes_per_sector, fat32_bs->bpb.bytes_per_sector,
        fat32_bs->bpb.sectors_per_cluster, fat32_bs->bpb.sectors_per_cluster,
        fat32_bs->bpb.reserved_sectors, fat32_bs->bpb.reserved_sectors,
        fat32_bs->bpb.fat_count, fat32_bs->bpb.fat_count,
        fat32_bs->bpb.total_sectors_32, fat32_bs->bpb.total_sectors_32,
        fat32_bs->ebpb.sectors_per_fat_32, fat32_bs->ebpb.sectors_per_fat_32,
        fat32_bs->ebpb.root_cluster, fat32_bs->ebpb.root_cluster );

    uint32_t fat_region = 
        fat32_bs->bpb.reserved_sectors * 
        fat32_bs->bpb.bytes_per_sector;

    uint32_t data_region = fat_region + 
        fat32_bs->bpb.fat_count * fat32_bs->ebpb.sectors_per_fat_32 *
        fat32_bs->bpb.bytes_per_sector;

    uint32_t root_dir = ((fat32_bs->ebpb.root_cluster - 2) * 
        fat32_bs->bpb.sectors_per_cluster * fat32_bs->bpb.bytes_per_sector) +
        data_region;

    printf( 
        "*********************************************************\n"
        "* FAT region                   %08i (hex: %08X) *\n"
        "* Data region                  %08i (hex: %08X) *\n"
        "* Root directory               %08i (hex: %08X) *\n"
        "*********************************************************\n\n",
        fat_region, fat_region,
        data_region, data_region,
        root_dir, root_dir );

    /* 
        FAT32 FAT entries are 32-bit, but upper 4-bits are reserved,
        so use 0x0FFFFFFF mask

        EOC = End Of Clusterchain = 0x0FFFFFF8
        Bad cluster = 0x0FFFFFF7
        Free clusters contain 0 in their FAT cluster entry
    */

    /* dump the root directory as a test, by emulating sector reads */
    uint8_t* tmp_sector = (uint8_t *)malloc(
        fat32_bs->bpb.bytes_per_sector);

    uint8_t* tmp_cluster = (uint8_t *)malloc(
        fat32_bs->bpb.bytes_per_sector *
        fat32_bs->bpb.sectors_per_cluster);

    if(tmp_cluster != NULL && tmp_sector != NULL)
    {
        uint32_t fat_val = fat32_bs->ebpb.root_cluster;

        while(true)
        {
            /* base sector of cluster(fat_val) */
            uint32_t base_sector = ((fat_val - 2) * 
                fat32_bs->bpb.sectors_per_cluster) +
                fat32_bs->bpb.reserved_sectors + 
                fat32_bs->bpb.fat_count * fat32_bs->ebpb.sectors_per_fat_32;

            printf("base sector: %08X\n", base_sector);

            /* read the cluster in memory by sector */
            for(int i = 0; i < fat32_bs->bpb.sectors_per_cluster; i++)
            {
                printf("reading sector: %08X\n", (base_sector + i));

                lseek(fd, (base_sector + i) * fat32_bs->bpb.bytes_per_sector, 
                    SEEK_SET);

                read(fd, tmp_sector, fat32_bs->bpb.bytes_per_sector);

                memcpy(
                    tmp_cluster + i * fat32_bs->bpb.bytes_per_sector, 
                    tmp_sector, fat32_bs->bpb.bytes_per_sector);
            }
            
#if _DEBUG
            dump_hex(tmp_cluster, 
                fat32_bs->bpb.bytes_per_sector *
                fat32_bs->bpb.sectors_per_cluster);
#endif

            /* loop through the cluster root dir */
            for(int i = 0; 
                i < fat32_bs->bpb.bytes_per_sector *
                    fat32_bs->bpb.sectors_per_cluster; 
                i += sizeof(fat_direntry_t))
            {
                fat_direntry_t* entry = (fat_direntry_t *)&tmp_cluster[i];

                if(entry->name[0] == 0x00)
                    break;

                if(entry->name[0] == 0xE5)
                    continue;
                
                if(entry->attrib == 0x0F)
                {
                    // long file name
                }
                else
                {
                    char name[12];
                    memcpy(&name[0], entry->name, 8);
                    memcpy(&name[8], entry->ext, 3);
                    name[11] = 0;

                    printf("file: %s @ cluster: %i\n", name, 
                        entry->starting_cluster);
                }
            }

            /* end of clusterchain? */
            uint32_t fat_offs = fat_val * sizeof(uint32_t);

            uint32_t fat_sect = (fat32_bs->bpb.reserved_sectors + 
                (fat_offs / fat32_bs->bpb.bytes_per_sector));
            
            uint32_t fat_rel_offs = fat_offs % fat32_bs->bpb.bytes_per_sector;

            lseek(fd, fat_sect * fat32_bs->bpb.bytes_per_sector, SEEK_SET);
            read(fd, tmp_sector, fat32_bs->bpb.bytes_per_sector);

            uint32_t new_fat_val = 
                (*((uint32_t *)&tmp_sector[fat_rel_offs])) & 0x0FFFFFFF;

            /* EOC */
            if(new_fat_val == 0x0FFFFFF8)
                break;

            fat_val = new_fat_val;
        }
    }
}

int 
main(int argc, char* argv[])
{
    struct stat st;

    uint8_t cur_bs[512];
    uint8_t new_bs[512];

    if(argc < 3 || strstr(argv[1], "/dev/sda") != NULL)
    {
        printf("RTFM: fixfs <target> <source>\n");
        return 0;
    }

    printf(
        "*****************************************************\n"
        "*                                                   *\n"
        "*                  x86_vbrkit tool                  *\n"
        "*                                                   *\n"
        "*****************************************************\n\n");

    int fd;
    int new_fd;
    int sector_size = 0;

    /* open raw device */
    if((fd = open(argv[1], O_RDWR)) == -1)
    {
        printf("[!] Failed to open %s\n", argv[1]);
        return 0;
    }

#if 0

    // BLKSSZGET: 		logical block size
    // BLKBSZGET: 		physical block size
    // BLKGETSIZE64: 	device size in bytes
    // BLKGETSIZE: 		device size / 512

    if(ioctl(fd, BLKSSZGET, &sector_size) >= 0)
    {
        printf("logical sector size: %i\n", sector_size);
    }

    if(ioctl(fd, BLKBSZGET, &sector_size) >= 0)
    {
        printf("physical sector size: %i\n\n", sector_size);
    }

#endif

    /* read the boot sector */
    lseek(fd, 0, SEEK_SET);
    
    if(read(fd, cur_bs, sizeof(cur_bs)) > 0)
    {
        if(is_volume_fat32(cur_bs))
        {
            print_fat32_info(fd, cur_bs);

            /* modify the bootsector */
            ssize_t bytes_read;

            /* open the new fat16 bootsector */
            if((new_fd = open(argv[2], O_RDONLY)) != -1)
            {
                fstat(new_fd, &st);

                if((bytes_read = read(new_fd, new_bs, sizeof(new_bs))) > 0)
                {
                    /* could check the jmp and then write the new bootcode... */
                    memcpy(
                        ((fat32_bs_t *)cur_bs)->bootcode, 
                        ((fat32_bs_t *)new_bs)->bootcode, 448);

                    /* or MSDOS5.0 */
                    memcpy(((fat32_bs_t *)cur_bs)->oem_name, "VBRKIT  ", 8);

                    /* bootsector signature (bootable) */
                    cur_bs[510] = 0x55;
                    cur_bs[511] = 0xAA;

                    /* write it to file */
                    lseek(fd, 0, SEEK_SET);
                    write(fd, cur_bs, sizeof(cur_bs));

                    printf("Boot sector successfully updated!\n");
                }
                
                close(new_fd);
            }
            else
            {
                printf("[!] Failed to open %s\n", argv[2]);
            }
        }
        else
        {
            printf("[!] Volume type is not FAT32\n");
        }
    }

    printf("\n");

    close(fd);

    return 0;
}

/***************************************************************************/




