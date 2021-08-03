;; Natnix86 bootloader
;; Meant for floppies
;; Max 4KB
;; Assumes the BIOS doesn't suck

cpu 8086

%include "hlc.inc"

%define SECTOR_SIZE         512
%define MAX_BOOT_SIZE       4096
%define MAX_BOOT_SECTORS    (MAX_BOOT_SIZE / SECTOR_SIZE)
%define PART2_SIGNATURE     "NX"
%define STACK_SIZE          1024

section .text vstart=0x7C00

    fn init
        cli
            xor ax, ax
            mov ds, ax
            mov es, ax
            mov ss, ax
            mov sp, stack.top
            jmp 0:$+5
        sti

        mov byte [boot_device], dl

        mov al, MAX_BOOT_SECTORS - 1
        mov ch, 0
        mov cl, 2
        mov dh, 0
        mov bx, p2
        call read_disk_chs

        mov ax, word [p2]
        if ax, !=, PART2_SIGNATURE
            mov si, msg1.bad_sign
            call print_string
            jmp freeze
        end

        jmp main
    end


    ; ds:si - null terminated string
    fn print_string
        cld
        push si
        push ax
        .print_loop:
            lodsb
            or al, al
            jz .finish
            mov ah, 0x0E
            int 0x10
            jmp .print_loop
        .finish:
            pop ax
            pop si
            ret
    end


    ; al - num
    fn print_hex8
        push bx
            mov bx, hex
            push ax
                shr al, 1
                shr al, 1
                shr al, 1
                shr al, 1
                xlatb
                mov ah, 0x0E
                int 0x10
            pop ax
            push ax
                and al, 0x0F
                xlatb
                mov ah, 0x0E
                int 0x10
            pop ax
        pop bx
        ret
    end


    ; Retries 3 times on fail and fails after third time
    ;
    ; al - n sectors
    ; ch/cl/dh - as in int 0x13
    ; dl - target drive
    ; es:bx - buffer
    fn read_disk_chs
        call .try_read
        call .try_read
        call .try_read
        jnc .return

        mov si, msg1.disk_error
        call print_string
        call print_hex8
        jmp freeze

        .return:
            ret
        
        .try_read:
            push ax
                xor ah, ah
                int 0x13
            pop ax
            mov ah, 0x02
            int 0x13
            jc .return
            add sp, 2
            jmp .return
    end


    fn freeze
        cli
        hlt
        jmp freeze
    end


    hex db `0123456789ABCDEF`

    msg1:
        .disk_error db `fatal: disk read error: \0`
        .bad_sign   db `fatal: invalid p2 signature\0`

    times 510 - ($ - $$) db 0
    db 0x55, 0xAA
    p2:
    db PART2_SIGNATURE

    fn main

        ; Set cursor pos to (0,0)
        mov ah, 2
        xor bh, bh
        xor dx, dx
        int 0x10

        ; Clear screen
        mov ax, 0xB800
        mov es, ax
        xor di, di
        mov cx, 80*25
        mov ax, 0x0720
        rep stosw

        ; Print welcome message
        mov si, msg.welcome
        call print_string

        ; Print a line
        mov cx, 80
        until cx, =, 0
            mov ax, 0x0E00 | 196
            int 0x10
            dec cx
        end
        mov ax, 0x0E00 | '\r'
        int 0x10
        mov ax, 0x0E00 | '\n'
        int 0x10


        jmp $
    end

    msg:
        .welcome db `Natnix86 Bootloader\r\n\0`

    times MAX_BOOT_SIZE - ($ - $$) db 0

section .bss vfollows=.text
    boot_device resb 1
    stack:
        resb STACK_SIZE
        .top:
