;;; Format macro for NASM
;;; Version 0.1.0
;;;
;;; Made by Natalia Cholewa <3
;;; See https://github.com/natanalt/fmt-nasm for detailed documentation
;;;

%ifndef FMT_INCLUDED
%define FMT_INCLUDED

%if __?BITS?__ != 16
    %fatal "I'm sorry, but format macros currently work only with 16 bit x86"
%endif

%define FMT_VERSION_MAJOR 0
%define FMT_VERSION_MINOR 1
%define FMT_VERSION_PATCH 0

%ifndef FMT_STRING_SEG
    %define FMT_STRING_SEG .rodata
%endif

%ifndef FMT_FN_PREFIX
    %define FMT_FN_PREFIX log_
%endif

; Since %+ doesn't cause concatenated tokens to be actually evaluated,
; this little hack forces the evaluation of FMT_FN_PREFIX at macro call
; time
%macro FMT_FN_hack 1
    %define FMT_FN(x) %1 %+ x
%endmacro
FMT_FN_hack FMT_FN_PREFIX

%macro print 1
    %push fmt_print
    
    fmt_put_debug_nop
    fmt_put_debug_nop
    
    %define %$str %1
    %strlen %$len %$str
    %assign %$i 1
    
    %assign %$lit_start -1
    %assign %$lit_len 0
    
    %rep %$len
        %if %$i > %$len
            %exitrep
        %endif
        
        %substr %$curr %$str %$i
        %if %$curr == "{"
            fmt_try_output_literal
            
            %assign %$fmt_start %$i + 1
            %assign %$fmt_len 0
            %rep %$len - %$fmt_start + 1
                %if %$fmt_start + %$fmt_len - 1 > %$len
                    %error "Unexpected end of string in format string"
                %endif
                
                %substr %$fmt_curr %$str %$fmt_start+%$fmt_len
                %if %$fmt_curr == "}"
                    %exitrep
                %else
                    %assign %$fmt_len %$fmt_len + 1
                %endif
            %endrep
            
            %if %$fmt_len <= 0
                %fatal Invalid empty format string at %$i
            %endif
            
            %assign %$i %$i + %$fmt_len + 1
            %substr %$fmt %$str %$fmt_start,%$fmt_len
            %substr %$fmt_first2 %$fmt 1,2
            %if %$fmt_first2 == "ax" || \
                %$fmt_first2 == "bx" || \
                %$fmt_first2 == "cx" || \
                %$fmt_first2 == "dx" || \
                %$fmt_first2 == "sp" || \
                %$fmt_first2 == "bp" || \
                %$fmt_first2 == "si" || \
                %$fmt_first2 == "di" || \
                %$fmt_first2 == "cs" || \
                %$fmt_first2 == "ds" || \
                %$fmt_first2 == "es" || \
                %$fmt_first2 == "ss" || \
                %$fmt_first2 == "fs" || \
                %$fmt_first2 == "gs" || \
                %$fmt_first2 == "fl"
                
                %if %$fmt_len != 2 && %$fmt_len != 4
                    %error Invalid format string at %$fmt_start
                %endif

                %if %$fmt_len == 2
                    %substr %$fmt_type "x" 1
                %elif %$fmt_len == 4
                    %substr %$fmt_type %$fmt 4
                %endif
                
                fmt_put_debug_nop
                %if %$fmt_first2 != "ax"
                    push ax
                %endif
                
                %if %$fmt_first2 == "fl"
                    pushf
                    pop ax
                %elif %$fmt_first2 != "ax"
                    %deftok %$fmt_reg %$fmt_first2
                    mov ax, %$fmt_reg
                %endif
                
                %if %$fmt_type == "u"
                    call FMT_FN(print_word_dec)
                %elif %$fmt_type == "b"
                    call FMT_FN(print_word_bin)
                %elif %$fmt_type == "x"
                    call FMT_FN(print_word_hex)
                %elif %$fmt_type == "c"
                    call FMT_FN(print_word_chr)
                %else
                    %error Invalid format type %$fmt_type
                %endif
                
                %if %$fmt_first2 != "ax"
                    pop ax
                %endif
                fmt_put_debug_nop
                
            %elif %$fmt_first2 == "al" || \
                %$fmt_first2 == "bl" || \
                %$fmt_first2 == "cl" || \
                %$fmt_first2 == "dl" || \
                %$fmt_first2 == "ah" || \
                %$fmt_first2 == "bh" || \
                %$fmt_first2 == "ch" || \
                %$fmt_first2 == "dh"
                
                %if %$fmt_len != 2 && %$fmt_len != 4
                    %error Invalid format string at %$fmt_start
                %endif
                
                %if %$fmt_len == 2
                    %substr %$fmt_type "x" 1
                %elif %$fmt_len == 4
                    %substr %$fmt_type %$fmt 4
                %endif
                
                fmt_put_debug_nop
                
                %if %$fmt_first2 != "al"
                    push ax
                %endif
                
                %deftok %$fmt_reg %$fmt_first2
                mov al, %$fmt_reg
                
                %if %$fmt_type == "u"
                    call FMT_FN(print_byte_dec)
                %elif %$fmt_type == "b"
                    call FMT_FN(print_byte_bin)
                %elif %$fmt_type == "x"
                    call FMT_FN(print_byte_hex)
                %elif %$fmt_type == "c"
                    call FMT_FN(print_byte_chr)
                %else
                    %error Invalid format type %$fmt_type
                %endif
                
                %if %$fmt_first2 != "al"
                    pop ax
                %endif
                fmt_put_debug_nop
            %elif %$fmt_first2 == "b:" || %$fmt_first2 == "B:"
                %if %$fmt_first2 == "B:"
                    %substr %$fmt_type "x" 1
                    
                    %assign %$fmt_expr_len %$fmt_len - 2
                    %substr %$fmt_expr_raw %$fmt 3,%$fmt_expr_len
                    %deftok %$fmt_expr %$fmt_expr_raw
                %else
                    %substr %$fmt_type %$fmt 3
                
                    %assign %$fmt_expr_len %$fmt_len - 4
                    %substr %$fmt_expr_raw %$fmt 5,%$fmt_expr_len
                    %deftok %$fmt_expr %$fmt_expr_raw
                %endif                
                
                fmt_put_debug_nop
                push ax
                
                mov al, %$fmt_expr
                %if %$fmt_type == "u"
                    call FMT_FN(print_byte_dec)
                %elif %$fmt_type == "b"
                    call FMT_FN(print_byte_bin)
                %elif %$fmt_type == "x"
                    call FMT_FN(print_byte_hex)
                %elif %$fmt_type == "c"
                    call FMT_FN(print_byte_chr)
                %else
                    %error Invalid format type %$fmt_type
                %endif
                
                pop ax
                fmt_put_debug_nop
            %elif %$fmt_first2 == "w:" || %$fmt_first2 == "W:"
                %if %$fmt_first2 == "W:"
                    %substr %$fmt_type "x" 1
                    
                    %assign %$fmt_expr_len %$fmt_len - 2
                    %substr %$fmt_expr_raw %$fmt 3,%$fmt_expr_len
                    %deftok %$fmt_expr %$fmt_expr_raw
                %else
                    %substr %$fmt_type %$fmt 3
                
                    %assign %$fmt_expr_len %$fmt_len - 4
                    %substr %$fmt_expr_raw %$fmt 5,%$fmt_expr_len
                    %deftok %$fmt_expr %$fmt_expr_raw
                %endif
                
                push ax
                
                mov ax, %$fmt_expr
                %if %$fmt_type == "u"
                    call FMT_FN(print_word_dec)
                %elif %$fmt_type == "b"
                    call FMT_FN(print_word_bin)
                %elif %$fmt_type == "x"
                    call FMT_FN(print_word_hex)
                %elif %$fmt_type == "c"
                    call FMT_FN(print_word_chr)
                %else
                    %error Invalid format type %$fmt_type
                %endif
                
                pop ax
                
                fmt_put_debug_nop
            %else
                %error Invalid format string at %$fmt_start
            %endif
        %else
            %if %$lit_start == -1
                %assign %$lit_start %$i
                %assign %$lit_len 1
            %else
                %assign %$lit_len %$lit_len + 1
            %endif
        %endif
        
        %assign %$i %$i+1
    %endrep
    
    fmt_try_output_literal
    
    call FMT_FN(print_finish)
    
    fmt_put_debug_nop
    fmt_put_debug_nop
    
    %pop
%endmacro

%macro fmt_try_output_literal 0
    %ifnctx fmt_print
        %error "Usage of fmt_try_output_literal outside of fmt_print context"
    %endif

    %if %$lit_len > 0 && %$lit_start != -1
        fmt_put_debug_nop
        %substr %$lit_string %$str %$lit_start,%$lit_len
        %assign %$lit_start -1
        %assign %$lit_len 0

        [section FMT_STRING_SEG]
        %%str: db %$lit_string, 0
        __?SECT?__
        
        push si
        
        %ifdef FMT_UPDATE_DS
            push ds
            mov si, seg FMT_STRING_SEG
            mov ds, si
        %endif
        
        mov si, %%str
        call FMT_FN(print_string)
        pop si
        
        %ifdef FMT_UPDATE_DS
            pop ds
        %endif
        fmt_put_debug_nop
    %endif
%endmacro

;%define FMT_DEBUG_NOPS
%macro fmt_put_debug_nop 0
    %ifdef FMT_DEBUG_NOPS
        nop
    %endif
%endmacro

%endif ; FMT_INCLUDED
