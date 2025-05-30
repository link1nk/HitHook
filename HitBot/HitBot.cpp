#include <Windows.h>
#include <TlHelp32.h>
#include <cstdio>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

DWORD ProcessId;
HWND  WindowHandle;
INT32 HP;

DWORD GetProcessIdByName(const std::wstring& ProcessName)
{
    PROCESSENTRY32W PE32;
    PE32.dwSize = sizeof(PROCESSENTRY32W);

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        return NULL;
    }

    if (!Process32FirstW(hSnapshot, &PE32))
    {
        CloseHandle(hSnapshot);
        return NULL;
    }

    do
    {
        if (_wcsicmp(PE32.szExeFile, ProcessName.c_str()) == 0)
        {
            CloseHandle(hSnapshot);
            std::wcout << ProcessName.c_str() << L" PID: " << PE32.th32ProcessID << '\n';
            return PE32.th32ProcessID;
        }

    } while (Process32NextW(hSnapshot, &PE32));

    CloseHandle(hSnapshot);

    return 0;
}

ULONG_PTR GetModuleBase(PCWCHAR ModuleName, DWORD PID)
{
    MODULEENTRY32 ModuleEntry = { 0 };

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, PID);

    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        std::cout << "Falha ao criar um snapshot\n";
        return NULL;
    }

    ModuleEntry.dwSize = sizeof(ModuleEntry);

    if (!Module32First(hSnapshot, &ModuleEntry))
    {
        std::cout << "Não foi encontrado nenhum modulo\n";
        return NULL;
    }

    do
    {
        if (!wcscmp(ModuleEntry.szModule, ModuleName))
        {
            CloseHandle(hSnapshot);

            ULONG_PTR ModuleBase = reinterpret_cast<ULONG_PTR>(ModuleEntry.modBaseAddr);
            std::cout << "ModuleBase: 0x" << std::hex << ModuleBase << '\n';

            return ModuleBase;
        }

    } while (Module32Next(hSnapshot, &ModuleEntry));

    CloseHandle(hSnapshot);

    return NULL;
}

int main(void)
{
    ProcessId = GetProcessIdByName(L"HitHealth.exe");

    if (ProcessId == 0)
    {
        return 1;
    }

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessId);

    if (hProcess == INVALID_HANDLE_VALUE)
    {
        std::cout << "Não foi possivel abrir um handle para o processo alvo\n";
        return 1;
    }

    ULONG_PTR BaseAddress = GetModuleBase(L"HitHealth.exe", ProcessId);

#ifdef _WIN64
    PVOID HealthAddress = (PVOID)0;
#else
    PVOID HealthAddress = (PVOID)0x0019FF28;
#endif // _WIN64

    while (true)
    {
        DWORD HealthValue;
        DWORD FullLife = 105;

        ReadProcessMemory(hProcess, HealthAddress, &HealthValue, sizeof(DWORD), nullptr);
        
        if (HealthValue < 50)
        {
            WriteProcessMemory(hProcess, HealthAddress, &FullLife, sizeof(DWORD), nullptr);
            std::cout << "Vida recuperada!\n";
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}