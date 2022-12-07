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
; [ORG 0000h]

; %define LDR_SEGMENT           0000h
; %define LDR_OFFSET            1000h
; %define RMODE_STACK           7000h

; *****************************************************************************

extern ldr_main
extern ldr_isr_rm_callback

extern pmode_stack
extern rmode_stack

global ldr_entrypoint
global ldr_bios_call
global ldr_jmp_to_rmode

; *****************************************************************************

struc rmode_ctx
    .eax                resd 1
    .ecx                resd 1
    .edx                resd 1
    .ebx                resd 1
    .esp                resd 1
    .ebp                resd 1
    .esi                resd 1
    .edi                resd 1
    .efl                resd 1
    .es                 resw 1
    .ds                 resw 1
endstruc

; *****************************************************************************

; *****************************************************************************
; * General Information                                                       *
; *****************************************************************************

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
; * Loader Entrypoint                                                         *
; *****************************************************************************

; DL = boot drive (floppies: 00h to 7Eh; hdd/removable: 80h to FEh)

[BITS 16]
[SECTION .init]

ldr_entrypoint:

    ; CLI/STI not really needed...
    mov     ax, cs
    mov     ds, ax
    mov     es, ax
    xor     ax, ax
    mov     fs, ax
    
    ; Loading the SS register with a MOV instruction inhibits all interrupts 
    ; until after the execution of the next instruction. This operation allows 
    ; a stack pointer to be loaded into the ESP register with the next 
    ; instruction (MOV ESP, stack-pointer value) before an interrupt occurs
    mov     ss, ax
    mov     sp, rmode_stack

    call    ldr_enable_a20_gate ; Enable A20 gate for high memory access
    call    ldr_isr_detour      ; Detour IVT ISRs

    ; Switch to protected mode (INTEL, Volume 3, Section 9.9), PE = 1
    ; Disable interrupts because currently no IDT setup (IDT soon(tm))
    cli     
    lgdt    [ldr_gdt.descriptor]
    mov     eax, cr0
    or      eax, 1
    mov     cr0, eax

    ; Protected mode 32 bit jump
    jmp     ldr_gdt.pmode32_cseg : dword ldr_pmode32

[BITS 32]

ldr_pmode32:

    mov     ax, ldr_gdt.pmode32_dseg
    mov     fs, ax
    mov     gs, ax
    mov     es, ax
    mov     ds, ax
    mov     ss, ax
    mov     esp, pmode_stack

    ; Enable SSE (OSFXSR = 1)
    mov     eax, cr4
    or      eax, 200h
    mov     cr4, eax

    ; Call the loader, no RETurn expected
    and     edx, 0FFh
    push    edx
    call    ldr_main
    add     esp, 4

ldr_end:

    hlt
    jmp     ldr_end

;*******************************************************************************

;
; __cdecl ldr_bios_call(int n, rmode_ctx* inout)
;
ldr_bios_call:

    push    ebp
    mov     ebp, esp

    ; [esp + 12]    = inout
    ; [esp + 8]     = n
    ; [esp + 4]     = return address
    ; [esp + 0]     = ebp
    lea     ebp, [ebp + 8]

    ; Save GPRs and EFLAGS
    pushad
    pushfd

    ; Setup the BIOS interrupt
    mov     eax, [ebp]
    mov     ebp, [ebp + 4]
    mov     byte [.rmode_int + 0], 0CDh     ; INT XX
    mov     byte [.rmode_int + 1], al       ; 
    mov     byte [.rmode_int + 2], 090h     ; NOP

    ; Save and set the stack
    mov     [ebp + rmode_ctx.esp], esp
    
    ; Setup real mode data segment (used later)
    mov     ecx, ebp
    shr     ecx, 4
    and     ecx, 0F000h

    ; Setup the segment selectors and jump to 16-bit protected mode
    mov     ax, ldr_gdt.pmode16_dseg
    mov     ds, ax
    mov     es, ax
    mov     ss, ax
    mov     fs, ax
    mov     gs, ax

    ; Protected mode 16 bit jump
    jmp     ldr_gdt.pmode16_cseg : dword .pmode16

[BITS 16]

.pmode16:

    ; Disable protected mode (PE = 0)
    mov     eax, cr0
    and     eax, 0FFFFFFFEh 
    mov     cr0, eax

    ; Jump to real mode
    push    word 0      ; cs
    push    word .rmode ; offset
    retf

.rmode:

    xor     ax, ax
    mov     fs, ax
    mov     gs, ax
    mov     ds, cx
    mov     ax, [bp + rmode_ctx.es]
    mov     es, ax 

    mov     eax, [bp + rmode_ctx.esp]
    mov     ebx, eax
    shr     eax, 4
    and     eax, 0F000h
    mov     ss, ax
    mov     sp, bx

    ; Enable interrupts
    sti

    mov     eax, [bp + rmode_ctx.eax]
    mov     ecx, [bp + rmode_ctx.ecx]
    mov     edx, [bp + rmode_ctx.edx]
    mov     ebx, [bp + rmode_ctx.ebx]
    mov     esi, [bp + rmode_ctx.esi]
    mov     edi, [bp + rmode_ctx.edi]
    push    ebp
    push    ebp

.rmode_int:

    int     0FFh 
    nop

    mov     [esp + 4], ebp
    pop     ebp
    pushfd
    pop     dword [bp + rmode_ctx.efl]
    pop     dword [bp + rmode_ctx.ebp]
    mov     [bp + rmode_ctx.eax], eax
    mov     [bp + rmode_ctx.ecx], ecx
    mov     [bp + rmode_ctx.edx], edx
    mov     [bp + rmode_ctx.ebx], ebx
    mov     [bp + rmode_ctx.esi], esi
    mov     [bp + rmode_ctx.edi], edi

    ; Switch back to protected mode 32 bit
    ; Disable interrupts (no IDT) and switch back to protected mode (PE = 1)
    cli
    mov     eax, cr0
    or      eax, 1
    mov     cr0, eax
    jmp     ldr_gdt.pmode32_cseg : dword .pmode32

[BITS 32]

.pmode32:

    mov     ax, ldr_gdt.pmode32_dseg
    mov     fs, ax
    mov     gs, ax
    mov     es, ax
    mov     ds, ax

    mov     ss, ax
    mov     esp, [ebp + rmode_ctx.esp]

    ; Restore GPRs and EFLAGS
    popfd
    popad
    pop     ebp
    retn

;*******************************************************************************

[BITS 16]

ldr_to_pmode32:

    cli
    xor     ax, ax
    mov     ds, ax
    lgdt    [ldr_gdt.descriptor]
    mov     eax, cr0
    or      eax, 1
    mov     cr0, eax
    jmp     ldr_gdt.pmode32_cseg : dword .pmode32

[BITS 32]

.pmode32:

    ; Setup the stack
    xor     eax, eax
    mov     ecx, esp
    and     ecx, 0FFFFh
    mov     ax, ss
    shl     eax, 4
    add     ecx, eax

    ; Setup the segments (selectors)
    mov     ax, ldr_gdt.pmode32_dseg
    mov     es, ax
    mov     ds, ax
    mov     ss, ax
    mov     esp, ecx

    retn

;*******************************************************************************

[BITS 32]

ldr_to_rmode:

    jmp     ldr_gdt.pmode16_cseg : dword .pmode16

[BITS 16]

.pmode16:

    ; Disable protected mode (PE = 0)
    mov     eax, cr0
    and     eax, 0FFFFFFFEh 
    mov     cr0, eax

    ; Jump to real mode
    push    word 0      ; cs
    push    word .rmode ; offset
    retf

.rmode:

    mov     eax, esp
    shr     eax, 4
    and     eax, 0F000h
    mov     ss, ax
    and     esp, 0FFFFh

    ; Size override to pop 32bit address
    o32     retn

;*******************************************************************************

[BITS 32]

;
; __cdecl ldr_jmp_to_rmode(uint32_t seg, uint32_t offs, rmode_ctx* in)
;
ldr_jmp_to_rmode:

    ; [esp + 12]    = in
    ; [esp + 8]     = offs
    ; [esp + 4]     = seg
    ; [esp + 0]     = return address
    mov     ebp, [esp + 12]
    mov     eax, [esp + 4]
    mov     ecx, [esp + 8]

    ; Setup real mode jmp
    mov     byte [.jmp + 0], 0EAh
    mov     word [.jmp + 1], cx
    mov     word [.jmp + 3], ax

    ; Setup real mode data segment (used later)
    mov     ecx, ebp
    shr     ecx, 4
    and     ecx, 0F000h

    ; Jump back to real mode
    call    dword ldr_to_rmode

[BITS 16]

    xor     ax, ax
    mov     fs, ax
    mov     gs, ax
    mov     es, ax 
    mov     ds, cx

    mov     eax, [bp + rmode_ctx.eax]
    mov     ecx, [bp + rmode_ctx.ecx]
    mov     edx, [bp + rmode_ctx.edx]
    mov     ebx, [bp + rmode_ctx.ebx]
    mov     esi, [bp + rmode_ctx.esi]
    mov     edi, [bp + rmode_ctx.edi]
    mov     ebp, [bp + rmode_ctx.ebp]

    ; Enable interrupts
    sti

.jmp:

    ; Far jmp to rmode code
    jmp     0000h : word 0000h

;*******************************************************************************

[BITS 16]

ldr_isr_detour:

    xor     ax, ax
    mov     fs, ax

    push    eax
    push    ecx

    ; INT 13h detour
    mov     eax, dword [fs:(13h << 2)]
    mov     ecx, eax
    shr     ecx, 16
    mov     byte [ldr_isr_int13h.jmp + 0], 0EAh
    mov     word [ldr_isr_int13h.jmp + 1], ax
    mov     word [ldr_isr_int13h.jmp + 3], cx
    xor     eax, eax
    mov     ax, cs
    shl     eax, 16
    mov     ax, ldr_isr_int13h
    mov     dword [fs:(13h << 2)], eax

    ; INT 15h detour
    mov     eax, dword [fs:(15h << 2)]
    mov     ecx, eax
    shr     ecx, 16
    mov     byte [ldr_isr_int15h.jmp + 0], 0EAh
    mov     word [ldr_isr_int15h.jmp + 1], ax
    mov     word [ldr_isr_int15h.jmp + 3], cx
    xor     eax, eax
    mov     ax, cs
    shl     eax, 16
    mov     ax, ldr_isr_int15h
    mov     dword [fs:(15h << 2)], eax

    pop     ecx
    pop     eax
    retn

;*******************************************************************************

[BITS 16]

ldr_isr_int13h:

    push    13h
    push    ax
    pushf
    push    cs
    push    ldr_isr_rm

.jmp:

    ; Far jmp to the original code
    jmp     0000h : word 0000h

ldr_isr_int15h:

    push    15h
    push    ax
    pushf
    push    cs
    push    ldr_isr_rm

.jmp:

    ; Far jmp to the original code
    jmp     0000h : word 0000h

;*******************************************************************************

[BITS 16]

ldr_isr_rm:

    push    es
    push    ds
    pushfd
    pushad  
    xor     ax, ax
    mov     ds, ax
    push    sp
    push    ss
    
    ; Jump to protected mode 32 bit
    call    dword ldr_to_pmode32

[BITS 32]

    push    esp
    call    dword ldr_isr_rm_callback
    add     esp, 4

    ; Jump back to real mode
    call    dword ldr_to_rmode

[BITS 16]
    
    pop     cx
    pop     ax
    mov     ss, cx
    mov     sp, ax
    popad
    popfd
    pop     ds
    pop     es
    lea     esp, [esp + 4] ; because add/inc causes efl corruption
    
    ; IRET not used to avoid fixing EFLAGS on stack
    retf    2

;*******************************************************************************

[BITS 16]

ldr_enable_a20_gate:

    pusha

    ; Method 1: BIOS INT 15H Service

    ; Query A20 Gate Support (BIOS)
    mov     ax, 2403h
    int     15h
    jc      .failed
    cmp     ah, 00h
    jnz     .failed

    ; Get A20 Gate Status
    mov     ax, 2402h
    int     15h
    jc      .failed
    cmp     ah, 00h
    jnz     .failed
    
    ; Check if it's already enabled
    cmp     al, 01h
    jz      .end 
    
    ; If not enabled, try to enable it
    mov     ax, 2401h
    int     15h
    jc      .failed
    cmp     ah, 00h
    jz      .end

.failed:

    ; Method 2: Fast Enable (System Control Port A)
    in      al, 92h
    or      al, 02h
    out     92h, al

.end:

    popa
    retn

;*******************************************************************************

%define gdt_descriptor(base, limit, flags, access) \
    dq  ((limit & 0FFFFh)       << 00) |        \
        ((base & 0FFFFh)        << 16) |        \
        (((base >> 16) & 0FFh)  << 32) |        \
        ((access & 0FFh)        << 40) |        \
        (((limit >> 16) & 0Fh)  << 48) |        \
        ((flags & 0Fh)          << 52) |        \
        (((base >> 24) & 0FFh)  << 56)

; Global Descriptor Table (GDT) 

;   Base            : 0
;   Limit           : FFFFF
;   Granularity     : 4 KB
;   Privilege       : 0 (highest)

align 16

ldr_gdt:

    ; Null descriptor
    ; The first descriptor in the GDT is not used by the processor.
    gdt_descriptor(0, 0, 0, 0)

.pmode32_cseg equ $ - ldr_gdt

    ; Code descriptor, 32 bit protected mode
    gdt_descriptor(0, 0FFFFFh, 1100b, 10011010b)

.pmode32_dseg equ $ - ldr_gdt

    ; Data descriptor, 32 bit protected mode
    gdt_descriptor(0, 0FFFFFh, 1100b, 10010010b)

.pmode16_cseg equ $ - ldr_gdt

    ; Code descriptor, 16 bit protected mode
    gdt_descriptor(0, 0FFFFFh, 0000b, 10011110b)

.pmode16_dseg equ $ - ldr_gdt

    ; Data descriptor, 16 bit protected mode
    gdt_descriptor(0, 0FFFFFh, 0000b, 10010010b)

align 16

.descriptor:

    dw $ - ldr_gdt - 1  ; Limit
    dd ldr_gdt          ; Base

;*******************************************************************************

msg_init: db 'Furtim vigilans.', 13, 10, 0

;*******************************************************************************




