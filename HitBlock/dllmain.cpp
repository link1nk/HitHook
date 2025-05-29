#include "pch.h"

#ifdef _WIN64
#define MINIMAL_SIZE 12
#define HOOK_SIZE    18
#define HOOK_ADDRESS 0x7FF61CD81097 // Esse endereço muda a cada compilação!
#else
#define MINIMAL_SIZE 5
#define HOOK_SIZE    6
#define HOOK_ADDRESS 0x811074       // Esse endereço muda a cada compilação!
#endif // _WIN64

extern "C" ULONG_PTR JmpBack = 0;

extern "C" void __cdecl DetourFunction(void);

bool Hook(PVOID TargetHookAddress, PVOID DetourFunctionAddress, SIZE_T HookFunctionSize)
{
    if (HookFunctionSize < MINIMAL_SIZE)
    {
        std::cout << "[ERROR] Função alvo é menor do que o necessário para o hook\n";
        return false;
    }

    DWORD OldProtect;
    if (!VirtualProtect(TargetHookAddress, HookFunctionSize, PAGE_EXECUTE_READWRITE, &OldProtect))
    {
        std::cout << "[ERROR] Falha ao mudar as permissões da página\n";
        return false;
    }

    memset(TargetHookAddress, 0x90, HookFunctionSize);

#ifdef _WIN64
    // Patch de 64 bits: mov rax, <endereço>; jmp rax
    BYTE Patch[] = {
        0x48, 0xB8,                          // mov rax, <imm64>
        0, 0, 0, 0, 0, 0, 0, 0,              // endereço 64-bit
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
        std::cout << "[ERROR] Falha ao restaurar permissões\n";
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

