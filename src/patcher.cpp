#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include "patcher.hpp"

inline std::vector<uint8_t> patchBytesEx(uintptr_t const address, std::vector<uint8_t> const& bytes, bool hardOverwrite = false) {
    auto hProcess = GetCurrentProcess();
    auto nTarget = reinterpret_cast<LPVOID>(address);
    std::vector<uint8_t> ret(bytes.size());
    if(!ReadProcessMemory(hProcess, nTarget, ret.data(), ret.size(), nullptr)) return {};

    DWORD oldprotect;
    if(hardOverwrite) VirtualProtectEx(hProcess, nTarget, bytes.size(), PAGE_EXECUTE_READWRITE, &oldprotect);
    if(!WriteProcessMemory(hProcess, nTarget, bytes.data(), bytes.size(), nullptr)) return {};
    if(hardOverwrite) VirtualProtectEx(hProcess, nTarget, bytes.size(), oldprotect, &oldprotect);

    return ret;
}

static std::unordered_map<uintptr_t, std::vector<uint8_t>> _patchedBytes;

std::vector<uint8_t> patch(uintptr_t addr, std::vector<uint8_t> bytes, bool hardOverwrite) {
    if(!_patchedBytes[addr].size()) _patchedBytes[addr] = patchBytesEx(addr, bytes, hardOverwrite);
    return _patchedBytes[addr];
}

void unpatch(uintptr_t addr, bool hardOverwrite) {
    if(_patchedBytes[addr].size()) patchBytesEx(addr, _patchedBytes[addr], hardOverwrite);
    _patchedBytes.erase(addr);
}

void swapBytes(uintptr_t addrLeft, uintptr_t addrRight, size_t size, bool hardOverwrite) {
    auto hProcess = GetCurrentProcess();

    auto nTargetLeft = reinterpret_cast<LPVOID>(addrLeft);
    std::vector<uint8_t> bytesLeft(size);
    if(!ReadProcessMemory(hProcess, nTargetLeft, bytesLeft.data(), size, nullptr)) return;

    auto nTargetRight = reinterpret_cast<LPVOID>(addrRight);
    std::vector<uint8_t> bytesRight(size);
    if(!ReadProcessMemory(hProcess, nTargetRight, bytesRight.data(), size, nullptr)) return;

    patch(addrLeft, bytesRight, hardOverwrite);
    patch(addrRight, bytesLeft, hardOverwrite);
}

std::vector<uint8_t> pointerToBytes(uintptr_t ptr) {
    return {
        (unsigned char)(ptr & 0xff),
        (unsigned char)((ptr & 0xff00) >> 0x8),
        (unsigned char)((ptr & 0xff0000) >> 0x10),
        (unsigned char)((ptr & 0xff000000) >> 0x18)
    };
}

std::vector<uint8_t> int32ToBytes(int32_t x) {
    return {
        (unsigned char)(x & 0xff),
        (unsigned char)((x & 0xff00) >> 0x8),
        (unsigned char)((x & 0xff0000) >> 0x10),
        (unsigned char)((x & 0xff000000) >> 0x18)
    };
}
