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
#ifndef _DISK_H_
#define _DISK_H_

#include <types.h>

/******************************************************************************/

#define WRITE 	                0x00
#define READ	                  0x01

/* hard disks */
#define DRIVE_HDD_START         0x80
#define DRIVE_HDD_END           0x8F

/* CD/DVD */
#define DRIVE_CD_DVD            0xE0

/* floppy */
#define DRIVE_FLOPPY_START      0x00
#define DRIVE_FLOPPY_END        0x7E

/******************************************************************************/

void disk_init(int drive);

bool_t disk_is_valid(int drive, bool_t* is_bootable);

int disk_get_bootable_drives(int* except, int except_count, int* out, int out_max_count);

int disk_get_booting_drive(void);

void disk_set_booting_drive(int drive);

bool_t disk_io(int mode, int drive_num, uint64_t sector_start, 
  int sectors_to_transfer, uint8_t* dst_buffer);

#endif //_DISK_H_ 

