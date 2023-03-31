#pragma once

#include <vector>
#include <unordered_map>

std::vector<uint8_t> patch(uintptr_t addr, std::vector<uint8_t> bytes, bool hardOverwrite = false);
void unpatch(uintptr_t addr, bool hardOverwrite = false);
void swapBytes(uintptr_t addrLeft, uintptr_t addrRight, size_t size, bool hardOverwrite = false);

std::vector<uint8_t> pointerToBytes(uintptr_t ptr);
std::vector<uint8_t> int32ToBytes(int32_t x);
