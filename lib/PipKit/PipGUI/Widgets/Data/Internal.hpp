#pragma once

#include <PipGUI/Core/GUI.hpp>
#include <PipGUI/Core/Internal/GuiAccess.hpp>
#include <PipGUI/Core/Internal/ViewModels.hpp>

#include <cstring>

namespace pipgui::graph_internal
{
    constexpr uint8_t kDefaultGraphRadius = 17;
    constexpr size_t kGraphRenderCacheMaxBytes = 64u * 1024u;

    struct SeriesWindow
    {
        uint16_t start = 0;
        uint16_t count = 0;
        uint16_t visible = 0;
    };

    inline pipcore::Platform *platform() noexcept
    {
        return pipcore::GetPlatform();
    }

    inline uint16_t resolveOscilloscopeVisibleSamples(const GraphArea &area, uint16_t fallbackWidth) noexcept
    {
        uint32_t desired = area.oscVisibleSamples;
        if (desired == 0 && area.oscSampleRateHz > 0 && area.oscTimebaseMs > 0)
            desired = ((uint32_t)area.oscSampleRateHz * (uint32_t)area.oscTimebaseMs + 999U) / 1000U;
        if (desired < 2)
            desired = fallbackWidth;
        if (desired < 2)
            desired = 2;
        if (desired > 1024U)
            desired = 1024U;
        return (uint16_t)desired;
    }

    inline uint16_t deriveGraphGridColor565(uint16_t bg565) noexcept
    {
        return detail::blend565(bg565, (uint16_t)0xFFFF, 13);
    }

    void invalidateGraphRenderCache(GraphArea &area) noexcept;
    void freeGraphBuffers(GraphArea &area) noexcept;

    bool ensureGraphLineStorage(GraphArea &area, uint16_t lineIndex);
    bool ensureGraphSampleCapacity(GraphArea &area, uint16_t desiredCap);
    bool ensureGraphLineBuffer(GraphArea &area, uint16_t lineIndex);
    SeriesWindow resolveSeriesWindow(const GraphArea &area, uint16_t lineIndex, uint16_t maxVisible) noexcept;
    void appendGraphSample(GraphArea &area, uint16_t lineIndex, int16_t value, uint16_t maxVisible);
    bool resolveAutoScale(GraphArea &area, uint16_t maxVisible, int16_t &outMin, int16_t &outMax);

    bool ensureGraphInnerCache(GraphArea &area);
    bool ensureGraphRenderCache(GraphArea &area);
    void snapshotGraphRenderCache(pipcore::Sprite *t, GraphArea &area, int16_t originX, int16_t originY, int16_t screenH) noexcept;
    bool blitGraphRenderCache(pipcore::Sprite *t, const GraphArea &area, int16_t originX, int16_t originY) noexcept;
    void buildGraphInnerCache(GraphArea &area);
    void snapshotRenderedGraph(GraphArea &area) noexcept;

    int16_t graphValueToY(const GraphArea &area, int16_t value, int16_t valueMin, int16_t valueMax) noexcept;
    bool canUseIncrementalScrollRender(const GraphArea &area) noexcept;
    bool shiftGraphInnerOnePixel(pipcore::Sprite *t, const GraphArea &area, int16_t originX, int16_t originY);
    void redrawGraphInner(pipcore::Sprite *t, const GraphArea &area, int16_t originX, int16_t originY);
    void renderSeries(GUI &gui,
                      pipcore::Sprite *t,
                      const GraphArea &area,
                      const int16_t *samples,
                      const SeriesWindow &window,
                      uint16_t sampleCapacity,
                      uint16_t line565,
                      int16_t valueMin,
                      int16_t valueMax,
                      uint8_t lineThickness);
    void renderBufferedGraph(GUI &gui, pipcore::Sprite *t, GraphArea &area, uint16_t maxVisible, int16_t originX, int16_t originY);
}
