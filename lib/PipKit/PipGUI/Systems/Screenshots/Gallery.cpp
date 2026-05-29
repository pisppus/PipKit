#include <PipGUI/Systems/Screenshots/Internals.hpp>
#include <PipGUI/Core/Internal/ViewModels.hpp>

#include <algorithm>
#include <cstring>

namespace pipgui
{
    namespace ssd = screenshots::detail;

    void GUI::syncScreenshotGalleryLayout(uint8_t maxShots, uint16_t thumbW, uint16_t thumbH, uint16_t padding)
    {
#if !PIPGUI_SCREENSHOTS
        (void)maxShots;
        (void)thumbW;
        (void)thumbH;
        (void)padding;
        return;
#else
        if (maxShots == 0)
            maxShots = 1;
        if (thumbW == 0 || thumbH == 0)
        {
            thumbW = 64;
            thumbH = 40;
        }
        const bool capacityChanged = (_shots.maxShots != maxShots);
        const bool thumbChanged = (_shots.thumbW != thumbW || _shots.thumbH != thumbH);
        const bool paddingChanged = (_shots.padding != padding);
        if (!capacityChanged && !thumbChanged && !paddingChanged)
            return;

        if (capacityChanged && _shots.entries)
        {
            freeScreenshotGallery(platform());
        }
        else if (thumbChanged && _shots.entries)
        {
            for (uint8_t i = 0; i < _shots.count; ++i)
            {
                if (!_shots.entries[i].pixels)
                    continue;
                detail::free(platform(), _shots.entries[i].pixels);
                _shots.entries[i].pixels = nullptr;
            }
#if (PIPGUI_SCREENSHOT_MODE == 2)
            _shots.flashLoadIndex = 0;
            _shots.flashThumbsDone = (_shots.count == 0);
            _shots.thumbIndexReady = false;
            _shots.thumbIndexW = 0;
            _shots.thumbIndexH = 0;
#endif
        }

        _shots.maxShots = maxShots;
        _shots.thumbW = thumbW;
        _shots.thumbH = thumbH;
        _shots.padding = padding;
#if (PIPGUI_SCREENSHOT_MODE == 2)
        if (capacityChanged)
        {
            _shots.flashLoadIndex = 0;
            _shots.fsDirsReady = false;
            _shots.flashScanDone = false;
            _shots.flashScanActive = false;
            _shots.flashThumbsDone = false;
            _shots.thumbIndexReady = false;
            _shots.thumbIndexW = 0;
            _shots.thumbIndexH = 0;
            if (_shots.scanDir)
                _shots.scanDir.close();
        }
#endif
#endif
    }

    void GUI::freeScreenshotGallery(pipcore::Platform *plat) noexcept
    {
#if (PIPGUI_SCREENSHOT_MODE == 2)
        if (_shots.scanDir)
            _shots.scanDir.close();
        if (_shots.rowBuf)
        {
            detail::free(plat, _shots.rowBuf);
            _shots.rowBuf = nullptr;
            _shots.rowBufSize = 0;
        }
        _shots.flashLoadIndex = 0;
        _shots.flashScanDone = false;
        _shots.flashScanActive = false;
        _shots.flashThumbsDone = false;
        _shots.thumbIndexReady = false;
        _shots.thumbIndexW = 0;
        _shots.thumbIndexH = 0;
#endif
        if (!_shots.entries)
            return;
        for (uint8_t i = 0; i < _shots.maxShots; ++i)
        {
            if (_shots.entries[i].pixels)
                detail::free(plat, _shots.entries[i].pixels);
        }
        detail::free(plat, _shots.entries);
        _shots.entries = nullptr;
        _shots.count = 0;
        _shots.lastUseMs = 0;
    }

    void GUI::releaseScreenshotGalleryCache(pipcore::Platform *plat) noexcept
    {
#if (PIPGUI_SCREENSHOT_MODE == 2)
        if (_shots.scanDir)
            _shots.scanDir.close();
        _shots.flashScanActive = false;
        _shots.flashLoadIndex = 0;
        _shots.flashThumbsDone = (_shots.count == 0);
        if (_shots.rowBuf)
        {
            detail::free(plat, _shots.rowBuf);
            _shots.rowBuf = nullptr;
            _shots.rowBufSize = 0;
        }
#endif
        if (_shots.entries)
        {
            for (uint8_t i = 0; i < _shots.count; ++i)
            {
                if (!_shots.entries[i].pixels)
                    continue;
                detail::free(plat, _shots.entries[i].pixels);
                _shots.entries[i].pixels = nullptr;
            }
        }
        _shots.lastUseMs = 0;
    }

    bool GUI::ensureScreenshotGallery(pipcore::Platform *plat)
    {
        if (_shots.entries)
            return true;
        if (!plat)
            return false;
        void *mem = detail::alloc(plat, sizeof(ScreenshotEntry) * _shots.maxShots, pipcore::AllocCaps::Default);
        if (!mem)
            return false;
        _shots.entries = static_cast<ScreenshotEntry *>(mem);
        for (uint8_t i = 0; i < _shots.maxShots; ++i)
            _shots.entries[i] = {};
        return true;
    }

    void GUI::insertShotToGalleryFrom565(const uint16_t *src, uint16_t w, uint16_t h, uint32_t stamp, const char *path)
    {
#if !PIPGUI_SCREENSHOTS
        (void)src;
        (void)w;
        (void)h;
        (void)stamp;
        (void)path;
        return;
#else
        if (!src || w == 0 || h == 0)
            return;
        if (!ensureScreenshotGallery(platform()))
            return;

        const uint16_t tw = _shots.thumbW;
        const uint16_t th = _shots.thumbH;
        if (tw == 0 || th == 0)
            return;

#if (PIPGUI_SCREENSHOT_MODE == 2)
        if (_shots.scanDir)
            _shots.scanDir.close();
        _shots.flashScanActive = false;
#endif

        const uint8_t cap = _shots.maxShots;
        if (cap == 0)
            return;

        ScreenshotEntry tmp = _shots.entries[cap - 1];
        for (int i = cap - 1; i > 0; --i)
            _shots.entries[i] = _shots.entries[i - 1];
        _shots.entries[0] = tmp;

        ScreenshotEntry &entry = _shots.entries[0];
        const size_t pixels = static_cast<size_t>(tw) * static_cast<size_t>(th);
        if (!entry.pixels)
        {
            entry.pixels = static_cast<uint16_t *>(
                detail::alloc(platform(), pixels * sizeof(uint16_t), pipcore::AllocCaps::Default));
            if (!entry.pixels)
                return;
        }

        const uint32_t srcW16 = static_cast<uint32_t>(w) << 16;
        const uint32_t srcH16 = static_cast<uint32_t>(h) << 16;
        uint32_t scale16 = srcW16 / tw;
        const uint32_t scaleY16 = srcH16 / th;
        if (scaleY16 < scale16)
            scale16 = scaleY16;
        if (scale16 == 0)
            scale16 = 1;

        uint32_t cropW16 = scale16 * tw;
        uint32_t cropH16 = scale16 * th;
        if (cropW16 > srcW16)
            cropW16 = srcW16;
        if (cropH16 > srcH16)
            cropH16 = srcH16;
        const uint32_t cropX16 = (srcW16 > cropW16) ? ((srcW16 - cropW16) / 2u) : 0u;
        const uint32_t cropY16 = (srcH16 > cropH16) ? ((srcH16 - cropH16) / 2u) : 0u;

        for (uint16_t dy = 0; dy < th; ++dy)
        {
            uint32_t y0 = cropY16 + static_cast<uint32_t>(dy) * scale16;
            uint32_t y1 = (dy + 1u == th) ? (cropY16 + cropH16) : (y0 + scale16);
            if (y1 > srcH16)
                y1 = srcH16;
            if (y0 >= y1)
                y0 = (y1 > 0) ? (y1 - 1u) : 0u;
            const uint32_t yLen = y1 - y0;
            const uint16_t sy0 = static_cast<uint16_t>(y0 >> 16);
            const uint16_t sy1 = static_cast<uint16_t>((y1 - 1u) >> 16);

            uint16_t *dstRow = entry.pixels + static_cast<uint32_t>(dy) * tw;
            for (uint16_t dx = 0; dx < tw; ++dx)
            {
                uint32_t x0 = cropX16 + static_cast<uint32_t>(dx) * scale16;
                uint32_t x1 = (dx + 1u == tw) ? (cropX16 + cropW16) : (x0 + scale16);
                if (x1 > srcW16)
                    x1 = srcW16;
                if (x0 >= x1)
                    x0 = (x1 > 0) ? (x1 - 1u) : 0u;
                const uint32_t xLen = x1 - x0;

                uint64_t ar = 0, ag = 0, ab = 0;
                const uint16_t sx0 = static_cast<uint16_t>(x0 >> 16);
                const uint16_t sx1 = static_cast<uint16_t>((x1 - 1u) >> 16);

                for (uint16_t sy = sy0; sy <= sy1; ++sy)
                {
                    const uint32_t wy = ssd::overlap16(y0, y1, static_cast<uint32_t>(sy) << 16);
                    if (!wy)
                        continue;
                    const uint16_t *srcRow = src + static_cast<uint32_t>(sy) * w;
                    for (uint16_t sx = sx0; sx <= sx1; ++sx)
                    {
                        const uint32_t wx = ssd::overlap16(x0, x1, static_cast<uint32_t>(sx) << 16);
                        if (!wx)
                            continue;
                        const uint64_t a = static_cast<uint64_t>(wx) * static_cast<uint64_t>(wy);
                        const uint16_t px = pipcore::Sprite::swap16(srcRow[sx]);
                        ar += static_cast<uint64_t>((px >> 11) & 31u) * a;
                        ag += static_cast<uint64_t>((px >> 5) & 63u) * a;
                        ab += static_cast<uint64_t>(px & 31u) * a;
                    }
                }

                const uint64_t total = static_cast<uint64_t>(xLen) * static_cast<uint64_t>(yLen);
                if (!total)
                {
                    dstRow[dx] = 0;
                    continue;
                }
                const uint64_t half = total / 2u;
                const uint64_t r = (ar + half) / total;
                const uint64_t g = (ag + half) / total;
                const uint64_t b = (ab + half) / total;
                dstRow[dx] = static_cast<uint16_t>((r << 11) | (g << 5) | b);
            }
        }

#if (PIPGUI_SCREENSHOT_MODE == 2)
        entry.stamp = stamp;
        entry.thumbOnFlash = false;
        if (path && path[0])
        {
            std::strncpy(entry.path, path, sizeof(entry.path) - 1);
            entry.path[sizeof(entry.path) - 1] = '\0';
        }
        else
        {
            entry.path[0] = '\0';
        }
#endif
        if (_shots.count < cap)
            ++_shots.count;
#if (PIPGUI_SCREENSHOT_MODE == 2)
        _shots.flashLoadIndex = 0;
        if (_shots.flashScanDone)
        {
            bool allLoaded = true;
            for (uint8_t i = 0; i < _shots.count; ++i)
            {
                if (_shots.entries[i].path[0] != '\0' && !_shots.entries[i].pixels)
                {
                    allLoaded = false;
                    break;
                }
            }
            _shots.flashThumbsDone = allLoaded;
        }
        else
        {
            _shots.flashThumbsDone = false;
        }
#endif
        _flags.needRedraw = 1;
#endif
    }

    void GUI::captureScreenshotToGallery()
    {
#if !PIPGUI_SCREENSHOTS
        return;
#else
        if (!_flags.spriteEnabled || _render.screenWidth == 0 || _render.screenHeight == 0)
            return;
        const uint16_t *src = static_cast<const uint16_t *>(_render.sprite.getBuffer());
        if (!src)
            return;
#if (PIPGUI_SCREENSHOT_MODE == 2)
        return;
#else
        insertShotToGalleryFrom565(src, _render.screenWidth, _render.screenHeight, 0, nullptr);
#endif
#endif
    }
}
