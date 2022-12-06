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
#ifndef _BIOS_H_
#define _BIOS_H_

#define BIOS_SVC_VIDEO                              0x10
#define BIOS_SVC_VIDEO_SET_CURSOR_POS               0x02
#define BIOS_SVC_VIDEO_GET_CURSOR_POS               0x03
#define BIOS_SVC_VIDEO_PUTCHAR                      0x0E

#define BIOS_SVC_DISK                               0x13
#define BIOS_SVC_DISK_INSTALL_CHECK                 0x41
#define BIOS_SVC_DISK_EXTENDED_READ                 0x42
#define BIOS_SVC_DISK_EXTENDED_WRITE                0x43
#define BIOS_SVC_DISK_EXTENDED_GET_PARAMS           0x48
#define BIOS_SVC_DISK_READ                          0x02
#define BIOS_SVC_DISK_WRITE                         0x03
#define BIOS_SVC_DISK_GET_PARAMS                    0x08
#define BIOS_SVC_DISK_GET_TYPE                      0x15

#define BIOS_SVC_SERIAL                             0x14
#define BIOS_SVC_SERIAL_INIT_PORT                   0x00
#define BIOS_SVC_SERIAL_PUTCHAR                     0x01
#define BIOS_SVC_SERIAL_GETCHAR                     0x02
#define BIOS_SVC_SERIAL_GET_PORT_STATUS             0x03

#define BIOS_SVC_SYSTEM                             0x15
#define BIOS_SVC_SYSTEM_QUERY_MEM_MAP               0xE820 /* AX */
#define BIOS_SVC_SYSTEM_GET_PARAMS                  0xC0
#define BIOS_SVC_SYSTEM_EXTENDED_GET_BDA            0xC1

#define BIOS_SVC_KEYBOARD                           0x16
#define BIOS_SVC_KEYBOARD_GETCHAR                   0x00

#endif //_BIOS_H_ 
 
