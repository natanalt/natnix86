cpu 8086

mov ah, 0x0E
mov al, '!'
int 0x10
int 0x10
int 0x10
int 0x10
int 0x10
int 0x10
int 0x10
int 0x10
jmp $
