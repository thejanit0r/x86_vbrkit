;
;    This file is part of x86_vbrkit.
;
;    Copyright 2017 / the`janitor / < email: base64dec(dGhlLmphbml0b3JAcHJvdG9ubWFpbC5jb20=) >
;
;    Licensed under the Apache License, Version 2.0 (the "License");
;    you may not use this file except in compliance with the License.
;    You may obtain a copy of the License at
;
;        http://www.apache.org/licenses/LICENSE-2.0
;
;    Unless required by applicable law or agreed to in writing, software
;    distributed under the License is distributed on an "AS IS" BASIS,
;    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
;    See the License for the specific language governing permissions and
;    limitations under the License.
;
;
[BITS 16]
[ORG 7C00h]

%define LDR_SEGMENT             0000h
%define LDR_OFFSET              1000h
%define TMP_SECTOR              7E00h 
%define STACK_ADDR              7C00h

%define TARGET_FILENAME_0_3     "LDR "
%define TARGET_FILENAME_4_7     "    "
%define TARGET_FILENAME_7_11    " BIN"

; TARGET_FILENAME_0_3   : from byte 0 to 3   \
; TARGET_FILENAME_4_7   : from byte 4 to 7    | Reminder: overlap at byte 7!
; TARGET_FILENAME_7_11  : from byte 7 to 11  /

; TMP_SECTOR must account for at least one sector size (and seg=0)

; *****************************************************************************
; * General Information                                                       *
; *****************************************************************************

; This boot sector is intended for FAT32 (File Allocation Table)

; BIOS  : Basic Input/Output System
; VBR   : Volume Boot Record
; MBR   : Master Boot Record

; Non partitioned devices dont need any MBR/GPT

; Memory Addressing in real mode (16 bit): 
;   Physical Address = Segment * 10H + Offset

; 0x00000500 to 0x00007BFF: ~30 KB RAM (guaranteed free for use)
; 0x00007E00 to 0x0007FFFF: 480.5 KB RAM (guaranteed free for use)

; Interrupt Vector Table (IVT) @ 0000:0000H (size = 400H)

; IVT Offset | INT #     | Description
; -----------+-----------+-----------------------------------
; 0x0000     | 0x00      | Divide by 0
; 0x0004     | 0x01      | Reserved
; 0x0008     | 0x02      | NMI Interrupt
; 0x000C     | 0x03      | Breakpoint (INT3)
; 0x0010     | 0x04      | Overflow (INTO)
; 0x0014     | 0x05      | Bounds range exceeded (BOUND)
; 0x0018     | 0x06      | Invalid opcode (UD2)
; 0x001C     | 0x07      | Device not available (WAIT/FWAIT)
; 0x0020     | 0x08      | Double fault
; 0x0024     | 0x09      | Coprocessor segment overrun
; 0x0028     | 0x0A      | Invalid TSS
; 0x002C     | 0x0B      | Segment not present
; 0x0030     | 0x0C      | Stack-segment fault
; 0x0034     | 0x0D      | General protection fault
; 0x0038     | 0x0E      | Page fault
; 0x003C     | 0x0F      | Reserved
; 0x0040     | 0x10      | x87 FPU error
; 0x0044     | 0x11      | Alignment check
; 0x0048     | 0x12      | Machine check
; 0x004C     | 0x13      | SIMD Floating-Point Exception
; 0x00xx     | 0x14-0x1F | Reserved
; 0x0xxx     | 0x20-0xFF | User definable

; *****************************************************************************
; * FAT32 Structure                                                           *
; *****************************************************************************
; 0 - Reserved Region (BPB, ...)
; 1 - FAT Region 
; 2 - File and Directory Data Region 
; *****************************************************************************

; Cluster: group/array of sectors

; *****************************************************************************
; * Bootsector Entrypoint                                                     *
; *****************************************************************************

; DL = boot drive (floppies: 00h to 7Eh; hdd/removable: 80h to FEh)

bootsector:

    jmp short bootcode
    nop

oem_name:                       

    db 'MSWIN4.1' ; 'MSWIN4.1' is recommended, or 'MSDOS5.0'

; BIOS Parameter Block (BPB) and Extended BIOS Parameter Block (EBPB)
struc bpb_fat32
    .bytes_per_sector           resw 1 ; for max compat. should be 512
    .sectors_per_cluster        resb 1 
    .reserved_sectors           resw 1
    .fat_count                  resb 1 ; for max compat. should be 2
    .root_entry_count           resw 1
    .total_sectors_16           resw 1
    .media_type                 resb 1
    .sectors_per_fat_16         resw 1
    .sectors_per_track          resw 1
    .head_count                 resw 1
    .hidden_sector_count        resd 1 ; 0 if media not partitioned
    .total_sectors_32           resd 1 
    .sectors_per_fat_32         resd 1 
    .ext_flags                  resw 1
    .fs_version                 resw 1
    .root_cluster               resd 1 
    .fs_info                    resw 1
    .backup                     resw 1
    .reserved                   resb 12
    .bios_drive_num             resb 1
    .nt_flags                   resb 1
    .nt_signature               resb 1
    .volume_id                  resb 4
    .volume_label               resb 11
    .fat_type_label             resb 8
endstruc

; FAT32 Directory Entry
struc fat32_direntry
    .name                       resb 8
    .ext                        resb 3
    .attrib                     resb 1
    .reserved_0                 resb 1
    .creation_ms                resb 1
    .creation_time              resw 1
    .creation_date              resw 1
    .last_access_date           resw 1
    .reserved_1                 resw 1
    .last_write_time            resw 1
    .last_write_date            resw 1
    .starting_cluster           resw 1
    .file_size                  resd 1
endstruc

bpb:

    istruc bpb_fat32 
    iend

bootcode:

; Check that we have setup a correct FAT32 header
%if (bootcode - bootsector) != 5Ah
    %fatal "Bad FAT32 BPB/EBPB"
%endif

    ; Fix CS segment (some BIOSes set CS = 07C0H)
    ; Not really needed as long as only relative addressing is used...

    ; jmp 0000h:start

start:

    ; CLI/STI not really needed...
    xor     ax, ax
    mov     ds, ax
    mov     es, ax

    ; Loading the SS register with a MOV instruction inhibits all interrupts 
    ; until after the execution of the next instruction. This operation allows 
    ; a stack pointer to be loaded into the ESP register with the next 
    ; instruction (MOV ESP, stack-pointer value) before an interrupt occurs
    mov     ss, ax
    mov     sp, STACK_ADDR

    call    get_base

get_base:

    pop     bp
    sub     bp, (get_base - bpb)

%define BPB(size, field) size [ bp + bpb_fat32. %+ field ]

    ; Save BIOS booting drive number
    mov     gs, dx

    ; Check INT 13h extensions presence (LBA support)
    ; If no LBA support, then terminate, CHS will not be supported
    ; most HDDs/BIOSes released after 1996 implement LBA...
    mov     bx, 55AAh
    mov     ah, 41h
    int     13h

    ; CF: set on not present, clear if present
    jc      fatal_error

    ; BX must be AA55h
    cmp     bx, 0AA55h
    jnz     fatal_error

    ; CX & 1: device access using the packet structure (DAP)
    test    cl, 1
    jz      fatal_error

    ; Get drive parameters
    mov     si, TMP_SECTOR
    mov     word [si + 0], 1Eh  ; For v2.x (1Ah for v1.x)
    mov     word [si + 2], 0    ; Workaround some shit bugs
    mov     dx, gs              ; Set drive number
    mov     ah, 48h
    int     13h

    ; CF: set on error
    jc      fatal_error 

    ; Check that bytes per sector match (BPB vs BIOS)
    mov     ax, word [si + 18h]
    cmp     BPB(word, bytes_per_sector), ax

    jz      sector_size_match

sector_size_mismatch:

    call    print_error
    db      'E01', 0

sector_size_match:

    ; Hidden sectors = sectors preceeding the current volume
    ; Reserved sectors = 1 or more (1: only boot sector)

    ; Read the whole root cluster/directory

    mov     esi, BPB(dword, root_cluster)
    call    read_clusters

find_file:

    ; Loop through the root directory entries
    xor     ebx, ebx
    mov     ax, BPB(word, bytes_per_sector)
    mov     bl, BPB(byte, sectors_per_cluster)
    mul     bx
    push    ax

    mov     di, LDR_SEGMENT
    mov     bx, LDR_OFFSET
    mov     ds, di

.next_cluster:

    mov     ax, word [esp]

.check_direntry:

    lea     di, [bx + fat32_direntry.name]

    ; If DIR_Name[0] == 00H, then the directory entry is free, 
    ; and there are no allocated directory entries after this one
    cmp     byte [di], 0
    jz      file_not_found

    ; If DIR_Name[0] == E5H, then the directory entry is free
    cmp     byte [di], 0E5h
    jz      .next_direntry

    ; Check if the file name matches the target file name
    cmp     dword [di + 0], TARGET_FILENAME_0_3
    jnz     .next_direntry
    cmp     dword [di + 4], TARGET_FILENAME_4_7
    jnz     .next_direntry
    cmp     dword [di + 7], TARGET_FILENAME_7_11
    jnz     .next_direntry

    ; Found a match, read the file's content
    movzx   esi, word [bx + fat32_direntry.starting_cluster]
    xor     di, di
    mov     ds, di
    call    read_clusters

    ; Set DL and jump to the code
    mov     dx, gs
    jmp     LDR_SEGMENT : LDR_OFFSET

.next_direntry:

    add     bx, fat32_direntry_size
    jnc     .no_carry

    ; Update the DS segment!
    mov     di, ds
    add     di, 1000h
    mov     ds, di

.no_carry:

    sub     ax, fat32_direntry_size
    jnz     .check_direntry

    dec     cx
    jnz     .next_cluster

file_not_found:

    call    print_error
    db      'E02', 0

fatal_error:

    call    print_error
    db      'E03', 0

;*******************************************************************************

; DI:BX = destination address (buffer)
; EAX = logical sector number to read
; ECX = number of sectors to be read
read_sectors:

    push    si

    ; Push the Disk Address Packet (DAP) on the stack

    push    dword 0     ; \ Starting absolute block number
    push    eax         ; / 
    push    di          ; \ DI:BX pointer to the memory buffer (destination)
    push    bx          ; /
    push    word 1      ; Number of sectors to be read
    push    word 10h    ; sizeof(DAP)
    mov     si, sp

    pusha               ; Backup GPRs
    mov     dx, gs      ; Set drive index
    mov     ah, 42h     ; 
    int     13h         ; Extended Read Sectors From Drive (LBA)
    popa                ; Restore GPRs

    lea     sp, [si + 10h]  ; Fix the stack (DAP)
    pop     si

    ; CF: set on error
    jc      .end 
    inc     eax                             ; Increment logical sector
    add     bx, BPB(word, bytes_per_sector) ; Increment offset
    jnc     .no_carry                       ; Exceeded 16-bit offset?
    add     di, 1000h                       ; Increment segment

.no_carry:

    dec     ecx
    jnz     read_sectors

.end:

    retn

;*******************************************************************************

; ESI = cluster number
; CX  = number of clusters read (return value)
read_clusters:

    mov     di, LDR_SEGMENT
    mov     bx, LDR_OFFSET

    xor     cx, cx 
    push    cx

    mov     eax, BPB(dword, hidden_sector_count)
    push    ebx
    movzx   ebx, BPB(word, reserved_sectors)
    add     eax, ebx 
    pop     ebx
    push    eax ; FAT offset (in sectors)
    movzx   eax, BPB(byte, fat_count)
    mul     BPB(dword, sectors_per_fat_32)
    push    eax ; FAT size (in sectors)

.next_cluster:

    mov     eax, esi
    sub     eax, 2
    movzx   ecx, BPB(byte, sectors_per_cluster)
    mul     ecx
    add     eax, dword [esp + 0] ; FAT size
    add     eax, dword [esp + 4] ; FAT offset
    call    read_sectors
    inc     word [esp + 8] ; Increment clusters read count

    mov     eax, esi
    shl     eax, 2
    div     BPB(word, bytes_per_sector) ; EDX = EAX % bytes_per_sector
    add     eax, dword [esp + 4] ; FAT offset
    xor     ecx, ecx
    inc     ecx

    pushad
    xor     di, di
    mov     bx, TMP_SECTOR
    call    read_sectors    ; Read a FAT sector
    popad

    mov     si, TMP_SECTOR
    add     si, dx          ; Add the reminder offset
    mov     esi, dword [si] ; Read the 32-bit FAT entry value
    and     esi, 0FFFFFFFh  ; Mask it, only 28 bits actually used
    cmp     esi, 0FFFFFF8h  ; Check for end of clusterchain
    jc      .next_cluster   ; Jump if below

    add     sp, 8
    pop     cx

    retn

;*******************************************************************************

print_error:

    xor     ax, ax
    xor     bx, bx
    mov     ds, ax
    pop     si
    
    ; Write Text in Teletype Mode
    mov     ah, 0Eh

.next_char:

    lodsb
    test    al, al
    jz      .halt
    int     10h
    jmp     .next_char

.halt:

    hlt
    jmp .halt

;*******************************************************************************

; safety check that we did not exceed the boot sector size
%if ($-$$) > 510
    %warning "Exceeded boot sector size"
%endif

; %assign used_bytes ($-$$)
; %warning used_bytes

; fill unused space
times 510 - ($-$$) db 0CCh

; boot sector signature (disk is bootable, else BIOS prints error)
dw 0xAA55
