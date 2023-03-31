#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include "extensions2.h"
#include "patcher.hpp"
#include <numeric>

static inline bool mhLoaded() { return GetModuleHandle(TEXT("hackpro.dll")); }

static uintptr_t base = 0;

// this will use 128 MiB of RAM
// 2147483647 / 128 * 8 / 1024 / 1024 = 128
// 16777216 points (or 8388608 lines) should be all you will ever need
static constexpr int32_t maxVertexCount = std::numeric_limits<int32_t>::max() / 128;
int32_t maxConfigVertexCount = maxVertexCount;

// have to do this because the cmp instructions for yellow and green guidelines use the single-byte variants
// meaning the maximum value the limit can have is the maximum signed 1-byte integer, which is 127
// i *could* technically just remove the limit here but it may cause crashes if there are way too many lines somehow
// also keeping this a non-configurable constant because it's low enough anyway
static constexpr int8_t maxLowVertexCount = std::numeric_limits<int8_t>::max();

void resizeArrayStatic(uintptr_t newAddr, uintptr_t loopAddr, uintptr_t memsetAddr, int32_t count) {
    auto size = int32ToBytes(count * sizeof(float) * 2);
    auto num1Lower = int32ToBytes(count - 1);
    patch(base + newAddr + 1, size); // new
    patch(base + loopAddr + 1, num1Lower); // loop
    patch(base + memsetAddr + 1, size); // memset
}

void updateLimits() {
    auto num = int32ToBytes(maxConfigVertexCount);

    // DrawGridLayer::draw
    // grid
    unpatch(base + 0x16d12c + 2);
    unpatch(base + 0x16d354 + 2);
    patch(base + 0x16d12c + 2, num);
    patch(base + 0x16d354 + 2, num);
    // guidelines
    unpatch(base + 0x16d631 + 2);
    unpatch(base + 0x16d6e5 + 2);
    unpatch(base + 0x16d78b + 1);
    patch(base + 0x16d631 + 2, { maxLowVertexCount });
    patch(base + 0x16d6e5 + 2, { maxLowVertexCount });
    patch(base + 0x16d78b + 1, num);
    // effect lines
    unpatch(base + 0x16da15 + 2);
    patch(base + 0x16da15 + 2, num);
    // duration lines
    unpatch(base + 0x16e65b + 2);
    patch(base + 0x16e65b + 2, num);
}

DWORD WINAPI mainThread(void*) {
    base = reinterpret_cast<uintptr_t>(GetModuleHandle(0));

    // DrawGridLayer::init
    // m_commonLines
    resizeArrayStatic(0x16c589, 0x16c5e8, 0x16c604, maxVertexCount);
    // m_yellowGuidelines
    resizeArrayStatic(0x16c61e, 0x16c63b, 0x16c654, maxLowVertexCount);
    // m_greenGuidelines
    resizeArrayStatic(0x16c66e, 0x16c68b, 0x16c6a4, maxLowVertexCount);

    updateLimits();

    if(!mhLoaded())
        return 0;

    auto window = MegaHackExt::Window::Create("Editor Line Limit Bypass");

    auto spinner = MegaHackExt::Spinner::Create("Line limit", "");
    spinner->set(maxConfigVertexCount / 2);
    spinner->setCallback([](MegaHackExt::Spinner* obj, double value) {
        maxConfigVertexCount = static_cast<int32_t>(value) * 2;
        if(maxConfigVertexCount < 0)
            maxConfigVertexCount = 0;
        else if(maxConfigVertexCount > maxVertexCount)
            maxConfigVertexCount = maxVertexCount;
        updateLimits();
    });
    window->add(spinner);

    MegaHackExt::Client::commit(window);

    return 0;
}

BOOL APIENTRY DllMain(HMODULE handle, DWORD reason, LPVOID reserved) {
    if(reason == DLL_PROCESS_ATTACH)
        CreateThread(nullptr, 0x100, mainThread, handle, 0, nullptr);
    return TRUE;
}
