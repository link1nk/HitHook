#include "pch.h"

#ifdef _WIN64
#define MINIMAL_SIZE 12
#define HOOK_SIZE    18
#define HOOK_ADDRESS 0x7FF61CD81097 // Esse endere�o muda a cada compila��o!
#else
#define MINIMAL_SIZE 5
#define HOOK_SIZE    6
#define HOOK_ADDRESS 0x811074       // Esse endere�o muda a cada compila��o!
#endif // _WIN64

extern "C" ULONG_PTR JmpBack = 0;

extern "C" void __cdecl DetourFunction(void);

bool Hook(PVOID TargetHookAddress, PVOID DetourFunctionAddress, SIZE_T HookFunctionSize)
{
    if (HookFunctionSize < MINIMAL_SIZE)
    {
        std::cout << "[ERROR] Fun��o alvo � menor do que o necess�rio para o hook\n";
        return false;
    }

    DWORD OldProtect;
    if (!VirtualProtect(TargetHookAddress, HookFunctionSize, PAGE_EXECUTE_READWRITE, &OldProtect))
    {
        std::cout << "[ERROR] Falha ao mudar as permiss�es da p�gina\n";
        return false;
    }

    memset(TargetHookAddress, 0x90, HookFunctionSize);

#ifdef _WIN64
    // Patch de 64 bits: mov rax, <endere�o>; jmp rax
    BYTE Patch[] = {
        0x48, 0xB8,                          // mov rax, <imm64>
        0, 0, 0, 0, 0, 0, 0, 0,              // endere�o 64-bit
        0xFF, 0xE0                           // jmp rax
    };

    *reinterpret_cast<ULONG_PTR*>(&Patch[2]) = reinterpret_cast<ULONG_PTR>(DetourFunctionAddress);

#else
    // Patch de 32 bits: jmp <rel32>
    BYTE Patch[5];
    Patch[0] = 0xE9;

    LONG RelativeOffset = (LONG)((ULONG_PTR)DetourFunctionAddress - (ULONG_PTR)TargetHookAddress - 5);
    *reinterpret_cast<LONG*>(&Patch[1]) = RelativeOffset;
#endif

    memcpy(TargetHookAddress, Patch, sizeof(Patch));

    if (!VirtualProtect(TargetHookAddress, HookFunctionSize, OldProtect, &OldProtect))
    {
        std::cout << "[ERROR] Falha ao restaurar permiss�es\n";
        return false;
    }

    return true;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        JmpBack = HOOK_ADDRESS + HOOK_SIZE;

        if (Hook((PVOID)HOOK_ADDRESS, DetourFunction, HOOK_SIZE))
        {
            MessageBox(0, L"Hook efetuado com sucesso", L"Sucesso", 0);
        }
        else
        {
            MessageBox(0, L"Nao foi possivel efetuar o Hook", L"Ops", 0);
        }

        break;
    }
    }
    return TRUE;
}

