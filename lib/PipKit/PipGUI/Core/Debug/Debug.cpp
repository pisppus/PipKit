#include "Debug.hpp"

#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <Arduino.h>

#include <PipCore/Debug/MemoryHooks.hpp>
#include <PipCore/Graphics/Sprite.hpp>
#include <PipCore/Platform.hpp>
#include <PipCore/Platforms/Select.hpp>
#include <PipGUI/Core/Config/Defaults.hpp>

namespace pipgui
{
    namespace detail
    {
        [[nodiscard]] bool recoverFromAllocFailure(pipcore::Platform *plat, size_t bytes, pipcore::AllocCaps caps) noexcept;
    }

    namespace
    {
        [[nodiscard]] inline uint32_t overdrawByteCount(uint16_t w, uint16_t h) noexcept
        {
            const uint32_t pixels = static_cast<uint32_t>(w) * static_cast<uint32_t>(h);
            return (pixels + 1U) >> 1;
        }

        [[nodiscard]] bool ensureRectCapacity(DirtyRect *&rects, uint16_t &capacity, uint16_t count)
        {
            if (count < capacity)
                return true;

            uint16_t newCap = capacity ? static_cast<uint16_t>(capacity * 2) : 32;
            while (newCap <= count)
            {
                if (newCap >= 0x8000u)
                {
                    newCap = 0xFFFFu;
                    break;
                }
                newCap = static_cast<uint16_t>(newCap * 2u);
            }
            pipcore::Platform *plat = pipcore::GetPlatform();
            if (!plat)
                return false;
            DirtyRect *newRects = static_cast<DirtyRect *>(plat->alloc(sizeof(DirtyRect) * newCap, pipcore::AllocCaps::Default));
            if (!newRects &&
                detail::recoverFromAllocFailure(plat,
                                                sizeof(DirtyRect) * static_cast<size_t>(newCap),
                                                pipcore::AllocCaps::Default))
            {
                newRects = static_cast<DirtyRect *>(plat->alloc(sizeof(DirtyRect) * newCap, pipcore::AllocCaps::Default));
            }
            if (!newRects)
                return false;
            for (uint16_t i = 0; i < count; ++i)
                newRects[i] = rects[i];
            plat->free(rects);
            rects = newRects;
            capacity = newCap;
            return true;
        }

        void appendRect(DirtyRect *&rects, uint16_t &capacity, uint16_t &count, int16_t x, int16_t y, int16_t w, int16_t h)
        {
            if (w <= 0 || h <= 0)
                return;
            if (!ensureRectCapacity(rects, capacity, count))
                return;
            rects[count++] = {x, y, w, h};
        }

        [[nodiscard]] uint16_t readOverdrawCount(const uint8_t *buf, uint32_t index) noexcept
        {
            if (!buf)
                return 0;
            const uint8_t packed = buf[index >> 1];
            return (index & 1U) ? static_cast<uint16_t>((packed >> 4) & 0x0Fu)
                                : static_cast<uint16_t>(packed & 0x0Fu);
        }

        void incrementOverdrawCount(uint8_t *buf, uint32_t index) noexcept
        {
            if (!buf)
                return;
            uint8_t &packed = buf[index >> 1];
            const uint8_t shift = (index & 1U) ? 4U : 0U;
            uint8_t value = static_cast<uint8_t>((packed >> shift) & 0x0Fu);
            if (value < 0x0Fu)
                ++value;
            packed = static_cast<uint8_t>((packed & ~(0x0Fu << shift)) | (value << shift));
        }

        [[nodiscard]] bool intersects(const DirtyRect &r, int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                                      int16_t &ix0, int16_t &iy0, int16_t &ix1, int16_t &iy1) noexcept
        {
            ix0 = (x0 > r.x) ? x0 : r.x;
            iy0 = (y0 > r.y) ? y0 : r.y;
            ix1 = (x1 < (r.x + r.w)) ? x1 : (int16_t)(r.x + r.w);
            iy1 = (y1 < (r.y + r.h)) ? y1 : (int16_t)(r.y + r.h);
            return ix1 > ix0 && iy1 > iy0;
        }

        [[nodiscard]] uint16_t overdrawColor(uint8_t count) noexcept
        {
            if (count >= 4)
                return 0xF800;
            if (count == 3)
                return 0xF81F;
            if (count == 2)
                return 0x07E0;
            return 0x001F;
        }

        [[nodiscard]] uint8_t overdrawAlpha(uint8_t count) noexcept
        {
            if (count >= 4)
                return 192;
            if (count == 3)
                return 168;
            if (count == 2)
                return 136;
            return 112;
        }

        [[nodiscard]] uint16_t blend565(uint16_t dstNative, uint16_t src565, uint8_t alpha) noexcept
        {
            if (alpha == 0)
                return dstNative;
            if (alpha >= 255)
                return pipcore::Sprite::swap16(src565);

            const uint16_t dst565 = pipcore::Sprite::swap16(dstNative);
            const uint32_t inv = 255u - alpha;
            const uint32_t srcRb = ((static_cast<uint32_t>(src565 & 0xF800u) << 5) | (src565 & 0x001Fu));
            const uint32_t srcG = src565 & 0x07E0u;
            const uint32_t dstRb = ((static_cast<uint32_t>(dst565 & 0xF800u) << 5) | (dst565 & 0x001Fu));
            const uint32_t dstG = dst565 & 0x07E0u;
            const uint32_t rb = ((srcRb * alpha + dstRb * inv) >> 8) & 0x001F001Fu;
            const uint32_t g = ((srcG * alpha + dstG * inv) >> 8) & 0x000007E0u;
            return pipcore::Sprite::swap16(static_cast<uint16_t>((rb >> 5) | rb | g));
        }

        void blendRectFill(uint16_t *buf, int16_t stride, const DirtyRect &r,
                           int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                           int16_t bufferY, uint16_t color565, uint8_t alpha)
        {
            int16_t ix0 = 0, iy0 = 0, ix1 = 0, iy1 = 0;
            if (!intersects(r, x0, y0, x1, y1, ix0, iy0, ix1, iy1))
                return;

            for (int16_t y = iy0; y < iy1; ++y)
            {
                uint16_t *row = buf + static_cast<int32_t>(y - bufferY) * stride;
                for (int16_t x = ix0; x < ix1; ++x)
                    row[x] = blend565(row[x], color565, alpha);
            }
        }

        void drawRectOutline(uint16_t *buf, int16_t stride, const DirtyRect &r, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t bufferY, uint16_t col)
        {
            int16_t ix0 = 0, iy0 = 0, ix1 = 0, iy1 = 0;
            if (!intersects(r, x0, y0, x1, y1, ix0, iy0, ix1, iy1))
                return;

            for (int16_t x = ix0; x < ix1; ++x)
            {
                if (iy0 == r.y)
                    buf[(int32_t)(iy0 - bufferY) * stride + x] = col;
                if (iy1 - 1 == r.y + r.h - 1)
                    buf[(int32_t)(iy1 - 1 - bufferY) * stride + x] = col;
            }

            for (int16_t y = iy0; y < iy1; ++y)
            {
                if (ix0 == r.x)
                    buf[(int32_t)(y - bufferY) * stride + ix0] = col;
                if (ix1 - 1 == r.x + r.w - 1)
                    buf[(int32_t)(y - bufferY) * stride + (ix1 - 1)] = col;
            }
        }

        struct MemoryRecord
        {
            const void *ptr = nullptr;
            const char *tag = nullptr;
            uint32_t bytes = 0;
            uint32_t actualBytes = 0;
            uint32_t caps = 0;
            uint32_t seq = 0;
        };

        MemoryRecord *g_memoryRecords = nullptr;
        uint16_t g_memoryRecordCapacity = 0;
        uint16_t g_memoryRecordCount = 0;
        uint32_t g_memorySeq = 0;
        uint32_t g_prevFreeHeapTotal = 0;
        uint32_t g_prevFreeHeapInternal = 0;
        uint32_t g_prevLargestFreeBlock = 0;
        constexpr uint16_t kMemoryLogLineCapacity = 384;
#if PIPGUI_DEBUG_METRICS
        constexpr uint8_t kMemoryLogQueueCapacity = 24;

        struct PendingMemoryLogLine
        {
            uint16_t len = 0;
            char text[kMemoryLogLineCapacity]{};
        };

        PendingMemoryLogLine g_memoryLogQueue[kMemoryLogQueueCapacity];
        uint8_t g_memoryLogHead = 0;
        uint8_t g_memoryLogTail = 0;
        uint8_t g_memoryLogCount = 0;
        uint16_t g_memoryLogTailOffset = 0;
        uint32_t g_memoryLogDropCount = 0;
        bool g_memoryLogIoActive = false;

        void enqueueMemoryLogLine(const char *text, uint16_t len) noexcept
        {
            if (!text || len == 0)
                return;

            if (len >= kMemoryLogLineCapacity)
                len = static_cast<uint16_t>(kMemoryLogLineCapacity - 1);

            if (g_memoryLogCount >= kMemoryLogQueueCapacity)
            {
                g_memoryLogTail = static_cast<uint8_t>((g_memoryLogTail + 1u) % kMemoryLogQueueCapacity);
                g_memoryLogTailOffset = 0;
                --g_memoryLogCount;
                ++g_memoryLogDropCount;
            }

            PendingMemoryLogLine &slot = g_memoryLogQueue[g_memoryLogHead];
            std::memcpy(slot.text, text, len);
            slot.text[len] = '\0';
            slot.len = len;

            g_memoryLogHead = static_cast<uint8_t>((g_memoryLogHead + 1u) % kMemoryLogQueueCapacity);
            ++g_memoryLogCount;
        }

        void enqueueDroppedMemoryLogLine() noexcept
        {
            if (g_memoryLogDropCount == 0 || g_memoryLogCount >= kMemoryLogQueueCapacity)
                return;

            char buf[96];
            const uint32_t dropped = g_memoryLogDropCount;
            const int written = std::snprintf(buf, sizeof(buf),
                                              "[PIPGUI][MEM] dropped=%lu queued-lines due to serial backpressure\n",
                                              static_cast<unsigned long>(dropped));
            if (written <= 0)
                return;

            g_memoryLogDropCount = 0;
            const uint16_t len = static_cast<uint16_t>((written < static_cast<int>(sizeof(buf))) ? written : (sizeof(buf) - 1));
            enqueueMemoryLogLine(buf, len);
        }

        void flushMemoryLogQueue() noexcept
        {
            if (g_memoryLogIoActive)
                return;

            g_memoryLogIoActive = true;
            while (g_memoryLogCount > 0)
            {
                enqueueDroppedMemoryLogLine();

                const size_t budget = Serial.availableForWrite();
                if (budget == 0)
                    break;

                PendingMemoryLogLine &slot = g_memoryLogQueue[g_memoryLogTail];
                if (g_memoryLogTailOffset >= slot.len)
                {
                    slot.len = 0;
                    g_memoryLogTailOffset = 0;
                    g_memoryLogTail = static_cast<uint8_t>((g_memoryLogTail + 1u) % kMemoryLogQueueCapacity);
                    --g_memoryLogCount;
                    continue;
                }

                const size_t remaining = static_cast<size_t>(slot.len - g_memoryLogTailOffset);
                const size_t chunk = (budget < remaining) ? budget : remaining;
                const size_t wrote = Serial.write(reinterpret_cast<const uint8_t *>(slot.text + g_memoryLogTailOffset), chunk);
                if (wrote == 0)
                    break;

                g_memoryLogTailOffset = static_cast<uint16_t>(g_memoryLogTailOffset + wrote);
                if (g_memoryLogTailOffset >= slot.len)
                {
                    slot.len = 0;
                    g_memoryLogTailOffset = 0;
                    g_memoryLogTail = static_cast<uint8_t>((g_memoryLogTail + 1u) % kMemoryLogQueueCapacity);
                    --g_memoryLogCount;
                }
            }
            g_memoryLogIoActive = false;
        }
#endif

        [[nodiscard]] bool ensureMemoryRecordCapacity(uint16_t needed)
        {
            if (needed <= g_memoryRecordCapacity)
                return true;
            uint16_t newCapacity = g_memoryRecordCapacity ? static_cast<uint16_t>(g_memoryRecordCapacity * 2U) : 32U;
            while (newCapacity < needed)
            {
                if (newCapacity >= 0x8000u)
                {
                    newCapacity = 0xFFFFu;
                    break;
                }
                newCapacity = static_cast<uint16_t>(newCapacity * 2u);
            }
            MemoryRecord *newRecords = static_cast<MemoryRecord *>(std::malloc(sizeof(MemoryRecord) * newCapacity));
            if (!newRecords)
                return false;
            if (g_memoryRecords && g_memoryRecordCount > 0)
                std::memcpy(newRecords, g_memoryRecords, sizeof(MemoryRecord) * g_memoryRecordCount);
            std::free(g_memoryRecords);
            g_memoryRecords = newRecords;
            g_memoryRecordCapacity = newCapacity;
            return true;
        }

        MemoryRecord *findMemoryRecord(const void *ptr) noexcept
        {
            if (!ptr)
                return nullptr;
            for (uint16_t i = 0; i < g_memoryRecordCount; ++i)
            {
                if (g_memoryRecords[i].ptr == ptr)
                    return &g_memoryRecords[i];
            }
            return nullptr;
        }

        [[nodiscard]] MemoryRecord removeMemoryRecord(const void *ptr) noexcept
        {
            MemoryRecord removed = {};
            if (!ptr)
                return removed;
            for (uint16_t i = 0; i < g_memoryRecordCount; ++i)
            {
                if (g_memoryRecords[i].ptr != ptr)
                    continue;
                removed = g_memoryRecords[i];
                --g_memoryRecordCount;
                if (i != g_memoryRecordCount)
                    g_memoryRecords[i] = g_memoryRecords[g_memoryRecordCount];
                g_memoryRecords[g_memoryRecordCount] = {};
                return removed;
            }
            return removed;
        }

        void upsertMemoryRecord(const void *ptr, const char *tag, uint32_t bytes, uint32_t actualBytes, uint32_t caps, uint32_t seq) noexcept
        {
            if (!ptr)
                return;
            if (MemoryRecord *existing = findMemoryRecord(ptr))
            {
                existing->tag = tag;
                existing->bytes = bytes;
                existing->actualBytes = actualBytes;
                existing->caps = caps;
                existing->seq = seq;
                return;
            }
            if (!ensureMemoryRecordCapacity(static_cast<uint16_t>(g_memoryRecordCount + 1U)))
                return;
            g_memoryRecords[g_memoryRecordCount++] = {ptr, tag, bytes, actualBytes, caps, seq};
        }

        void printMemoryLine(uint32_t seq,
                             const char *event,
                             const char *tag,
                             const char *ownerTag,
                             const void *ptr,
                             const void *oldPtr,
                             uint32_t requestedBytes,
                             uint32_t actualBytes,
                             uint32_t releasedBytes,
                             uint32_t caps,
                             int32_t deltaFree,
                             int32_t deltaInternal,
                             int32_t deltaLargest,
                             const DebugMetrics &metrics) noexcept
        {
            char line[kMemoryLogLineCapacity];
            int written = 0;
            if (oldPtr)
            {
                written = std::snprintf(line, sizeof(line),
                                        "[PIPGUI][MEM] #%lu %-12s tag=%s owner=%s ptr=%p old=%p req=%u actual=%lu rel=%lu caps=0x%08lx dFree=%ld dIn=%ld dLargest=%ld free=%lu internal=%lu largest=%lu min=%lu live=%lu peakLive=%lu alloc=%lu freeCount=%lu fail=%lu\n",
                                        static_cast<unsigned long>(seq),
                                        event ? event : "?",
                                        tag ? tag : "?",
                                        ownerTag ? ownerTag : "?",
                                        ptr,
                                        oldPtr,
                                        static_cast<unsigned>(requestedBytes),
                                        static_cast<unsigned long>(actualBytes),
                                        static_cast<unsigned long>(releasedBytes),
                                        static_cast<unsigned long>(caps),
                                        static_cast<long>(deltaFree),
                                        static_cast<long>(deltaInternal),
                                        static_cast<long>(deltaLargest),
                                        static_cast<unsigned long>(metrics.freeHeapTotal),
                                        static_cast<unsigned long>(metrics.freeHeapInternal),
                                        static_cast<unsigned long>(metrics.largestFreeBlock),
                                        static_cast<unsigned long>(metrics.minFreeHeap),
                                        static_cast<unsigned long>(metrics.liveAllocBytes),
                                        static_cast<unsigned long>(metrics.peakLiveAllocBytes),
                                        static_cast<unsigned long>(metrics.allocCount),
                                        static_cast<unsigned long>(metrics.freeCount),
                                        static_cast<unsigned long>(metrics.allocFailCount));
            }
            else
            {
                written = std::snprintf(line, sizeof(line),
                                        "[PIPGUI][MEM] #%lu %-12s tag=%s owner=%s ptr=%p req=%u actual=%lu rel=%lu caps=0x%08lx dFree=%ld dIn=%ld dLargest=%ld free=%lu internal=%lu largest=%lu min=%lu live=%lu peakLive=%lu alloc=%lu freeCount=%lu fail=%lu\n",
                                        static_cast<unsigned long>(seq),
                                        event ? event : "?",
                                        tag ? tag : "?",
                                        ownerTag ? ownerTag : "?",
                                        ptr,
                                        static_cast<unsigned>(requestedBytes),
                                        static_cast<unsigned long>(actualBytes),
                                        static_cast<unsigned long>(releasedBytes),
                                        static_cast<unsigned long>(caps),
                                        static_cast<long>(deltaFree),
                                        static_cast<long>(deltaInternal),
                                        static_cast<long>(deltaLargest),
                                        static_cast<unsigned long>(metrics.freeHeapTotal),
                                        static_cast<unsigned long>(metrics.freeHeapInternal),
                                        static_cast<unsigned long>(metrics.largestFreeBlock),
                                        static_cast<unsigned long>(metrics.minFreeHeap),
                                        static_cast<unsigned long>(metrics.liveAllocBytes),
                                        static_cast<unsigned long>(metrics.peakLiveAllocBytes),
                                        static_cast<unsigned long>(metrics.allocCount),
                                        static_cast<unsigned long>(metrics.freeCount),
                                        static_cast<unsigned long>(metrics.allocFailCount));
            }

            if (written <= 0)
                return;

            uint16_t len = static_cast<uint16_t>((written < static_cast<int>(sizeof(line))) ? written : (sizeof(line) - 1));
            if (len > 0 && line[len - 1] != '\n')
            {
                if (len + 1 < sizeof(line))
                    line[len++] = '\n';
                else
                    line[len - 1] = '\n';
            }
#if PIPGUI_DEBUG_METRICS
            enqueueMemoryLogLine(line, len);
#else
            std::fwrite(line, 1, len, stdout);
#endif
        }
    }

    DebugMetrics Debug::_metrics;
    bool Debug::_enabled = false;
    bool Debug::_loggingMemory = false;
    bool Debug::_dirtyRectEnabled = false;
    bool Debug::_overdrawEnabled = false;
    bool Debug::_paintCaptureSuspended = false;
    bool Debug::_layoutBoundsEnabled = false;
    uint16_t Debug::_dirtyRectActiveColor = 0xF81F;
    DirtyRect *Debug::_dirtyRects = nullptr;
    uint16_t Debug::_dirtyRectCapacity = 0;
    uint16_t Debug::_dirtyRectCount = 0;
    DirtyRect *Debug::_layoutRects = nullptr;
    uint16_t Debug::_layoutRectCapacity = 0;
    uint16_t Debug::_layoutRectCount = 0;
    DirtyRect *Debug::_spacingRects = nullptr;
    uint16_t Debug::_spacingRectCapacity = 0;
    uint16_t Debug::_spacingRectCount = 0;
    uint8_t *Debug::_overdrawCounts = nullptr;
    uint32_t Debug::_overdrawCountCapacity = 0;
    uint16_t Debug::_canvasWidth = 0;
    uint16_t Debug::_canvasHeight = 0;

    void Debug::init()
    {
        _enabled = true;
#if PIPGUI_DEBUG_OVERDRAW
        _overdrawEnabled = true;
#endif
#if PIPGUI_DEBUG_LAYOUT_BOUNDS
        _layoutBoundsEnabled = true;
#endif
        update();
        logMemoryEvent("init", "debug.metrics", nullptr, nullptr, 0, 0);
    }

    void Debug::setCanvasSize(int16_t w, int16_t h)
    {
        if (w <= 0 || h <= 0)
        {
            pipcore::Platform *plat = pipcore::GetPlatform();
            if (plat && _overdrawCounts)
                plat->free(_overdrawCounts);
            _overdrawCounts = nullptr;
            _overdrawCountCapacity = 0;
            _canvasWidth = 0;
            _canvasHeight = 0;
            return;
        }

        const uint16_t newW = static_cast<uint16_t>(w);
        const uint16_t newH = static_cast<uint16_t>(h);
        const uint32_t neededBytes = overdrawByteCount(newW, newH);
        if (_overdrawCountCapacity < neededBytes)
        {
            pipcore::Platform *plat = pipcore::GetPlatform();
            if (!plat)
                return;
            uint8_t *newBuf = static_cast<uint8_t *>(plat->alloc(neededBytes, pipcore::AllocCaps::Default));
            if (!newBuf && detail::recoverFromAllocFailure(plat, neededBytes, pipcore::AllocCaps::Default))
                newBuf = static_cast<uint8_t *>(plat->alloc(neededBytes, pipcore::AllocCaps::Default));
            if (!newBuf)
                return;
            if (_overdrawCounts)
                plat->free(_overdrawCounts);
            _overdrawCounts = newBuf;
            _overdrawCountCapacity = neededBytes;
        }

        _canvasWidth = newW;
        _canvasHeight = newH;
        if (_overdrawCounts && _overdrawCountCapacity > 0)
            std::memset(_overdrawCounts, 0, _overdrawCountCapacity);
    }

    void Debug::update()
    {
#if PIPGUI_DEBUG_METRICS
        flushMemoryLogQueue();
#endif
        if (!_enabled)
            return;

        pipcore::Platform *plat = pipcore::GetPlatform();
        if (!plat)
            return;
        _metrics.freeHeapTotal = plat->freeHeapTotal();
        _metrics.freeHeapInternal = plat->freeHeapInternal();
        _metrics.largestFreeBlock = plat->largestFreeBlock();
        _metrics.minFreeHeap = plat->minFreeHeap();

        const uint32_t used = _metrics.freeHeapTotal >= _metrics.minFreeHeap ? (_metrics.freeHeapTotal - _metrics.minFreeHeap) : 0;
        if (used > _metrics.peakUsedHeap)
            _metrics.peakUsedHeap = used;
    }

    void Debug::formatStatusBar(char *out, size_t len)
    {
        if (!_enabled || len == 0)
        {
            if (len > 0)
                out[0] = '\0';
            return;
        }

        int written = snprintf(out, len, "T:%dk In:%dk Bl:%dk Mn:%dk",
                               (int)(_metrics.freeHeapTotal / 1024),
                               (int)(_metrics.freeHeapInternal / 1024),
                               (int)(_metrics.largestFreeBlock / 1024),
                               (int)(_metrics.minFreeHeap / 1024));

        if (written < 0 || (size_t)written >= len)
            out[len - 1] = '\0';
    }

    void Debug::recordPaintRect(int16_t x, int16_t y, int16_t w, int16_t h)
    {
        if (_paintCaptureSuspended)
            return;
        if (_overdrawEnabled && _overdrawCounts && _canvasWidth > 0 && _canvasHeight > 0 && w > 0 && h > 0)
        {
            const int16_t x0 = std::max<int16_t>(0, x);
            const int16_t y0 = std::max<int16_t>(0, y);
            const int16_t x1 = std::min<int16_t>(_canvasWidth, static_cast<int16_t>(x + w));
            const int16_t y1 = std::min<int16_t>(_canvasHeight, static_cast<int16_t>(y + h));
            for (int16_t py = y0; py < y1; ++py)
            {
                const uint32_t rowBase = static_cast<uint32_t>(py) * _canvasWidth;
                for (int16_t px = x0; px < x1; ++px)
                    incrementOverdrawCount(_overdrawCounts, rowBase + static_cast<uint32_t>(px));
            }
        }
    }

    void Debug::recordPaintSpan(int16_t x, int16_t y, int16_t w)
    {
        if (_paintCaptureSuspended)
            return;
        if (!_overdrawEnabled || !_overdrawCounts || _canvasWidth == 0 || _canvasHeight == 0 || w <= 0)
            return;
        if (y < 0 || y >= static_cast<int16_t>(_canvasHeight))
            return;
        const int16_t x0 = std::max<int16_t>(0, x);
        const int16_t x1 = std::min<int16_t>(_canvasWidth, static_cast<int16_t>(x + w));
        if (x1 <= x0)
            return;
        const uint32_t rowBase = static_cast<uint32_t>(y) * _canvasWidth;
        for (int16_t px = x0; px < x1; ++px)
            incrementOverdrawCount(_overdrawCounts, rowBase + static_cast<uint32_t>(px));
    }

    void Debug::recordPaintPixel(int16_t x, int16_t y)
    {
        if (_paintCaptureSuspended)
            return;
        if (!_overdrawEnabled || !_overdrawCounts || _canvasWidth == 0 || _canvasHeight == 0)
            return;
        if (x < 0 || y < 0 || x >= static_cast<int16_t>(_canvasWidth) || y >= static_cast<int16_t>(_canvasHeight))
            return;
        incrementOverdrawCount(_overdrawCounts, static_cast<uint32_t>(y) * _canvasWidth + static_cast<uint32_t>(x));
    }

    void Debug::recordLayoutBounds(int16_t x, int16_t y, int16_t w, int16_t h)
    {
        if (_layoutBoundsEnabled)
            appendRect(_layoutRects, _layoutRectCapacity, _layoutRectCount, x, y, w, h);
    }

    void Debug::recordSpacingBounds(int16_t x, int16_t y, int16_t w, int16_t h)
    {
        if (_layoutBoundsEnabled)
            appendRect(_spacingRects, _spacingRectCapacity, _spacingRectCount, x, y, w, h);
    }

    void Debug::recordDirtyRect(int16_t x, int16_t y, int16_t w, int16_t h)
    {
        if (_dirtyRectEnabled)
            appendRect(_dirtyRects, _dirtyRectCapacity, _dirtyRectCount, x, y, w, h);
    }

    void Debug::drawOverlay(uint16_t *buf, int16_t stride,
                            int16_t x0, int16_t y0, int16_t w, int16_t h,
                            int16_t bufferY)
    {
        if (!buf || stride <= 0 || w <= 0 || h <= 0)
            return;

        const int16_t x1 = (int16_t)(x0 + w);
        const int16_t y1 = (int16_t)(y0 + h);

        if (_overdrawEnabled && _overdrawCounts && _canvasWidth > 0 && _canvasHeight > 0)
        {
            for (int16_t y = y0; y < y1; ++y)
            {
                uint16_t *row = buf + (int32_t)(y - bufferY) * stride;
                for (int16_t x = x0; x < x1; ++x)
                {
                    const uint8_t count = static_cast<uint8_t>(readOverdrawCount(_overdrawCounts, static_cast<uint32_t>(y) * _canvasWidth + static_cast<uint32_t>(x)));
                    if (count > 0)
                        row[x] = blend565(row[x], overdrawColor(count), overdrawAlpha(count));
                }
            }
        }

        if (_dirtyRectEnabled && _dirtyRects && _dirtyRectCount > 0)
        {
            const uint16_t col = pipcore::Sprite::swap16(_dirtyRectActiveColor);
            for (uint16_t i = 0; i < _dirtyRectCount; ++i)
                drawRectOutline(buf, stride, _dirtyRects[i], x0, y0, x1, y1, bufferY, col);
        }

        if (_layoutBoundsEnabled)
        {
            const uint16_t layoutCol = pipcore::Sprite::swap16(0x2D7F);
            const uint16_t spacingCol = pipcore::Sprite::swap16(0xFD20);
            for (uint16_t i = 0; i < _layoutRectCount; ++i)
                drawRectOutline(buf, stride, _layoutRects[i], x0, y0, x1, y1, bufferY, layoutCol);
            for (uint16_t i = 0; i < _spacingRectCount; ++i)
            {
                blendRectFill(buf, stride, _spacingRects[i], x0, y0, x1, y1, bufferY, 0xFD20, 48);
                drawRectOutline(buf, stride, _spacingRects[i], x0, y0, x1, y1, bufferY, spacingCol);
            }
        }
    }

    void Debug::clearRects()
    {
        _dirtyRectCount = 0;
        _layoutRectCount = 0;
        _spacingRectCount = 0;
        if (_overdrawCounts && _overdrawCountCapacity > 0)
            std::memset(_overdrawCounts, 0, _overdrawCountCapacity);
    }

    void Debug::logMemoryEvent(const char *event, const char *tag, const void *ptr, const void *oldPtr, size_t bytes, uint32_t caps)
    {
#if PIPGUI_DEBUG_METRICS
        if (!_enabled || _loggingMemory || g_memoryLogIoActive)
            return;
        _loggingMemory = true;
        update();
        const bool isAlloc = event && std::strcmp(event, "alloc") == 0;
        const bool isAllocFail = event && std::strcmp(event, "alloc-fail") == 0;
        const bool isFree = event && std::strcmp(event, "free") == 0;
        const bool isRealloc = event && std::strcmp(event, "realloc") == 0;
        const bool isReallocFail = event && std::strcmp(event, "realloc-fail") == 0;
        const uint32_t requestedBytes = static_cast<uint32_t>(bytes);

        const uint32_t seq = ++g_memorySeq;
        const uint32_t trackedActualBytes = requestedBytes;
        const bool shouldConsumeOldPtr = oldPtr && (isRealloc || (!ptr && isFree));
        const MemoryRecord oldRecord = shouldConsumeOldPtr ? removeMemoryRecord(oldPtr) : MemoryRecord{};
        const MemoryRecord freedRecord = (isFree || isRealloc) ? removeMemoryRecord(ptr) : MemoryRecord{};
        const uint32_t releasedBytes = freedRecord.actualBytes ? freedRecord.actualBytes
                                                               : (freedRecord.bytes ? freedRecord.bytes : requestedBytes);
        const char *ownerTag = freedRecord.tag ? freedRecord.tag : (oldRecord.tag ? oldRecord.tag : tag);

        if (isAlloc)
            ++_metrics.allocCount;
        else if (isFree)
            ++_metrics.freeCount;
        if (isAllocFail || isReallocFail)
            ++_metrics.allocFailCount;

        if (isAlloc)
        {
            _metrics.liveAllocBytes += trackedActualBytes;
            if (_metrics.liveAllocBytes > _metrics.peakLiveAllocBytes)
                _metrics.peakLiveAllocBytes = _metrics.liveAllocBytes;
            upsertMemoryRecord(ptr, tag, requestedBytes, trackedActualBytes, caps, seq);
        }
        else if (isFree)
        {
            _metrics.liveAllocBytes = (releasedBytes > _metrics.liveAllocBytes) ? 0 : (_metrics.liveAllocBytes - releasedBytes);
        }
        else if (isRealloc)
        {
            _metrics.liveAllocBytes = (releasedBytes > _metrics.liveAllocBytes) ? 0 : (_metrics.liveAllocBytes - releasedBytes);
            _metrics.liveAllocBytes += trackedActualBytes;
            if (_metrics.liveAllocBytes > _metrics.peakLiveAllocBytes)
                _metrics.peakLiveAllocBytes = _metrics.liveAllocBytes;
            upsertMemoryRecord(ptr, tag, requestedBytes, trackedActualBytes, caps, seq);
        }

        const int32_t deltaFree = g_prevFreeHeapTotal ? (static_cast<int32_t>(_metrics.freeHeapTotal) - static_cast<int32_t>(g_prevFreeHeapTotal)) : 0;
        const int32_t deltaInternal = g_prevFreeHeapInternal ? (static_cast<int32_t>(_metrics.freeHeapInternal) - static_cast<int32_t>(g_prevFreeHeapInternal)) : 0;
        const int32_t deltaLargest = g_prevLargestFreeBlock ? (static_cast<int32_t>(_metrics.largestFreeBlock) - static_cast<int32_t>(g_prevLargestFreeBlock)) : 0;
        g_prevFreeHeapTotal = _metrics.freeHeapTotal;
        g_prevFreeHeapInternal = _metrics.freeHeapInternal;
        g_prevLargestFreeBlock = _metrics.largestFreeBlock;

        printMemoryLine(seq, event, tag, ownerTag, ptr, oldPtr,
                        requestedBytes, trackedActualBytes, releasedBytes,
                        caps, deltaFree, deltaInternal, deltaLargest, _metrics);
        flushMemoryLogQueue();
        _loggingMemory = false;
#else
        (void)event;
        (void)tag;
        (void)ptr;
        (void)oldPtr;
        (void)bytes;
        (void)caps;
#endif
    }
}

namespace pipcore::debug
{
    namespace
    {
        void forwardMemoryEvent(MemoryEvent event,
                                const char *tag,
                                void *ptr,
                                void *oldPtr,
                                size_t bytes,
                                uint32_t caps) noexcept
        {
            const char *name = "sample";
            switch (event)
            {
            case MemoryEvent::Alloc:
                name = "alloc";
                break;
            case MemoryEvent::AllocFail:
                name = "alloc-fail";
                break;
            case MemoryEvent::Free:
                name = "free";
                break;
            case MemoryEvent::Realloc:
                name = "realloc";
                break;
            case MemoryEvent::ReallocFail:
                name = "realloc-fail";
                break;
            case MemoryEvent::HeapSample:
                name = "sample";
                break;
            }
            pipgui::Debug::logMemoryEvent(name, tag, ptr, oldPtr, bytes, caps);
        }

        struct MemoryEventRegistrar
        {
            MemoryEventRegistrar() noexcept
            {
                setMemoryEventHandler(&forwardMemoryEvent);
            }
        };

        const MemoryEventRegistrar g_memoryEventRegistrar{};
    }
}
