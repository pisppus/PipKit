#include <PipGUI/Widgets/Data/Internal.hpp>

namespace pipgui::graph_internal
{
    int16_t graphValueToY(const GraphArea &area, int16_t value, int16_t valueMin, int16_t valueMax) noexcept
    {
        if (valueMax <= valueMin)
            valueMax = valueMin + 1;
        if (value < valueMin)
            value = valueMin;
        else if (value > valueMax)
            value = valueMax;

        const int32_t rangeY = valueMax - valueMin;
        const int32_t heightY = area.innerH - 1;
        return (int16_t)(area.innerY + heightY - ((int32_t)(value - valueMin) * heightY) / rangeY);
    }

    bool canUseIncrementalScrollRender(const GraphArea &area) noexcept
    {
        if (!area.renderSnapshotValid ||
            area.autoScaleEnabled ||
            area.direction == Oscilloscope ||
            area.innerW < 2 ||
            area.sampleCapacity < 2 ||
            area.sampleCapacity != static_cast<uint16_t>(area.innerW) ||
            !area.samples ||
            !area.sampleCounts ||
            !area.sampleHead ||
            !area.renderCounts ||
            !area.renderHead)
            return false;

        bool hasActiveLine = false;
        for (uint16_t line = 0; line < area.lineCount; ++line)
        {
            if (!area.samples[line])
                continue;

            hasActiveLine = true;
            if (area.sampleCounts[line] != area.sampleCapacity || area.renderCounts[line] != area.sampleCapacity)
                return false;

            const uint16_t expectedHead = (uint16_t)((area.renderHead[line] + 1) % area.sampleCapacity);
            if (area.sampleHead[line] != expectedHead)
                return false;
        }

        return hasActiveLine;
    }

    bool shiftGraphInnerOnePixel(pipcore::Sprite *t, const GraphArea &area, int16_t originX, int16_t originY)
    {
        if (!t || !area.innerCache || area.innerW < 2 || area.innerH < 1)
            return false;

        uint16_t *buf = static_cast<uint16_t *>(t->getBuffer());
        const int32_t stride = t->width();
        const int32_t height = t->height();
        if (!buf || stride <= 0 || height <= 0)
            return false;

        const int16_t innerXS = (int16_t)(area.innerX - originX);
        const int16_t innerYS = (int16_t)(area.innerY - originY);

        int32_t clipX = 0;
        int32_t clipY = 0;
        int32_t clipW = stride;
        int32_t clipH = height;
        t->getClipRect(&clipX, &clipY, &clipW, &clipH);
        const int32_t clipR = clipX + clipW - 1;
        const int32_t clipB = clipY + clipH - 1;
        if (innerXS < clipX || innerYS < clipY ||
            innerXS + area.innerW - 1 > clipR ||
            innerYS + area.innerH - 1 > clipB)
            return false;

        const bool leftToRight = (area.direction == LeftToRight);
        for (int16_t y = 0; y < area.innerH; ++y)
        {
            uint16_t *row = buf + static_cast<size_t>(innerYS + y) * stride + innerXS;
            if (leftToRight)
            {
                std::memmove(row, row + 1, static_cast<size_t>(area.innerW - 1) * sizeof(uint16_t));
                row[area.innerW - 1] = area.innerCache[static_cast<size_t>(y) * area.innerW + (area.innerW - 1)];
            }
            else
            {
                std::memmove(row + 1, row, static_cast<size_t>(area.innerW - 1) * sizeof(uint16_t));
                row[0] = area.innerCache[static_cast<size_t>(y) * area.innerW];
            }
        }

        return true;
    }

    void redrawGraphInner(pipcore::Sprite *t, const GraphArea &area, int16_t originX, int16_t originY)
    {
        if (!t || area.innerW <= 0 || area.innerH <= 0)
            return;

        GraphArea &mutableArea = const_cast<GraphArea &>(area);
        if (!mutableArea.innerCache || mutableArea.innerCacheW != area.innerW || mutableArea.innerCacheH != area.innerH)
            buildGraphInnerCache(mutableArea);

        const int16_t innerXS = (int16_t)(area.innerX - originX);
        const int16_t innerYS = (int16_t)(area.innerY - originY);

        if (mutableArea.innerCache)
        {
            uint16_t *buf = static_cast<uint16_t *>(t->getBuffer());
            const int32_t stride = t->width();
            const int32_t height = t->height();
            if (buf && stride > 0 && height > 0)
            {
                int32_t clipX = 0;
                int32_t clipY = 0;
                int32_t clipW = stride;
                int32_t clipH = height;
                t->getClipRect(&clipX, &clipY, &clipW, &clipH);
                const int32_t clipR = clipX + clipW - 1;
                const int32_t clipB = clipY + clipH - 1;
                const int16_t copyX = innerXS < clipX ? static_cast<int16_t>(clipX) : innerXS;
                const int16_t copyY = innerYS < clipY ? static_cast<int16_t>(clipY) : innerYS;
                const int16_t copyR = (innerXS + area.innerW - 1 > clipR) ? static_cast<int16_t>(clipR) : static_cast<int16_t>(innerXS + area.innerW - 1);
                const int16_t copyB = (innerYS + area.innerH - 1 > clipB) ? static_cast<int16_t>(clipB) : static_cast<int16_t>(innerYS + area.innerH - 1);
                if (copyX <= copyR && copyY <= copyB)
                {
                    const int16_t copyW = static_cast<int16_t>(copyR - copyX + 1);
                    const int16_t copyH = static_cast<int16_t>(copyB - copyY + 1);
                    const int16_t srcX = static_cast<int16_t>(copyX - innerXS);
                    const int16_t srcY = static_cast<int16_t>(copyY - innerYS);
                    const size_t rowBytes = static_cast<size_t>(copyW) * sizeof(uint16_t);
                    for (int16_t y = 0; y < copyH; ++y)
                    {
                        uint16_t *dst = buf + static_cast<size_t>(copyY + y) * stride + copyX;
                        const uint16_t *src = mutableArea.innerCache + static_cast<size_t>(srcY + y) * area.innerW + srcX;
                        std::memcpy(dst, src, rowBytes);
                    }
                    return;
                }
            }
        }

        const uint16_t grid565 = deriveGraphGridColor565(area.bgColor565);
        t->fillRect(innerXS, innerYS, area.innerW, area.innerH, area.bgColor565);

        for (uint16_t i = 1; i < area.gridCellsX; ++i)
        {
            const int16_t gx = innerXS + (int16_t)((int32_t)area.innerW * i / area.gridCellsX);
            for (int16_t yy = innerYS; yy < innerYS + area.innerH; ++yy)
                t->drawPixel(gx, yy, grid565);
        }

        for (uint16_t j = 1; j < area.gridCellsY; ++j)
        {
            const int16_t gy = innerYS + (int16_t)((int32_t)area.innerH * j / area.gridCellsY);
            for (int16_t xx = innerXS; xx < innerXS + area.innerW; ++xx)
                t->drawPixel(xx, gy, grid565);
        }
    }

    void renderSeries(GUI &gui,
                      pipcore::Sprite *t,
                      const GraphArea &area,
                      const int16_t *samples,
                      const SeriesWindow &window,
                      uint16_t sampleCapacity,
                      uint16_t line565,
                      int16_t valueMin,
                      int16_t valueMax,
                      uint8_t lineThickness)
    {
        if (!t || !samples || window.count < 2 || window.visible < 2 || sampleCapacity < 2 || area.innerW <= 1 || area.innerH <= 1)
            return;

        if (valueMax <= valueMin)
            valueMax = valueMin + 1;

        const int32_t rangeY = valueMax - valueMin;
        const int32_t heightY = area.innerH - 1;
        const uint32_t stepXFixed = (window.visible > 1) ? (((uint32_t)(area.innerW - 1) << 16) / (window.visible - 1)) : 0;

        auto drawSegment = [&](int16_t x0, int16_t y0, int16_t x1, int16_t y1, bool &drewSegment)
        {
            if (x0 == x1 && y0 == y1)
                return;
            const bool roundStart = !drewSegment;
            detail::GuiAccess::drawLineSegment(gui, x0, y0, x1, y1, lineThickness, line565, roundStart, false);
            drewSegment = true;
        };

        if (area.direction == Oscilloscope && window.visible > static_cast<uint16_t>(area.innerW))
        {
            int16_t prevX = 0;
            int16_t prevY = 0;
            uint32_t currXFixed = 0;
            bool havePrev = false;
            bool drewSegment = false;
            bool haveColumn = false;
            int16_t colX = 0;
            int16_t colFirstY = 0;
            int16_t colLastY = 0;
            int16_t colMinY = 0;
            int16_t colMaxY = 0;

            auto flushColumn = [&]()
            {
                if (!haveColumn)
                    return;
                if (havePrev)
                    drawSegment(prevX, prevY, colX, colFirstY, drewSegment);
                if (colMinY != colMaxY)
                    drawSegment(colX, colMinY, colX, colMaxY, drewSegment);
                prevX = colX;
                prevY = colLastY;
                havePrev = true;
                haveColumn = false;
            };

            for (uint16_t i = 0; i < window.count; ++i)
            {
                const int16_t localX = (int16_t)((currXFixed + 32768U) >> 16);
                currXFixed += stepXFixed;
                const int16_t x = (area.direction == RightToLeft)
                                      ? (int16_t)(area.innerX + area.innerW - 1 - localX)
                                      : (int16_t)(area.innerX + localX);

                uint16_t idx = (uint16_t)(window.start + i);
                if (idx >= sampleCapacity)
                    idx = (uint16_t)(idx % sampleCapacity);

                int16_t v = samples[idx];
                if (v < valueMin)
                    v = valueMin;
                else if (v > valueMax)
                    v = valueMax;

                const int16_t y = (int16_t)(area.innerY + heightY - ((int32_t)(v - valueMin) * heightY) / rangeY);
                if (!haveColumn)
                {
                    colX = x;
                    colFirstY = y;
                    colLastY = y;
                    colMinY = y;
                    colMaxY = y;
                    haveColumn = true;
                    continue;
                }

                if (x == colX)
                {
                    colLastY = y;
                    if (y < colMinY)
                        colMinY = y;
                    if (y > colMaxY)
                        colMaxY = y;
                    continue;
                }

                flushColumn();
                colX = x;
                colFirstY = y;
                colLastY = y;
                colMinY = y;
                colMaxY = y;
                haveColumn = true;
            }

            flushColumn();
            return;
        }

        int16_t prevX = 0;
        int16_t prevY = 0;
        uint32_t currXFixed = 0;
        bool havePrev = false;
        bool drewSegment = false;

        for (uint16_t i = 0; i < window.count; ++i)
        {
            const int16_t localX = (int16_t)((currXFixed + 32768U) >> 16);
            currXFixed += stepXFixed;
            const int16_t x = (area.direction == RightToLeft)
                                  ? (int16_t)(area.innerX + area.innerW - 1 - localX)
                                  : (int16_t)(area.innerX + localX);

            uint16_t idx = (uint16_t)(window.start + i);
            if (idx >= sampleCapacity)
                idx = (uint16_t)(idx % sampleCapacity);

            int16_t v = samples[idx];
            if (v < valueMin)
                v = valueMin;
            else if (v > valueMax)
                v = valueMax;

            const int16_t y = (int16_t)(area.innerY + heightY - ((int32_t)(v - valueMin) * heightY) / rangeY);
            if (havePrev && (x != prevX || y != prevY))
                drawSegment(prevX, prevY, x, y, drewSegment);

            prevX = x;
            prevY = y;
            havePrev = true;
        }
    }

    bool renderBufferedGraphIncremental(GUI &gui, pipcore::Sprite *t, GraphArea &area, bool useAutoScale, int16_t autoMin, int16_t autoMax, int16_t originX, int16_t originY)
    {
        (void)useAutoScale;
        (void)autoMin;
        (void)autoMax;

        if (!shiftGraphInnerOnePixel(t, area, originX, originY))
            return false;

        const bool leftToRight = (area.direction == LeftToRight);
        const int16_t xPrev = leftToRight ? (int16_t)(area.innerX + area.innerW - 2) : (int16_t)(area.innerX + 1);
        const int16_t xCurr = leftToRight ? (int16_t)(area.innerX + area.innerW - 1) : area.innerX;

        for (uint16_t line = 0; line < area.lineCount; ++line)
        {
            if (!area.samples || !area.samples[line] || area.sampleCounts[line] < 2)
                continue;

            const uint16_t currIdx = (uint16_t)((area.sampleHead[line] + area.sampleCapacity - 1) % area.sampleCapacity);
            const uint16_t prevIdx = (uint16_t)((currIdx + area.sampleCapacity - 1) % area.sampleCapacity);
            const int16_t drawMin = area.lineValueMins[line];
            const int16_t drawMax = area.lineValueMaxs[line];
            const int16_t yPrev = graphValueToY(area, area.samples[line][prevIdx], drawMin, drawMax);
            const int16_t yCurr = graphValueToY(area, area.samples[line][currIdx], drawMin, drawMax);
            const uint8_t lineThickness = area.lineThicknesses ? area.lineThicknesses[line] : 1;

            detail::GuiAccess::drawLineSegment(gui, xPrev, yPrev, xCurr, yCurr, lineThickness, area.lineColors565[line], false, false);
        }

        snapshotRenderedGraph(area);
        return true;
    }

    void renderBufferedGraph(GUI &gui, pipcore::Sprite *t, GraphArea &area, uint16_t maxVisible, int16_t originX, int16_t originY)
    {
        bool prevClipEnabled = false;
        int16_t prevClipX = 0;
        int16_t prevClipY = 0;
        int16_t prevClipW = 0;
        int16_t prevClipH = 0;
        detail::GuiAccess::getClipState(gui, prevClipEnabled, prevClipX, prevClipY, prevClipW, prevClipH);
        detail::GuiAccess::setClip(gui, area.innerX, area.innerY, area.innerW, area.innerH);

        int16_t autoMin = 0;
        int16_t autoMax = 1;
        const bool useAutoScale = area.autoScaleEnabled && resolveAutoScale(area, maxVisible, autoMin, autoMax);
        if (!useAutoScale && !gui.tiledMode() && canUseIncrementalScrollRender(area) &&
            renderBufferedGraphIncremental(gui, t, area, useAutoScale, autoMin, autoMax, originX, originY))
        {
            detail::GuiAccess::restoreClipState(gui, prevClipEnabled, prevClipX, prevClipY, prevClipW, prevClipH);
            return;
        }

        redrawGraphInner(t, area, originX, originY);

        for (uint16_t line = 0; line < area.lineCount; ++line)
        {
            if (!area.samples || !area.samples[line])
                continue;

            const SeriesWindow window = resolveSeriesWindow(area, line, maxVisible);
            if (window.count < 2)
                continue;

            const int16_t drawMin = useAutoScale ? autoMin : area.lineValueMins[line];
            const int16_t drawMax = useAutoScale ? autoMax : area.lineValueMaxs[line];
            const uint8_t lineThickness = area.lineThicknesses ? area.lineThicknesses[line] : 1;
            renderSeries(gui, t, area, area.samples[line], window, area.sampleCapacity, area.lineColors565[line], drawMin, drawMax, lineThickness);
        }

        snapshotRenderedGraph(area);
        detail::GuiAccess::restoreClipState(gui, prevClipEnabled, prevClipX, prevClipY, prevClipW, prevClipH);
    }
}
