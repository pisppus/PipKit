#include <PipGUI/Widgets/Data/Internal.hpp>

namespace pipgui::graph_internal
{
    void invalidateGraphRenderCache(GraphArea &area) noexcept
    {
        area.renderCacheValid = false;
        area.renderCacheTileMask = 0;
        if (!area.paused && area.renderCache)
        {
            if (pipcore::Platform *plat = platform())
                detail::free(plat, area.renderCache);
            area.renderCache = nullptr;
            area.renderCacheW = 0;
            area.renderCacheH = 0;
        }
    }

    void freeGraphBuffers(GraphArea &area) noexcept
    {
        pipcore::Platform *plat = platform();
        if (!plat)
            return;

        if (area.samples)
        {
            for (uint16_t i = 0; i < area.lineCount; ++i)
            {
                if (area.samples[i])
                    detail::free(plat, area.samples[i]);
            }
            detail::free(plat, area.samples);
        }

        if (area.lineColors565)
            detail::free(plat, area.lineColors565);
        if (area.lineValueMins)
            detail::free(plat, area.lineValueMins);
        if (area.lineValueMaxs)
            detail::free(plat, area.lineValueMaxs);
        if (area.lineThicknesses)
            detail::free(plat, area.lineThicknesses);
        if (area.sampleCounts)
            detail::free(plat, area.sampleCounts);
        if (area.sampleHead)
            detail::free(plat, area.sampleHead);
        if (area.renderCounts)
            detail::free(plat, area.renderCounts);
        if (area.renderHead)
            detail::free(plat, area.renderHead);
        if (area.innerCache)
            detail::free(plat, area.innerCache);
        if (area.renderCache)
            detail::free(plat, area.renderCache);

        area.samples = nullptr;
        area.lineColors565 = nullptr;
        area.lineValueMins = nullptr;
        area.lineValueMaxs = nullptr;
        area.lineThicknesses = nullptr;
        area.sampleCounts = nullptr;
        area.sampleHead = nullptr;
        area.renderCounts = nullptr;
        area.renderHead = nullptr;
        area.innerCache = nullptr;
        area.innerCacheW = 0;
        area.innerCacheH = 0;
        area.innerCacheDisabled = false;
        area.renderCache = nullptr;
        area.renderCacheW = 0;
        area.renderCacheH = 0;
        area.renderCacheValid = false;
        area.renderCacheTileMask = 0;
        area.renderCacheDisabled = false;
        area.lineCount = 0;
        area.sampleCapacity = 0;
        area.renderSnapshotValid = false;
        area.autoScaleInitialized = false;
        area.lastPeakMs = 0;
        area.drawEpoch = 0;
        area.oscClearEpoch = 0;
        area.x = 0;
        area.y = 0;
        area.w = 0;
        area.h = 0;
        area.innerX = 0;
        area.innerY = 0;
        area.innerW = 0;
        area.innerH = 0;
        area.gridCellsX = 0;
        area.gridCellsY = 0;
        area.frameUsed = false;
        area.pendingRender = false;
        area.paused = false;
        area.pauseToggled = false;
    }

    bool ensureGraphInnerCache(GraphArea &area)
    {
        if (area.innerW <= 0 || area.innerH <= 0)
            return false;
        if (area.innerCacheDisabled)
            return false;
        if (area.innerCache && area.innerCacheW == area.innerW && area.innerCacheH == area.innerH)
            return true;

        pipcore::Platform *plat = platform();
        if (!plat)
            return false;

        if (area.innerCache)
        {
            detail::free(plat, area.innerCache);
            area.innerCache = nullptr;
            area.innerCacheW = 0;
            area.innerCacheH = 0;
        }

        const size_t pixels = static_cast<size_t>(area.innerW) * static_cast<size_t>(area.innerH);
        area.innerCache = (uint16_t *)detail::alloc(plat, pixels * sizeof(uint16_t), pipcore::AllocCaps::Default);
        if (!area.innerCache)
        {
            area.innerCacheDisabled = true;
            return false;
        }

        area.innerCacheW = area.innerW;
        area.innerCacheH = area.innerH;
        area.innerCacheDisabled = false;
        return true;
    }

    bool ensureGraphRenderCache(GraphArea &area)
    {
        if (!area.paused || area.innerW <= 0 || area.innerH <= 0)
            return false;
        if (area.renderCacheDisabled)
            return false;
        if (area.renderCache && area.renderCacheW == area.innerW && area.renderCacheH == area.innerH)
            return true;

        pipcore::Platform *plat = platform();
        if (!plat)
            return false;

        if (area.renderCache)
        {
            detail::free(plat, area.renderCache);
            area.renderCache = nullptr;
            area.renderCacheW = 0;
            area.renderCacheH = 0;
            area.renderCacheValid = false;
            area.renderCacheTileMask = 0;
        }

        const size_t pixels = static_cast<size_t>(area.innerW) * static_cast<size_t>(area.innerH);
        const size_t bytes = pixels * sizeof(uint16_t);
        if (bytes > kGraphRenderCacheMaxBytes)
        {
            area.renderCacheDisabled = true;
            return false;
        }

        area.renderCache = (uint16_t *)detail::alloc(plat, bytes, pipcore::AllocCaps::Default);
        if (!area.renderCache)
        {
            area.renderCacheDisabled = true;
            return false;
        }

        area.renderCacheW = area.innerW;
        area.renderCacheH = area.innerH;
        area.renderCacheValid = false;
        area.renderCacheTileMask = 0;
        area.renderCacheDisabled = false;
        return true;
    }

    void snapshotGraphRenderCache(pipcore::Sprite *t, GraphArea &area, int16_t originX, int16_t originY, int16_t screenH) noexcept
    {
        if (!area.paused || !t || area.innerW <= 0 || area.innerH <= 0 || screenH <= 0)
            return;
        if (!ensureGraphRenderCache(area) || !area.renderCache)
            return;

        uint16_t *buf = static_cast<uint16_t *>(t->getBuffer());
        const int32_t stride = t->width();
        const int16_t tileW = t->width();
        const int16_t tileH = t->height();
        if (!buf || stride <= 0 || tileW <= 0 || tileH <= 0)
            return;

        const bool tiled = (screenH > tileH);
        const uint8_t tileBit = tiled ? ((originY >= tileH) ? 0x2 : 0x1) : 0x1;

        uint8_t requiredMask = 0x1;
        if (tiled)
        {
            requiredMask = 0;
            const int32_t iy1 = area.innerY;
            const int32_t iy2 = (int32_t)area.innerY + area.innerH;
            if (iy1 < tileH && iy2 > 0)
                requiredMask |= 0x1;
            if (iy1 < screenH && iy2 > tileH)
                requiredMask |= 0x2;
            if (requiredMask == 0)
                requiredMask = 0x1;
        }

        const int32_t tileX1 = originX;
        const int32_t tileY1 = originY;
        const int32_t tileX2 = (int32_t)originX + tileW;
        const int32_t tileY2 = (int32_t)originY + tileH;

        const int32_t innerX1 = area.innerX;
        const int32_t innerY1 = area.innerY;
        const int32_t innerX2 = (int32_t)area.innerX + area.innerW;
        const int32_t innerY2 = (int32_t)area.innerY + area.innerH;

        const int32_t ix1 = std::max<int32_t>(innerX1, tileX1);
        const int32_t iy1 = std::max<int32_t>(innerY1, tileY1);
        const int32_t ix2 = std::min<int32_t>(innerX2, tileX2);
        const int32_t iy2 = std::min<int32_t>(innerY2, tileY2);

        if (ix2 <= ix1 || iy2 <= iy1)
        {
            if ((requiredMask & tileBit) == 0)
                area.renderCacheTileMask |= tileBit;
            area.renderCacheValid = ((area.renderCacheTileMask & requiredMask) == requiredMask);
            return;
        }

        const int16_t copyW = (int16_t)(ix2 - ix1);
        const int16_t copyH = (int16_t)(iy2 - iy1);
        const int32_t srcX = ix1 - tileX1;
        const int32_t srcY = iy1 - tileY1;
        const int32_t dstX = ix1 - innerX1;
        const int32_t dstY = iy1 - innerY1;
        if (srcX < 0 || srcY < 0 || dstX < 0 || dstY < 0)
            return;

        const size_t rowBytes = static_cast<size_t>(copyW) * sizeof(uint16_t);
        for (int16_t row = 0; row < copyH; ++row)
        {
            const uint16_t *src = buf + static_cast<size_t>(srcY + row) * stride + srcX;
            uint16_t *dst = area.renderCache + static_cast<size_t>(dstY + row) * area.innerW + dstX;
            std::memcpy(dst, src, rowBytes);
        }

        area.renderCacheTileMask |= tileBit;
        area.renderCacheValid = ((area.renderCacheTileMask & requiredMask) == requiredMask);
    }

    bool blitGraphRenderCache(pipcore::Sprite *t, const GraphArea &area, int16_t originX, int16_t originY) noexcept
    {
        if (!t || !area.renderCacheValid || !area.renderCache)
            return false;
        if (area.renderCacheW != area.innerW || area.renderCacheH != area.innerH)
            return false;
        if (area.innerW <= 0 || area.innerH <= 0)
            return false;

        uint16_t *buf = static_cast<uint16_t *>(t->getBuffer());
        const int32_t stride = t->width();
        const int16_t tileW = t->width();
        const int16_t tileH = t->height();
        if (!buf || stride <= 0 || tileW <= 0 || tileH <= 0)
            return false;

        const int32_t tileX1 = originX;
        const int32_t tileY1 = originY;
        const int32_t tileX2 = (int32_t)originX + tileW;
        const int32_t tileY2 = (int32_t)originY + tileH;

        const int32_t innerX1 = area.innerX;
        const int32_t innerY1 = area.innerY;
        const int32_t innerX2 = (int32_t)area.innerX + area.innerW;
        const int32_t innerY2 = (int32_t)area.innerY + area.innerH;

        const int32_t ix1 = std::max<int32_t>(innerX1, tileX1);
        const int32_t iy1 = std::max<int32_t>(innerY1, tileY1);
        const int32_t ix2 = std::min<int32_t>(innerX2, tileX2);
        const int32_t iy2 = std::min<int32_t>(innerY2, tileY2);

        if (ix2 <= ix1 || iy2 <= iy1)
            return true;

        const int16_t copyW = (int16_t)(ix2 - ix1);
        const int16_t copyH = (int16_t)(iy2 - iy1);
        const int32_t dstX = ix1 - tileX1;
        const int32_t dstY = iy1 - tileY1;
        const int32_t srcX = ix1 - innerX1;
        const int32_t srcY = iy1 - innerY1;
        if (dstX < 0 || dstY < 0 || srcX < 0 || srcY < 0)
            return false;

        const size_t rowBytes = static_cast<size_t>(copyW) * sizeof(uint16_t);
        for (int16_t row = 0; row < copyH; ++row)
        {
            uint16_t *dst = buf + static_cast<size_t>(dstY + row) * stride + dstX;
            const uint16_t *src = area.renderCache + static_cast<size_t>(srcY + row) * area.innerW + srcX;
            std::memcpy(dst, src, rowBytes);
        }
        return true;
    }

    void buildGraphInnerCache(GraphArea &area)
    {
        if (!ensureGraphInnerCache(area) || !area.innerCache)
            return;

        const uint16_t bg = pipcore::Sprite::swap16(area.bgColor565);
        const uint16_t grid = pipcore::Sprite::swap16(deriveGraphGridColor565(area.bgColor565));

        for (int16_t y = 0; y < area.innerH; ++y)
        {
            uint16_t *row = area.innerCache + static_cast<size_t>(y) * area.innerW;
            for (int16_t x = 0; x < area.innerW; ++x)
                row[x] = bg;
        }

        for (uint16_t i = 1; i < area.gridCellsX; ++i)
        {
            const int16_t gx = (int16_t)((int32_t)area.innerW * i / area.gridCellsX);
            if (gx <= 0 || gx >= area.innerW)
                continue;
            for (int16_t y = 0; y < area.innerH; ++y)
                area.innerCache[static_cast<size_t>(y) * area.innerW + gx] = grid;
        }

        for (uint16_t j = 1; j < area.gridCellsY; ++j)
        {
            const int16_t gy = (int16_t)((int32_t)area.innerH * j / area.gridCellsY);
            if (gy <= 0 || gy >= area.innerH)
                continue;
            uint16_t *row = area.innerCache + static_cast<size_t>(gy) * area.innerW;
            for (int16_t x = 0; x < area.innerW; ++x)
                row[x] = grid;
        }
    }

    void snapshotRenderedGraph(GraphArea &area) noexcept
    {
        if (!area.renderCounts || !area.renderHead || !area.sampleCounts || !area.sampleHead)
        {
            area.renderSnapshotValid = false;
            return;
        }

        for (uint16_t i = 0; i < area.lineCount; ++i)
        {
            area.renderCounts[i] = area.sampleCounts[i];
            area.renderHead[i] = area.sampleHead[i];
        }
        area.renderSnapshotValid = true;
    }
}
