#pragma once

#include <cstdint>
#include <cstddef>

namespace pipgui
{

    struct DebugMetrics
    {
        uint32_t freeHeapTotal = 0;
        uint32_t freeHeapInternal = 0;
        uint32_t largestFreeBlock = 0;
        uint32_t minFreeHeap = 0;
        uint32_t peakUsedHeap = 0;
        uint32_t allocCount = 0;
        uint32_t freeCount = 0;
        uint32_t allocFailCount = 0;
        uint32_t liveAllocBytes = 0;
        uint32_t peakLiveAllocBytes = 0;
    };

    struct DirtyRect
    {
        int16_t x, y, w, h;
    };

    class Debug
    {
    public:
        static void init();
        static void update();
        static void setCanvasSize(int16_t w, int16_t h);

        [[nodiscard]] static const DebugMetrics &metrics() noexcept { return _metrics; }

        static void formatStatusBar(char *out, size_t len);

        [[nodiscard]] static bool isEnabled() noexcept { return _enabled; }
        static void setEnabled(bool enable) noexcept { _enabled = enable; }

        static void setDirtyRectEnabled(bool enabled) noexcept { _dirtyRectEnabled = enabled; }
        [[nodiscard]] static bool dirtyRectEnabled() noexcept { return _dirtyRectEnabled; }

        static void setDirtyRectActiveColor(uint16_t color) noexcept { _dirtyRectActiveColor = color; }
        [[nodiscard]] static uint16_t dirtyRectActiveColor() noexcept { return _dirtyRectActiveColor; }

        static void setOverdrawEnabled(bool enabled) noexcept { _overdrawEnabled = enabled; }
        [[nodiscard]] static bool overdrawEnabled() noexcept { return _overdrawEnabled; }
        static void setPaintCaptureSuspended(bool suspended) noexcept { _paintCaptureSuspended = suspended; }
        [[nodiscard]] static bool paintCaptureSuspended() noexcept { return _paintCaptureSuspended; }

        static void setLayoutBoundsEnabled(bool enabled) noexcept { _layoutBoundsEnabled = enabled; }
        [[nodiscard]] static bool layoutBoundsEnabled() noexcept { return _layoutBoundsEnabled; }

        static void recordPaintRect(int16_t x, int16_t y, int16_t w, int16_t h);
        static void recordPaintSpan(int16_t x, int16_t y, int16_t w);
        static void recordPaintPixel(int16_t x, int16_t y);
        static void recordLayoutBounds(int16_t x, int16_t y, int16_t w, int16_t h);
        static void recordSpacingBounds(int16_t x, int16_t y, int16_t w, int16_t h);
        static void recordDirtyRect(int16_t x, int16_t y, int16_t w, int16_t h);

        static void drawOverlay(uint16_t *buf, int16_t stride,
                                int16_t x0, int16_t y0, int16_t w, int16_t h,
                                int16_t bufferY = 0);

        static void clearRects();
        static void logMemoryEvent(const char *event, const char *tag, const void *ptr, const void *oldPtr, size_t bytes, uint32_t caps);

    private:
        static DebugMetrics _metrics;
        static bool _enabled;
        static bool _loggingMemory;

        static bool _dirtyRectEnabled;
        static bool _overdrawEnabled;
        static bool _paintCaptureSuspended;
        static bool _layoutBoundsEnabled;
        static uint16_t _dirtyRectActiveColor;
        static DirtyRect *_dirtyRects;
        static uint16_t _dirtyRectCapacity;
        static uint16_t _dirtyRectCount;
        static DirtyRect *_layoutRects;
        static uint16_t _layoutRectCapacity;
        static uint16_t _layoutRectCount;
        static DirtyRect *_spacingRects;
        static uint16_t _spacingRectCapacity;
        static uint16_t _spacingRectCount;
        static uint8_t *_overdrawCounts;
        static uint32_t _overdrawCountCapacity;
        static uint16_t _canvasWidth;
        static uint16_t _canvasHeight;
    };

}
