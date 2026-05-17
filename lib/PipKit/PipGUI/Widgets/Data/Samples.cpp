#include <PipGUI/Widgets/Data/Internal.hpp>

namespace pipgui::graph_internal
{
    bool ensureGraphLineStorage(GraphArea &area, uint16_t lineIndex)
    {
        if (area.lineCount > lineIndex &&
            area.samples &&
            area.lineColors565 &&
            area.lineValueMins &&
            area.lineValueMaxs &&
            area.lineThicknesses &&
            area.sampleCounts &&
            area.sampleHead &&
            area.renderCounts &&
            area.renderHead)
            return true;

        const uint16_t newLineCount = (uint16_t)(lineIndex + 1);
        pipcore::Platform *plat = platform();
        if (!plat)
            return false;

        int16_t **newSamples = (int16_t **)detail::alloc(plat, sizeof(int16_t *) * newLineCount, pipcore::AllocCaps::Default);
        uint16_t *newLineColors565 = (uint16_t *)detail::alloc(plat, sizeof(uint16_t) * newLineCount, pipcore::AllocCaps::Default);
        int16_t *newLineValueMins = (int16_t *)detail::alloc(plat, sizeof(int16_t) * newLineCount, pipcore::AllocCaps::Default);
        int16_t *newLineValueMaxs = (int16_t *)detail::alloc(plat, sizeof(int16_t) * newLineCount, pipcore::AllocCaps::Default);
        uint8_t *newLineThicknesses = (uint8_t *)detail::alloc(plat, sizeof(uint8_t) * newLineCount, pipcore::AllocCaps::Default);
        uint16_t *newCounts = (uint16_t *)detail::alloc(plat, sizeof(uint16_t) * newLineCount, pipcore::AllocCaps::Default);
        uint16_t *newHead = (uint16_t *)detail::alloc(plat, sizeof(uint16_t) * newLineCount, pipcore::AllocCaps::Default);
        uint16_t *newRenderCounts = (uint16_t *)detail::alloc(plat, sizeof(uint16_t) * newLineCount, pipcore::AllocCaps::Default);
        uint16_t *newRenderHead = (uint16_t *)detail::alloc(plat, sizeof(uint16_t) * newLineCount, pipcore::AllocCaps::Default);

        if (!newSamples || !newLineColors565 || !newLineValueMins || !newLineValueMaxs || !newLineThicknesses || !newCounts || !newHead || !newRenderCounts || !newRenderHead)
        {
            if (newSamples)
                detail::free(plat, newSamples);
            if (newLineColors565)
                detail::free(plat, newLineColors565);
            if (newLineValueMins)
                detail::free(plat, newLineValueMins);
            if (newLineValueMaxs)
                detail::free(plat, newLineValueMaxs);
            if (newLineThicknesses)
                detail::free(plat, newLineThicknesses);
            if (newCounts)
                detail::free(plat, newCounts);
            if (newHead)
                detail::free(plat, newHead);
            if (newRenderCounts)
                detail::free(plat, newRenderCounts);
            if (newRenderHead)
                detail::free(plat, newRenderHead);
            return false;
        }

        for (uint16_t i = 0; i < newLineCount; ++i)
        {
            newSamples[i] = nullptr;
            newLineColors565[i] = 0;
            newLineValueMins[i] = 0;
            newLineValueMaxs[i] = 1;
            newLineThicknesses[i] = 1;
            newCounts[i] = 0;
            newHead[i] = 0;
            newRenderCounts[i] = 0;
            newRenderHead[i] = 0;
        }

        for (uint16_t i = 0; i < area.lineCount; ++i)
        {
            newSamples[i] = area.samples ? area.samples[i] : nullptr;
            newLineColors565[i] = area.lineColors565 ? area.lineColors565[i] : 0;
            newLineValueMins[i] = area.lineValueMins ? area.lineValueMins[i] : 0;
            newLineValueMaxs[i] = area.lineValueMaxs ? area.lineValueMaxs[i] : 1;
            newLineThicknesses[i] = area.lineThicknesses ? area.lineThicknesses[i] : 1;
            newCounts[i] = area.sampleCounts ? area.sampleCounts[i] : 0;
            newHead[i] = area.sampleHead ? area.sampleHead[i] : 0;
            newRenderCounts[i] = area.renderCounts ? area.renderCounts[i] : 0;
            newRenderHead[i] = area.renderHead ? area.renderHead[i] : 0;
        }

        if (area.samples)
            detail::free(plat, area.samples);
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

        area.samples = newSamples;
        area.lineColors565 = newLineColors565;
        area.lineValueMins = newLineValueMins;
        area.lineValueMaxs = newLineValueMaxs;
        area.lineThicknesses = newLineThicknesses;
        area.sampleCounts = newCounts;
        area.sampleHead = newHead;
        area.renderCounts = newRenderCounts;
        area.renderHead = newRenderHead;
        area.lineCount = newLineCount;
        return true;
    }

    bool ensureGraphSampleCapacity(GraphArea &area, uint16_t desiredCap)
    {
        if (desiredCap < 2)
            desiredCap = 2;
        if (area.sampleCapacity >= desiredCap)
            return true;
        if (area.lineCount == 0)
        {
            area.sampleCapacity = desiredCap;
            return true;
        }
        if (!area.samples || !area.sampleCounts || !area.sampleHead)
            return false;

        pipcore::Platform *plat = platform();
        if (!plat)
            return false;

        int16_t **newBuffers = (int16_t **)detail::alloc(plat, sizeof(int16_t *) * area.lineCount, pipcore::AllocCaps::Default);
        if (!newBuffers)
            return false;

        for (uint16_t i = 0; i < area.lineCount; ++i)
            newBuffers[i] = nullptr;

        for (uint16_t line = 0; line < area.lineCount; ++line)
        {
            newBuffers[line] = (int16_t *)detail::alloc(plat, sizeof(int16_t) * desiredCap, pipcore::AllocCaps::Default);
            if (!newBuffers[line])
            {
                for (uint16_t i = 0; i < line; ++i)
                    detail::free(plat, newBuffers[i]);
                detail::free(plat, newBuffers);
                return false;
            }

            for (uint16_t i = 0; i < desiredCap; ++i)
                newBuffers[line][i] = 0;

            const int16_t *oldBuf = area.samples[line];
            const uint16_t oldCap = area.sampleCapacity;
            const uint16_t oldCount = area.sampleCounts[line];
            const uint16_t keep = (oldCount < desiredCap) ? oldCount : desiredCap;

            if (!oldBuf || oldCap < 2 || keep == 0)
                continue;

            const uint16_t start = (uint16_t)((area.sampleHead[line] + oldCap - keep) % oldCap);
            for (uint16_t i = 0; i < keep; ++i)
            {
                const uint16_t idx = (uint16_t)((start + i) % oldCap);
                newBuffers[line][i] = oldBuf[idx];
            }
        }

        for (uint16_t line = 0; line < area.lineCount; ++line)
        {
            if (area.samples[line])
                detail::free(plat, area.samples[line]);
            area.samples[line] = newBuffers[line];

            const uint16_t keep = (area.sampleCounts[line] < desiredCap) ? area.sampleCounts[line] : desiredCap;
            area.sampleCounts[line] = keep;
            area.sampleHead[line] = (keep >= desiredCap) ? 0 : keep;
        }

        detail::free(plat, newBuffers);
        area.sampleCapacity = desiredCap;
        return true;
    }

    bool ensureGraphLineBuffer(GraphArea &area, uint16_t lineIndex)
    {
        if (!area.samples || !area.sampleCounts || !area.sampleHead || lineIndex >= area.lineCount || area.sampleCapacity < 2)
            return false;
        if (area.samples[lineIndex])
            return true;

        pipcore::Platform *plat = platform();
        if (!plat)
            return false;

        int16_t *buf = (int16_t *)detail::alloc(plat, sizeof(int16_t) * area.sampleCapacity, pipcore::AllocCaps::Default);
        if (!buf)
            return false;

        for (uint16_t i = 0; i < area.sampleCapacity; ++i)
            buf[i] = 0;

        area.samples[lineIndex] = buf;
        area.sampleCounts[lineIndex] = 0;
        area.sampleHead[lineIndex] = 0;
        return true;
    }

    SeriesWindow resolveSeriesWindow(const GraphArea &area, uint16_t lineIndex, uint16_t maxVisible) noexcept
    {
        SeriesWindow window;
        if (!area.sampleCounts || !area.sampleHead || lineIndex >= area.lineCount || area.sampleCapacity < 2)
            return window;

        const uint16_t total = area.sampleCounts[lineIndex];
        if (total < 2)
            return window;

        if (area.direction == Oscilloscope)
        {
            window.visible = (maxVisible < area.sampleCapacity) ? maxVisible : area.sampleCapacity;
            if (window.visible < 2)
                return {};

            uint16_t head = area.sampleHead[lineIndex];
            if (head >= window.visible)
                head = 0;

            window.count = (total < window.visible) ? total : window.visible;
            window.start = (uint16_t)((head + window.visible - window.count) % window.visible);
            return window;
        }

        uint16_t head = area.sampleHead[lineIndex];
        if (head >= area.sampleCapacity)
            head = 0;

        window.count = (total < area.sampleCapacity) ? total : area.sampleCapacity;
        window.visible = window.count;
        window.start = (uint16_t)((head + area.sampleCapacity - window.count) % area.sampleCapacity);
        return window;
    }

    void appendGraphSample(GraphArea &area, uint16_t lineIndex, int16_t value, uint16_t maxVisible)
    {
        const uint16_t cap = area.sampleCapacity;
        uint16_t head = area.sampleHead[lineIndex];
        uint16_t count = area.sampleCounts[lineIndex];

        if (area.direction == Oscilloscope)
        {
            const uint16_t visible = (maxVisible < cap) ? maxVisible : cap;
            if (visible < 2)
                return;
            if (head >= visible)
                head = 0;

            if (count == 0)
            {
                for (uint16_t i = 0; i < visible; ++i)
                    area.samples[lineIndex][i] = value;
                count = visible;
                head = (uint16_t)((head + 1) % visible);
            }
            else
            {
                area.samples[lineIndex][head] = value;
                head = (uint16_t)((head + 1) % visible);
            }
        }
        else
        {
            if (head >= cap)
                head = 0;
            area.samples[lineIndex][head] = value;
            head = (uint16_t)((head + 1) % cap);
            if (count < cap)
                ++count;
        }

        area.sampleHead[lineIndex] = head;
        area.sampleCounts[lineIndex] = count;
    }

    bool resolveAutoScale(GraphArea &area, uint16_t maxVisible, int16_t &outMin, int16_t &outMax)
    {
        bool hasData = false;
        int16_t dataMin = 0;
        int16_t dataMax = 0;

        for (uint16_t line = 0; line < area.lineCount; ++line)
        {
            if (!area.samples || !area.samples[line])
                continue;

            const SeriesWindow window = resolveSeriesWindow(area, line, maxVisible);
            if (window.count < 2)
                continue;

            for (uint16_t i = 0; i < window.count; ++i)
            {
                uint16_t idx = (uint16_t)(window.start + i);
                if (idx >= area.sampleCapacity)
                    idx = (uint16_t)(idx % area.sampleCapacity);

                const int16_t v = area.samples[line][idx];
                if (!hasData)
                {
                    dataMin = v;
                    dataMax = v;
                    hasData = true;
                }
                else
                {
                    if (v < dataMin)
                        dataMin = v;
                    if (v > dataMax)
                        dataMax = v;
                }
            }
        }

        if (!hasData)
            return false;

        if (dataMax <= dataMin)
            dataMax = dataMin + 1;

        const int16_t range = (int16_t)(dataMax - dataMin);
        int16_t margin = (int16_t)(range / 6);
        if (margin < 1)
            margin = 1;

        const int16_t targetMin = (int16_t)(dataMin - margin);
        const int16_t targetMax = (int16_t)(dataMax + margin);

        if (!area.autoScaleInitialized)
        {
            area.autoMin = targetMin;
            area.autoMax = targetMax;
            area.autoScaleInitialized = true;
        }
        else
        {
            const uint32_t now = platform() ? platform()->nowMs() : 0;
            bool expanded = false;

            if (targetMax > area.autoMax)
            {
                area.autoMax += (targetMax - area.autoMax + 1) / 2;
                expanded = true;
            }
            else if (now >= area.lastPeakMs && now - area.lastPeakMs > 800)
            {
                const int16_t diff = (int16_t)(targetMax - area.autoMax);
                if (diff < -2)
                    area.autoMax += (diff / 20) ? (diff / 20) : -1;
            }

            if (targetMin < area.autoMin)
            {
                area.autoMin += (targetMin - area.autoMin - 1) / 2;
                expanded = true;
            }
            else if (now >= area.lastPeakMs && now - area.lastPeakMs > 800)
            {
                const int16_t diff = (int16_t)(targetMin - area.autoMin);
                if (diff > 2)
                    area.autoMin += (diff / 20) ? (diff / 20) : 1;
            }

            if (expanded)
                area.lastPeakMs = now;
        }

        if (area.autoMax <= area.autoMin)
            area.autoMax = area.autoMin + 1;

        outMin = area.autoMin;
        outMax = area.autoMax;
        return true;
    }
}
