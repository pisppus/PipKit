#include <PipGUI/Core/Config/Defaults.hpp>

#if PIPGUI_DEBUG_METRICS

#include <PipCore/Debug/MemoryHooks.hpp>

#include <cstddef>
#include <cstdlib>
#include <new>

namespace
{
    [[nodiscard]] void *debugMalloc(size_t bytes, const char *tag)
    {
        void *ptr = std::malloc(bytes);
        pipcore::debug::memoryEvent(ptr ? pipcore::debug::MemoryEvent::Alloc : pipcore::debug::MemoryEvent::AllocFail,
                                    tag, ptr, nullptr, bytes, 0);
        if (!ptr)
        {
#if defined(__cpp_exceptions)
            throw std::bad_alloc();
#else
            std::abort();
#endif
        }
        return ptr;
    }

    void debugFree(void *ptr, const char *tag) noexcept
    {
        if (!ptr)
            return;
        std::free(ptr);
        pipcore::debug::memoryEvent(pipcore::debug::MemoryEvent::Free, tag, ptr, nullptr, 0, 0);
    }
}

void *operator new(size_t bytes)
{
    return debugMalloc(bytes, "global.new");
}

void *operator new[](size_t bytes)
{
    return debugMalloc(bytes, "global.new[]");
}

void *operator new(size_t bytes, const std::nothrow_t &) noexcept
{
    void *ptr = std::malloc(bytes);
    pipcore::debug::memoryEvent(ptr ? pipcore::debug::MemoryEvent::Alloc : pipcore::debug::MemoryEvent::AllocFail,
                                "global.new.nothrow", ptr, nullptr, bytes, 0);
    return ptr;
}

void *operator new[](size_t bytes, const std::nothrow_t &) noexcept
{
    void *ptr = std::malloc(bytes);
    pipcore::debug::memoryEvent(ptr ? pipcore::debug::MemoryEvent::Alloc : pipcore::debug::MemoryEvent::AllocFail,
                                "global.new[].nothrow", ptr, nullptr, bytes, 0);
    return ptr;
}

void operator delete(void *ptr) noexcept
{
    debugFree(ptr, "global.delete");
}

void operator delete[](void *ptr) noexcept
{
    debugFree(ptr, "global.delete[]");
}

void operator delete(void *ptr, size_t) noexcept
{
    debugFree(ptr, "global.delete.sized");
}

void operator delete[](void *ptr, size_t) noexcept
{
    debugFree(ptr, "global.delete[].sized");
}

void operator delete(void *ptr, const std::nothrow_t &) noexcept
{
    debugFree(ptr, "global.delete.nothrow");
}

void operator delete[](void *ptr, const std::nothrow_t &) noexcept
{
    debugFree(ptr, "global.delete[].nothrow");
}

#endif
