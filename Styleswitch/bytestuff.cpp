#include "pch.h"
#include <windows.h>
#include "PatternScan.h"
#include "mempatch.h"
#include <string>
#include <sstream>
#include "Hook/MinHook.h"

#pragma comment(lib, "resources/libMinHook.x64.lib")


void Patch(BYTE* dst, BYTE* src, unsigned int size)
{
    DWORD oldProtect;
    VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(dst, src, size);
    VirtualProtect(dst, size, oldProtect, &oldProtect);
}

bool PatchBytes(BYTE* address, BYTE* bytes, SIZE_T size) {
    DWORD oldProtect;
    if (!VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        return false;
    }

    memcpy(address, bytes, size);

    if (!FlushInstructionCache(GetCurrentProcess(), address, size)) {
        return false;
    }

    DWORD temp;
    if (!VirtualProtect(address, size, oldProtect, &temp)) {
        return false;
    }

    return true;
}

void DisableRestriction()
{
    BYTE* atk_inside = PatternScan(GetModuleHandle(NULL), "83 B8 88 01 00 00 02 0F 94 C3 E9"); // changes condition atk inside so that it checks for style value 1 instead
    if (atk_inside)
    {
        BYTE nopPatch[] = { 0x83, 0xB8, 0x68, 0x01, 0x00, 0x00, 0x01 };
        Patch(atk_inside, nopPatch, sizeof(nopPatch));
    }

    BYTE* atk_outside = PatternScan(GetModuleHandle(NULL), "83 B8 88 01 00 00 03 0F 94 C3 E9"); // changes condition atk inside so that it checks for style value 4 instead
    if (atk_outside)
    {
        BYTE nopPatch[] = { 0x83, 0xB8, 0x68, 0x01, 0x00, 0x00, 0x04 };
        Patch(atk_outside, nopPatch, sizeof(nopPatch));
    }
}