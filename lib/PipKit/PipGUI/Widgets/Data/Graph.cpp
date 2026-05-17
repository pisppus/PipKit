#include <PipGUI/Widgets/Data/Internal.hpp>

namespace pipgui
{
    void GUI::beginGraphFrame(uint8_t screenId) noexcept
    {
        if (screenId >= _screen.capacity || !_screen.graphAreas)
            return;

        GraphArea *area = _screen.graphAreas[screenId];
        if (area)
            area->frameUsed = false;
    }

    void GUI::flushPendingGraphRender(uint8_t screenId) noexcept
    {
        if (!_flags.spriteEnabled || !_disp.display || screenId >= _screen.capacity || !_screen.graphAreas)
            return;

        GraphArea *area = _screen.graphAreas[screenId];
        if (!area || !area->pendingRender || area->innerW <= 1 || area->innerH <= 1)
            return;

        if (_flags.tiledMode)
        {
            uint16_t visibleSamples = (uint16_t)((area->innerW > 2) ? area->innerW : 2);
            if (area->direction == Oscilloscope)
                visibleSamples = graph_internal::resolveOscilloscopeVisibleSamples(*area, visibleSamples);

            tiledRenderAndPresentRect(area->innerX, area->innerY,
                                      area->innerW, area->innerH,
                                      "tiled-flush-graph",
                                      [&]()
                                      {
                                          graph_internal::renderBufferedGraph(*this, &_render.sprite, *area, visibleSamples, _render.originX, _render.originY);
                                          if (area->paused)
                                              graph_internal::snapshotGraphRenderCache(&_render.sprite, *area, _render.originX, _render.originY, (int16_t)_render.screenHeight);
                                      });
            area->pendingRender = false;
            return;
        }

        const bool prevRender = _flags.inSpritePass;
        pipcore::Sprite *prevActive = _render.activeSprite;
        _flags.inSpritePass = 1;
        _render.activeSprite = &_render.sprite;

        uint16_t visibleSamples = (uint16_t)((area->innerW > 2) ? area->innerW : 2);
        if (area->direction == Oscilloscope)
            visibleSamples = graph_internal::resolveOscilloscopeVisibleSamples(*area, visibleSamples);

        graph_internal::renderBufferedGraph(*this, &_render.sprite, *area, visibleSamples, _render.originX, _render.originY);
        if (area->paused)
            graph_internal::snapshotGraphRenderCache(&_render.sprite, *area, _render.originX, _render.originY, (int16_t)_render.screenHeight);

        _flags.inSpritePass = prevRender;
        _render.activeSprite = prevActive;
        area->pendingRender = false;

        invalidateRect(area->innerX, area->innerY, area->innerW, area->innerH);
    }

    void GUI::endGraphFrame(uint8_t screenId) noexcept
    {
        if (screenId >= _screen.capacity || !_screen.graphAreas)
            return;

        GraphArea *area = _screen.graphAreas[screenId];
        if (area && !area->frameUsed)
            graph_internal::freeGraphBuffers(*area);
    }

    void GUI::releaseGraphBuffers(uint8_t screenId) noexcept
    {
        if (screenId >= _screen.capacity || !_screen.graphAreas)
            return;

        GraphArea *area = _screen.graphAreas[screenId];
        if (area)
            graph_internal::freeGraphBuffers(*area);
    }

    void GUI::drawGraphGrid(int16_t x, int16_t y,
                            int16_t w, int16_t h,
                            uint8_t radius,
                            GraphDirection dir,
                            uint32_t bgColor,
                            float speed,
                            bool autoScale,
                            uint16_t scopeRateHz,
                            uint16_t scopeTimebaseMs,
                            uint16_t scopeVisibleSamples)
    {
        if (w <= 0 || h <= 0)
            return;

        if (_flags.spriteEnabled && _disp.display && !_flags.inSpritePass)
        {
            updateGraphGrid(x, y, w, h, radius, dir, bgColor, speed, autoScale,
                            scopeRateHz, scopeTimebaseMs, scopeVisibleSamples);
            return;
        }

        const uint8_t screenId = (_screen.current < _screen.capacity) ? _screen.current : 0;
        GraphArea *area = ensureGraphArea(screenId);
        pipcore::Sprite *t = getDrawTarget();
        if (!area || !t)
            return;

        area->frameUsed = true;

        if (x == center)
        {
            int16_t availW = _render.screenWidth;
            if (availW < w)
                availW = w;
            x = (int16_t)((availW - w) / 2);
        }

        if (y == center)
        {
            int16_t top = 0;
            int16_t availH = _render.screenHeight;
            const int16_t sb = statusBarHeight();
            if (_flags.statusBarEnabled && sb > 0)
            {
                if (_status.pos == Top)
                {
                    top += sb;
                    availH -= sb;
                }
                else if (_status.pos == Bottom)
                {
                    availH -= sb;
                }
            }
            if (availH < h)
                availH = h;
            y = (int16_t)(top + (availH - h) / 2);
        }

        const uint16_t bg565 = detail::color888To565(bgColor);
        const uint8_t resolvedRadius = radius ? radius : graph_internal::kDefaultGraphRadius;
        const bool graphChanged =
            area->x != x || area->y != y ||
            area->w != w || area->h != h ||
            area->radius != resolvedRadius ||
            area->direction != dir ||
            area->bgColor != bgColor;
        const bool scaleModeChanged = area->autoScaleEnabled != autoScale;
        const bool scopeConfigChanged =
            area->oscSampleRateHz != scopeRateHz ||
            area->oscTimebaseMs != scopeTimebaseMs ||
            area->oscVisibleSamples != scopeVisibleSamples;

        const uint8_t outerRadius = resolvedRadius;
        drawSquircleRect().pos(x, y).size(w, h).radius(outerRadius).fill(graph_internal::deriveGraphGridColor565(bg565));

        const int16_t innerX = (int16_t)(x + 2);
        const int16_t innerY = (int16_t)(y + 2);
        const int16_t innerW = (w > 4) ? (int16_t)(w - 4) : 0;
        const int16_t innerH = (h > 4) ? (int16_t)(h - 4) : 0;

        if (graphChanged)
        {
            const bool keepAutoScale = area->autoScaleEnabled;
            graph_internal::freeGraphBuffers(*area);
            area->autoScaleEnabled = autoScale;
            if (keepAutoScale != autoScale)
                area->autoScaleInitialized = false;
        }
        else
        {
            if (scaleModeChanged)
                area->autoScaleInitialized = false;
            if (scaleModeChanged || scopeConfigChanged)
                graph_internal::invalidateGraphRenderCache(*area);
        }

        area->x = x;
        area->y = y;
        area->w = w;
        area->h = h;
        area->innerX = innerX;
        area->innerY = innerY;
        area->innerW = innerW;
        area->innerH = innerH;
        area->radius = resolvedRadius;
        area->direction = dir;
        area->speed = speed;
        area->autoScaleEnabled = autoScale;
        area->oscSampleRateHz = scopeRateHz;
        area->oscTimebaseMs = scopeTimebaseMs;
        area->oscVisibleSamples = scopeVisibleSamples;
        area->bgColor = bgColor;
        area->bgColor565 = bg565;
        bool bumpEpoch = true;
        if (_flags.tiledMode && _flags.inSpritePass)
        {
            const int16_t tileH = _render.sprite.height();
            if (tileH > 0)
            {
                const int16_t firstTileY = static_cast<int16_t>((innerY / tileH) * tileH);
                bumpEpoch = (_render.originY == firstTileY);
            }
        }
        if (bumpEpoch)
            area->drawEpoch = (area->drawEpoch == 0xFFFFFFFFU) ? 1U : (area->drawEpoch + 1U);

        if (innerW <= 0 || innerH <= 0)
            return;

        const int16_t innerRadius = (outerRadius > 2) ? (int16_t)(outerRadius - 2) : (outerRadius > 0 ? (int16_t)(outerRadius - 1) : 0);
        drawSquircleRect().pos(innerX, innerY).size(innerW, innerH).radius((uint8_t)innerRadius).fill(bg565);

        int16_t cellsX = (int16_t)((float)innerW / 12.0f + 0.5f);
        int16_t cellsY = (int16_t)((float)innerH / 12.0f + 0.5f);
        if (cellsX < 3)
            cellsX = 3;
        if (cellsY < 3)
            cellsY = 3;
        area->gridCellsX = (cellsX > 255) ? 255 : (uint8_t)cellsX;
        area->gridCellsY = (cellsY > 255) ? 255 : (uint8_t)cellsY;

        graph_internal::redrawGraphInner(t, *area, _render.originX, _render.originY);
    }

    void GUI::updateGraphGrid(int16_t x, int16_t y,
                              int16_t w, int16_t h,
                              uint8_t radius,
                              GraphDirection dir,
                              uint32_t bgColor,
                              float speed,
                              bool autoScale,
                              uint16_t scopeRateHz,
                              uint16_t scopeTimebaseMs,
                              uint16_t scopeVisibleSamples)
    {
        if (!_flags.spriteEnabled || !_disp.display)
        {
            drawGraphGrid(x, y, w, h, radius, dir, bgColor, speed, autoScale,
                          scopeRateHz, scopeTimebaseMs, scopeVisibleSamples);
            return;
        }

        int16_t rx = x;
        int16_t ry = y;
        if (rx == center)
            rx = AutoX(w);
        if (ry == center)
            ry = AutoY(h);

        if (_flags.tiledMode && !_flags.inSpritePass)
        {
            tiledRenderAndPresentRect((int16_t)(rx - 2), (int16_t)(ry - 2), (int16_t)(w + 4), (int16_t)(h + 4),
                                      "tiled-update-graph-grid",
                                      [&]()
                                      { drawGraphGrid(x, y, w, h, radius, dir, bgColor, speed, autoScale, scopeRateHz, scopeTimebaseMs, scopeVisibleSamples); });
            return;
        }

        const bool prevRender = _flags.inSpritePass;
        pipcore::Sprite *prevActive = _render.activeSprite;

        _flags.inSpritePass = 1;
        _render.activeSprite = &_render.sprite;
        drawGraphGrid(x, y, w, h, radius, dir, bgColor, speed, autoScale,
                      scopeRateHz, scopeTimebaseMs, scopeVisibleSamples);
        _flags.inSpritePass = prevRender;
        _render.activeSprite = prevActive;

        if (prevRender || _screen.current >= _screen.capacity)
            return;

        GraphArea *area = ensureGraphArea(_screen.current);
        if (!area)
            return;

        invalidateRect((int16_t)(area->x - 2), (int16_t)(area->y - 2), (int16_t)(area->w + 4), (int16_t)(area->h + 4));
    }

    void GUI::drawGraphLine(uint8_t lineIndex,
                            int16_t value,
                            uint32_t color,
                            int16_t valueMin,
                            int16_t valueMax,
                            uint8_t thickness)
    {
        if (_flags.spriteEnabled && _disp.display && !_flags.inSpritePass)
        {
            updateGraphLine(lineIndex, value, color, valueMin, valueMax, thickness);
            return;
        }
        if (_screen.current >= _screen.capacity)
            return;

        GraphArea *area = ensureGraphArea(_screen.current);
        if (!area || area->innerW <= 1 || area->innerH <= 1)
            return;

        area->frameUsed = true;

        pipcore::Sprite *t = getDrawTarget();
        if (!t)
            return;

        uint16_t visibleSamples = (uint16_t)((area->innerW > 2) ? area->innerW : 2);
        if (area->direction == Oscilloscope)
            visibleSamples = graph_internal::resolveOscilloscopeVisibleSamples(*area, visibleSamples);

        if (!graph_internal::ensureGraphLineStorage(*area, lineIndex) ||
            !graph_internal::ensureGraphSampleCapacity(*area, visibleSamples) ||
            !graph_internal::ensureGraphLineBuffer(*area, lineIndex))
            return;

        if (graphPaused())
        {
            if (graph_internal::blitGraphRenderCache(t, *area, _render.originX, _render.originY))
                return;
            graph_internal::renderBufferedGraph(*this, t, *area, visibleSamples, _render.originX, _render.originY);
            area->pendingRender = false;
            graph_internal::snapshotGraphRenderCache(t, *area, _render.originX, _render.originY, (int16_t)_render.screenHeight);
            return;
        }

        area->lineColors565[lineIndex] = detail::color888To565(color);
        area->lineValueMins[lineIndex] = valueMin;
        area->lineValueMaxs[lineIndex] = (valueMax > valueMin) ? valueMax : (int16_t)(valueMin + 1);
        area->lineThicknesses[lineIndex] = thickness < 1 ? 1 : thickness;

        bool appendSample = true;
        if (_flags.tiledMode && _flags.inSpritePass)
        {
            const int16_t tileH = _render.sprite.height();
            if (tileH > 0)
            {
                const int16_t firstTileY = static_cast<int16_t>((area->innerY / tileH) * tileH);
                appendSample = (_render.originY == firstTileY);
            }
        }
        if (appendSample)
            graph_internal::appendGraphSample(*area, lineIndex, value, visibleSamples);

        graph_internal::invalidateGraphRenderCache(*area);
        graph_internal::renderBufferedGraph(*this, t, *area, visibleSamples, _render.originX, _render.originY);
        area->pendingRender = false;
    }

    void GUI::updateGraphLine(uint8_t lineIndex,
                              int16_t value,
                              uint32_t color,
                              int16_t valueMin,
                              int16_t valueMax,
                              uint8_t thickness)
    {
        if (!_flags.spriteEnabled || !_disp.display)
        {
            drawGraphLine(lineIndex, value, color, valueMin, valueMax, thickness);
            return;
        }
        if (_screen.current >= _screen.capacity)
            return;

        if (_flags.tiledMode && !_flags.inSpritePass)
        {
            GraphArea *area = ensureGraphArea(_screen.current);
            if (!area || area->innerW <= 1 || area->innerH <= 1)
                return;

            uint16_t visibleSamples = (uint16_t)((area->innerW > 2) ? area->innerW : 2);
            if (area->direction == Oscilloscope)
                visibleSamples = graph_internal::resolveOscilloscopeVisibleSamples(*area, visibleSamples);

            if (!graph_internal::ensureGraphLineStorage(*area, lineIndex) ||
                !graph_internal::ensureGraphSampleCapacity(*area, visibleSamples) ||
                !graph_internal::ensureGraphLineBuffer(*area, lineIndex))
                return;

            if (graphPaused())
            {
                area->pendingRender = false;
                return;
            }

            area->lineColors565[lineIndex] = detail::color888To565(color);
            area->lineValueMins[lineIndex] = valueMin;
            area->lineValueMaxs[lineIndex] = (valueMax > valueMin) ? valueMax : (int16_t)(valueMin + 1);
            area->lineThicknesses[lineIndex] = thickness < 1 ? 1 : thickness;
            graph_internal::appendGraphSample(*area, lineIndex, value, visibleSamples);
            graph_internal::invalidateGraphRenderCache(*area);
            area->pendingRender = true;
            return;
        }

        const bool prevRender = _flags.inSpritePass;
        pipcore::Sprite *prevActive = _render.activeSprite;
        _flags.inSpritePass = 1;
        _render.activeSprite = &_render.sprite;
        drawGraphLine(lineIndex, value, color, valueMin, valueMax, thickness);
        _flags.inSpritePass = prevRender;
        _render.activeSprite = prevActive;

        GraphArea *area = ensureGraphArea(_screen.current);
        if (area && !prevRender && !graphPaused())
            area->pendingRender = true;
    }

    void GUI::drawGraphSamples(uint8_t lineIndex,
                               const int16_t *samples,
                               uint16_t sampleCount,
                               uint32_t color,
                               int16_t valueMin,
                               int16_t valueMax,
                               uint8_t thickness)
    {
        if (_flags.spriteEnabled && _disp.display && !_flags.inSpritePass)
        {
            updateGraphSamples(lineIndex, samples, sampleCount, color, valueMin, valueMax, thickness);
            return;
        }
        if (_screen.current >= _screen.capacity || !samples || sampleCount < 2)
            return;

        GraphArea *area = ensureGraphArea(_screen.current);
        if (!area || area->innerW <= 1 || area->innerH <= 1)
            return;

        area->frameUsed = true;

        pipcore::Sprite *t = getDrawTarget();
        if (!t)
            return;

        if (!graphPaused())
            graph_internal::invalidateGraphRenderCache(*area);

        if (lineIndex == 0 || area->oscClearEpoch != area->drawEpoch)
        {
            area->oscClearEpoch = area->drawEpoch;
            graph_internal::redrawGraphInner(t, *area, _render.originX, _render.originY);
        }

        const graph_internal::SeriesWindow window{0, sampleCount, sampleCount};
        graph_internal::renderSeries(*this, t, *area, samples, window, sampleCount, detail::color888To565(color), valueMin, valueMax, thickness);
    }

    void GUI::updateGraphSamples(uint8_t lineIndex,
                                 const int16_t *samples,
                                 uint16_t sampleCount,
                                 uint32_t color,
                                 int16_t valueMin,
                                 int16_t valueMax,
                                 uint8_t thickness)
    {
        if (!_flags.spriteEnabled || !_disp.display)
        {
            drawGraphSamples(lineIndex, samples, sampleCount, color, valueMin, valueMax, thickness);
            return;
        }

        if (_flags.tiledMode && !_flags.inSpritePass)
        {
            if (_screen.current >= _screen.capacity)
                return;

            GraphArea *area = ensureGraphArea(_screen.current);
            if (!area || area->innerW <= 1 || area->innerH <= 1)
                return;

            tiledRenderAndPresentRect(area->innerX, area->innerY,
                                      area->innerW, area->innerH,
                                      "tiled-update-graph-samples",
                                      [&]()
                                      { drawGraphSamples(lineIndex, samples, sampleCount, color, valueMin, valueMax, thickness); });
            return;
        }

        const bool prevRender = _flags.inSpritePass;
        pipcore::Sprite *prevActive = _render.activeSprite;

        _flags.inSpritePass = 1;
        _render.activeSprite = &_render.sprite;
        drawGraphSamples(lineIndex, samples, sampleCount, color, valueMin, valueMax, thickness);
        _flags.inSpritePass = prevRender;
        _render.activeSprite = prevActive;

        if (prevRender || _screen.current >= _screen.capacity)
            return;

        GraphArea *area = ensureGraphArea(_screen.current);
        if (!area)
            return;

        invalidateRect(area->innerX, area->innerY, area->innerW, area->innerH);
    }
}
