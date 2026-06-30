#pragma once
#include <windows.h>
#include <tlhelp32.h>
#include <vector>
#include <string>
#include <iostream>

namespace Memory {
    extern HANDLE hProcess;
    extern DWORD processId;
    extern HWND hwnd;
    extern uintptr_t baseAddress;

    // Find window by partial title match
    HWND FindRobloxWindow() {
        HWND hwnd = NULL;
        do {
            hwnd = FindWindowEx(NULL, hwnd, NULL, NULL);
            if (!hwnd) continue;
            char title[256];
            GetWindowTextA(hwnd, title, sizeof(title));
            std::string titleStr(title);
            if (titleStr.find("Roblox") != std::string::npos) {
                return hwnd;
            }
        } while (hwnd != NULL);
        return NULL;
    }

    DWORD GetProcessId(HWND hwnd) {
        DWORD pid;
        GetWindowThreadProcessId(hwnd, &pid);
        return pid;
    }

    uintptr_t GetModuleBaseAddress(DWORD pid, const wchar_t* moduleName) {
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
        if (snapshot == INVALID_HANDLE_VALUE) return 0;

        MODULEENTRY32W me = { sizeof(me) };
        uintptr_t base = 0;

        if (Module32FirstW(snapshot, &me)) {
            do {
                if (_wcsicmp(me.szModule, moduleName) == 0) {
                    base = (uintptr_t)me.modBaseAddr;
                    break;
                }
            } while (Module32NextW(snapshot, &me));
        }

        CloseHandle(snapshot);
        return base;
    }

    bool Init() {
        hwnd = FindRobloxWindow();
        if (!hwnd) {
            std::cerr << "[!] Roblox window not found." << std::endl;
            return false;
        }

        processId = GetProcessId(hwnd);
        if (!processId) {
            std::cerr << "[!] Failed to get process ID." << std::endl;
            return false;
        }

        hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
        if (!hProcess) {
            std::cerr << "[!] Failed to open process. Error: " << GetLastError() << std::endl;
            return false;
        }

        baseAddress = GetModuleBaseAddress(processId, L"RobloxPlayerBeta.exe");
        if (!baseAddress) {
            // Try alternate module name
            baseAddress = GetModuleBaseAddress(processId, L"RobloxPlayerBeta.dll");
        }
        if (!baseAddress) {
            std::cerr << "[!] Failed to get module base address." << std::endl;
            return false;
        }

        std::cout << "[+] Roblox window found: 0x" << std::hex << (uintptr_t)hwnd << std::dec << std::endl;
        std::cout << "[+] Process ID: " << processId << std::endl;
        std::cout << "[+] Base Address: 0x" << std::hex << baseAddress << std::dec << std::endl;

        return true;
    }

    template <typename T>
    T Read(uintptr_t address) {
        T value = {};
        ReadProcessMemory(hProcess, (LPCVOID)address, &value, sizeof(T), NULL);
        return value;
    }

    template <typename T>
    bool Read(uintptr_t address, T& out) {
        return ReadProcessMemory(hProcess, (LPCVOID)address, &out, sizeof(T), NULL) != 0;
    }

    bool ReadBuffer(uintptr_t address, void* buffer, size_t size) {
        return ReadProcessMemory(hProcess, (LPCVOID)address, buffer, size, NULL) != 0;
    }

    template <typename T>
    bool Write(uintptr_t address, T value) {
        return WriteProcessMemory(hProcess, (LPVOID)address, &value, sizeof(T), NULL) != 0;
    }

    void Cleanup() {
        if (hProcess) {
            CloseHandle(hProcess);
            hProcess = NULL;
        }
    }

    // Read pointer chain: base -> offsets[0] -> offsets[1] -> ... -> final value
    template <typename T>
    T ReadChain(uintptr_t base, const std::vector<uintptr_t>& offsets) {
        uintptr_t addr = Read<uintptr_t>(base);
        if (!addr) return T{};

        for (size_t i = 0; i < offsets.size() - 1; i++) {
            addr = Read<uintptr_t>(addr + offsets[i]);
            if (!addr) return T{};
        }

        if (offsets.empty()) return T{};
        return Read<T>(addr + offsets.back());
    }

    // Read string at address
    std::string ReadString(uintptr_t address, size_t maxLen = 50) {
        char buffer[256] = {};
        size_t len = (maxLen < sizeof(buffer)) ? maxLen : sizeof(buffer) - 1;
        if (ReadBuffer(address, buffer, len)) {
            buffer[len] = '\0';
            return std::string(buffer);
        }
        return "";
    }
}