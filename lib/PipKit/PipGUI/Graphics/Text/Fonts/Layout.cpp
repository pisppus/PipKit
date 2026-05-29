#include "Internal.hpp"

namespace pipgui
{
    namespace
    {
        static uint32_t hashMarqueePhaseKey(int16_t x, int16_t y,
                                            int16_t maxWidth,
                                            TextAlign align,
                                            FontId fontId,
                                            uint16_t sizePx,
                                            uint16_t weight,
                                            uint32_t phaseStartMs) noexcept
        {
            uint32_t hash = 2166136261u;
            auto mix = [&](uint32_t value)
            {
                hash ^= value;
                hash *= 16777619u;
            };

            mix(static_cast<uint16_t>(x));
            mix(static_cast<uint16_t>(y));
            mix(static_cast<uint16_t>(maxWidth));
            mix(static_cast<uint8_t>(align));
            mix(static_cast<uint8_t>(fontId));
            mix(sizePx);
            mix(weight);
            mix(phaseStartMs);
            return hash;
        }

        static detail::MarqueePhaseCacheEntry &resolveMarqueePhaseEntry(detail::MarqueePhaseCacheState &cache,
                                                                        uint32_t key,
                                                                        uint32_t nowMs) noexcept
        {
            detail::MarqueePhaseCacheEntry *best = &cache.entries[0];
            for (uint8_t i = 0; i < detail::MARQUEE_PHASE_CACHE_MAX; ++i)
            {
                detail::MarqueePhaseCacheEntry &entry = cache.entries[i];
                if (entry.valid && entry.key == key)
                {
                    entry.lastUseMs = nowMs;
                    return entry;
                }
                if (!entry.valid)
                    best = &entry;
                else if (best->valid && entry.lastUseMs < best->lastUseMs)
                    best = &entry;
            }

            *best = {};
            best->valid = true;
            best->key = key;
            best->lastUseMs = nowMs;
            return *best;
        }
    }

    bool GUI::measureText(const String &text, int16_t &outW, int16_t &outH) const
    {
        outW = outH = 0;
        const FontData *font = fontDataForId(_typo.currentFontId);
        if (!_typo.psdfSizePx || !font)
            return false;

        TextLayoutBox box;
        if (!resolveTextLayoutBoxCached(text, font, _typo.psdfSizePx, _typo.psdfWeight, box))
            return false;
        outW = box.width;
        outH = box.height;
        return true;
    }

    bool GUI::drawTextMarquee(const String &text, int16_t x, int16_t y,
                              int16_t maxWidth, uint16_t fg565,
                              TextAlign align, const MarqueeTextOptions &opts)
    {
        if (maxWidth <= 0)
            return false;

        const FontData *font = fontDataForId(_typo.currentFontId);
        if (!_typo.psdfSizePx || !font)
            return false;
        TextLayoutBox box;
        if (!resolveTextLayoutBoxCached(text, font, _typo.psdfSizePx, _typo.psdfWeight, box) ||
            box.width <= 0 || box.height <= 0)
            return false;
        const int16_t tw = box.width;
        const int16_t th = box.height;

        int16_t boxX = (x == -1) ? AutoX((int32_t)maxWidth) : x;
        if (align == TextAlign::Center)
            boxX -= maxWidth / 2;
        else if (align == TextAlign::Right)
            boxX -= maxWidth;
        const int16_t boxY = (y == -1) ? AutoY((int32_t)th) : y;
        if (tw <= maxWidth)
            return false;

        pipcore::Sprite *target = getDrawTarget();
        if (!target)
            return false;

        int32_t prevClipX = 0, prevClipY = 0, prevClipW = 0, prevClipH = 0;
        target->getClipRect(&prevClipX, &prevClipY, &prevClipW, &prevClipH);

        int32_t clipX = (int32_t)boxX - _render.originX;
        int32_t clipY = (int32_t)boxY - _render.originY;
        int32_t clipW = maxWidth;
        int32_t clipH = th;

        if (prevClipW > 0 && prevClipH > 0)
        {
            const int32_t prevRight = prevClipX + prevClipW;
            const int32_t prevBottom = prevClipY + prevClipH;
            const int32_t boxRight = clipX + clipW;
            const int32_t boxBottom = clipY + clipH;

            if (clipX < prevClipX)
                clipX = prevClipX;
            if (clipY < prevClipY)
                clipY = prevClipY;

            const int32_t finalRight = (boxRight < prevRight) ? boxRight : prevRight;
            const int32_t finalBottom = (boxBottom < prevBottom) ? boxBottom : prevBottom;
            clipW = finalRight - clipX;
            clipH = finalBottom - clipY;
        }

        if (clipW <= 0 || clipH <= 0)
        {
            target->setClipRect(prevClipX, prevClipY, prevClipW, prevClipH);
            return true;
        }

        target->setClipRect(clipX, clipY, clipW, clipH);

        const uint16_t speedPxPerSec = opts.speedPxPerSec ? opts.speedPxPerSec : 28;
        const uint16_t holdStartMs = opts.holdStartMs;
        const int32_t loopPx = tw + kMarqueeGapPx;

        const uint32_t now = nowMs();
        uint32_t elapsedMs = now;
        if (opts.phaseStartMs != 0)
            elapsedMs = (now >= opts.phaseStartMs) ? (now - opts.phaseStartMs) : 0U;

        int16_t offsetPx = 0;
        if (speedPxPerSec > 0 && loopPx > 0 && opts.phaseStartMs != 0)
        {
            const uint32_t key = hashMarqueePhaseKey(x, y, maxWidth, align,
                                                     _typo.currentFontId,
                                                     _typo.psdfSizePx,
                                                     _typo.psdfWeight,
                                                     opts.phaseStartMs);
            detail::MarqueePhaseCacheEntry &entry = resolveMarqueePhaseEntry(_marqueePhaseCache, key, now);
            const uint32_t holdEndMs = opts.phaseStartMs + holdStartMs;
            const uint32_t loopMilliPx = static_cast<uint32_t>(loopPx) * 1000u;

            if (entry.phaseStartMs != opts.phaseStartMs || entry.loopMilliPx == 0)
            {
                entry.phaseStartMs = opts.phaseStartMs;
                entry.lastNowMs = now;
                entry.holdEndMs = holdEndMs;
                entry.loopMilliPx = loopMilliPx;
                entry.holdDone = (now > holdEndMs);
                entry.offsetMilliPx = 0;

                if (entry.holdDone && loopMilliPx > 0)
                {
                    const uint64_t distanceMilliPx = static_cast<uint64_t>(now - holdEndMs) * speedPxPerSec;
                    entry.offsetMilliPx = static_cast<uint32_t>(distanceMilliPx % loopMilliPx);
                }
            }
            else
            {
                if (!entry.holdDone)
                {
                    if (now > entry.holdEndMs)
                    {
                        entry.holdDone = true;
                        entry.lastNowMs = entry.holdEndMs;
                    }
                    else
                    {
                        entry.lastNowMs = now;
                        entry.offsetMilliPx = 0;
                    }
                }

                if (entry.holdDone && loopMilliPx > 0)
                {
                    if (entry.loopMilliPx != loopMilliPx)
                    {
                        if (entry.offsetMilliPx >= loopMilliPx)
                            entry.offsetMilliPx %= loopMilliPx;
                        entry.loopMilliPx = loopMilliPx;
                    }

                    const uint32_t deltaMs = (now >= entry.lastNowMs) ? (now - entry.lastNowMs) : 0u;
                    const uint64_t advanced = static_cast<uint64_t>(entry.offsetMilliPx) +
                                             static_cast<uint64_t>(deltaMs) * speedPxPerSec;
                    entry.offsetMilliPx = static_cast<uint32_t>(advanced % loopMilliPx);
                    entry.lastNowMs = now;
                }
            }

            if (entry.holdDone && loopMilliPx > 0)
                offsetPx = static_cast<int16_t>(entry.offsetMilliPx / 1000u);
        }
        else if (speedPxPerSec > 0 && loopPx > 0 && elapsedMs > holdStartMs)
        {
            const uint64_t distanceMilliPx = (uint64_t)(elapsedMs - holdStartMs) * speedPxPerSec;
            const uint64_t loopMilliPx = (uint64_t)loopPx * 1000ULL;
            const uint64_t wrappedMilliPx = loopMilliPx ? (distanceMilliPx % loopMilliPx) : 0ULL;
            offsetPx = (int16_t)((wrappedMilliPx + 500ULL) / 1000ULL);
            if (offsetPx >= loopPx)
                offsetPx = 0;
        }

        const int16_t drawX = (int16_t)(boxX - offsetPx + box.originX);
        drawTextImmediateMasked(text, drawX, (int16_t)(boxY + box.originY),
                                tw, th, fg565, 0, TextAlign::Left, boxX, maxWidth, kMarqueeEdgeFadePx);
        if (speedPxPerSec > 0 && loopPx > 0)
        {
            drawTextImmediateMasked(text, (int16_t)(drawX + loopPx), (int16_t)(boxY + box.originY),
                                    tw, th, fg565, 0, TextAlign::Left, boxX, maxWidth, kMarqueeEdgeFadePx);
        }

        target->setClipRect(prevClipX, prevClipY, prevClipW, prevClipH);
        if (speedPxPerSec > 0)
            requestRedraw();
        return true;
    }

    bool GUI::drawTextEllipsized(const String &text, int16_t x, int16_t y,
                                 int16_t maxWidth, uint16_t fg565,
                                 TextAlign align)
    {
        if (maxWidth <= 0)
            return false;

        const FontData *font = fontDataForId(_typo.currentFontId);
        if (!_typo.psdfSizePx || !font)
            return false;

        const char *buf = text.c_str();
        const size_t textLen = text.length();
        TextLayoutBox box;
        if (!resolveTextLayoutBoxCached(text, font, _typo.psdfSizePx, _typo.psdfWeight, box) ||
            box.width <= 0 || box.height <= 0)
            return false;
        if (box.width <= maxWidth)
            return false;

        const String dots("...");
        TextLayoutBox dotsBox;
        if (!resolveTextLayoutBoxCached(dots, font, _typo.psdfSizePx, _typo.psdfWeight, dotsBox))
            return false;

        String clipped;
        if (dotsBox.width >= maxWidth)
        {
            clipped = dots;
        }
        else
        {
            size_t bestCut = 0;
            size_t lo = 0;
            size_t hi = textLen;

            while (lo <= hi)
            {
                const size_t mid = lo + ((hi - lo) >> 1);
                const size_t cut = utf8BoundaryFloor(buf, textLen, mid);
                String trial = text.substring(0, cut) + dots;
                TextLayoutBox trialBox;
                const bool fits = computeTextLayoutBox(trial.c_str(), (int)trial.length(), font, _typo.psdfSizePx, _typo.psdfWeight, trialBox) &&
                                  trialBox.width <= maxWidth;

                if (fits)
                {
                    bestCut = cut;
                    if (cut >= textLen)
                        break;
                    lo = cut + 1;
                }
                else
                {
                    if (cut == 0)
                        break;
                    hi = cut - 1;
                }
            }

            clipped = (bestCut > 0) ? (text.substring(0, bestCut) + dots) : dots;
        }

        if (clipped.length() == 0)
            return false;

        drawTextAligned(clipped, x, y, fg565, 0, align);
        return true;
    }

    bool GUI::drawTextBox(const String &text,
                          int16_t x, int16_t y,
                          int16_t w, int16_t h,
                          uint16_t fg565, uint16_t bg565,
                          TextAlign align,
                          int16_t lineGap)
    {
        if (w <= 0 || h <= 0 || text.length() == 0)
            return false;

        const FontData *font = fontDataForId(_typo.currentFontId);
        if (!_typo.psdfSizePx || !font)
            return false;

        if (lineGap < 0)
            lineGap = static_cast<int16_t>(std::max<int>(1, static_cast<int>(_typo.psdfSizePx) / 10));

        const int16_t lineAdvance = static_cast<int16_t>(std::max(1, ceilToInt(font->lineHeight * static_cast<float>(_typo.psdfSizePx))) + lineGap);
        const int16_t maxY = static_cast<int16_t>(y + h);
        const char *buf = text.c_str();
        const size_t len = text.length();
        size_t pos = 0;
        int16_t lineY = y;
        bool drewAny = false;

        pipcore::Sprite *target = getDrawTarget();
        if (!target)
            return false;

        int32_t prevClipX = 0, prevClipY = 0, prevClipW = 0, prevClipH = 0;
        target->getClipRect(&prevClipX, &prevClipY, &prevClipW, &prevClipH);

        int32_t clipX = static_cast<int32_t>(x) - _render.originX;
        int32_t clipY = static_cast<int32_t>(y) - _render.originY;
        int32_t clipW = w;
        int32_t clipH = h;

        if (prevClipW > 0 && prevClipH > 0)
        {
            const int32_t prevRight = prevClipX + prevClipW;
            const int32_t prevBottom = prevClipY + prevClipH;
            const int32_t boxRight = clipX + clipW;
            const int32_t boxBottom = clipY + clipH;
            if (clipX < prevClipX)
                clipX = prevClipX;
            if (clipY < prevClipY)
                clipY = prevClipY;
            const int32_t finalRight = (boxRight < prevRight) ? boxRight : prevRight;
            const int32_t finalBottom = (boxBottom < prevBottom) ? boxBottom : prevBottom;
            clipW = finalRight - clipX;
            clipH = finalBottom - clipY;
        }

        if (clipW <= 0 || clipH <= 0)
        {
            target->setClipRect(prevClipX, prevClipY, prevClipW, prevClipH);
            return false;
        }

        target->setClipRect(clipX, clipY, clipW, clipH);

        auto fitsWidth = [&](const String &line) -> bool
        {
            int16_t lineW = 0;
            int16_t lineH = 0;
            return measureText(line, lineW, lineH) && lineW <= w;
        };

        auto fitPartialWord = [&](size_t start, size_t end) -> size_t
        {
            size_t best = start;
            size_t cursor = start;
            while (cursor < end)
            {
                size_t candidateEnd = utf8BoundaryFloor(buf, end, cursor + 1);
                if (candidateEnd <= cursor)
                    candidateEnd = cursor + 1;
                if (candidateEnd > end)
                    candidateEnd = end;

                const String candidate = text.substring(start, candidateEnd);
                if (fitsWidth(candidate))
                {
                    best = candidateEnd;
                    cursor = candidateEnd;
                }
                else
                {
                    break;
                }
            }
            return best;
        };

        while (pos < len && lineY < maxY)
        {
            while (pos < len && (buf[pos] == ' ' || buf[pos] == '\t' || buf[pos] == '\r'))
                ++pos;

            if (pos < len && buf[pos] == '\n')
            {
                ++pos;
                lineY = static_cast<int16_t>(lineY + lineAdvance);
                continue;
            }
            if (pos >= len)
                break;

            const size_t lineStart = pos;
            size_t acceptedEnd = pos;
            size_t scan = pos;

            while (scan < len)
            {
                if (buf[scan] == '\n')
                    break;

                size_t tokenEnd = scan;
                while (tokenEnd < len &&
                       buf[tokenEnd] != ' ' &&
                       buf[tokenEnd] != '\t' &&
                       buf[tokenEnd] != '\r' &&
                       buf[tokenEnd] != '\n')
                    ++tokenEnd;

                const String candidate = text.substring(lineStart, tokenEnd);
                if (fitsWidth(candidate))
                {
                    acceptedEnd = tokenEnd;
                    scan = tokenEnd;
                    while (scan < len && (buf[scan] == ' ' || buf[scan] == '\t' || buf[scan] == '\r'))
                        ++scan;
                    continue;
                }
                break;
            }

            if (acceptedEnd == lineStart)
            {
                size_t tokenEnd = lineStart;
                while (tokenEnd < len &&
                       buf[tokenEnd] != ' ' &&
                       buf[tokenEnd] != '\t' &&
                       buf[tokenEnd] != '\r' &&
                       buf[tokenEnd] != '\n')
                    ++tokenEnd;
                acceptedEnd = fitPartialWord(lineStart, tokenEnd);
                if (acceptedEnd == lineStart)
                    acceptedEnd = utf8BoundaryFloor(buf, tokenEnd, lineStart + 1);
                if (acceptedEnd <= lineStart)
                    break;
            }

            const String line = text.substring(lineStart, acceptedEnd);
            int16_t lineX = x;
            if (align == TextAlign::Center)
                lineX = static_cast<int16_t>(x + w / 2);
            else if (align == TextAlign::Right)
                lineX = static_cast<int16_t>(x + w);

            drawTextAligned(line, lineX, lineY, fg565, bg565, align);
            drewAny = true;

            pos = acceptedEnd;
            while (pos < len && (buf[pos] == ' ' || buf[pos] == '\t' || buf[pos] == '\r'))
                ++pos;
            if (pos < len && buf[pos] == '\n')
                ++pos;

            lineY = static_cast<int16_t>(lineY + lineAdvance);
        }

        target->setClipRect(prevClipX, prevClipY, prevClipW, prevClipH);
        return drewAny;
    }

}
