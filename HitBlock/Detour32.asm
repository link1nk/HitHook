PUBLIC DetourFunction

.MODEL FLAT, C

EXTERN JmpBack:DWORD

.CODE

DetourFunction PROC
    jmp [Jmpback]
DetourFunction ENDP

END