#include "pch.h"
#include <windows.h>
#include <string>
#include <sstream>


void* VirtualAllocNear(void* target, SIZE_T size) {
    const intptr_t RANGE = 0x70000000; // ~1.87GB
    uintptr_t base = (uintptr_t)target;
    uintptr_t start = (base > RANGE) ? base - RANGE : 0;
    uintptr_t end = base + RANGE;

    // try several hints starting near the target
    for (uintptr_t hint = base; hint >= start; hint -= 0x10000) {
        void* p = VirtualAlloc((LPVOID)hint, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (p) return p;
        if (hint < start + 0x10000) break;
    }
    for (uintptr_t hint = base; hint <= end; hint += 0x10000) {
        void* p = VirtualAlloc((LPVOID)hint, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (p) return p;
        if (hint > end - 0x10000) break;
    }
    // fallback to default allocation
    return VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
}