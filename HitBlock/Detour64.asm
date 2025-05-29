PUBLIC DetourFunction

EXTERN JmpBack:QWORD

.CODE

DetourFunction PROC
    mov edx, [rsp + 20h]
    mov rax, 7FF61CD82284h
    lea rcx, [rax]
    mov rax, Jmpback
    jmp rax
DetourFunction ENDP

END