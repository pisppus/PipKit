#include "Internal.hpp"
#include <PipGUI/Graphics/Draw/Internal.hpp>

#include <math.h>

namespace pipgui
{
    namespace
    {
        static inline uint16_t computeGlowStrength(uint8_t glowStrength, GlowAnim anim,
                                                   uint16_t pulsePeriodMs, uint32_t now)
        {
            if (anim != Pulse || pulsePeriodMs == 0)
                return glowStrength;

            const uint32_t PI_Q16 = 205887;
            const uint32_t TWO_PI_Q16 = 411774;
            uint32_t angle = (uint32_t)(((uint64_t)(now % pulsePeriodMs) * TWO_PI_Q16) / pulsePeriodMs);
            bool neg = (angle > PI_Q16);
            uint32_t x = neg ? (angle - PI_Q16) : angle;
            uint32_t xpm = (uint32_t)((uint64_t)x * (PI_Q16 - x) >> 16);
            uint32_t den = (uint32_t)((5ULL * PI_Q16 * PI_Q16) >> 16) - 4 * xpm;
            uint32_t abs_sin = den ? (uint32_t)(((uint64_t)(16 * xpm) << 16) / den) : 0;
            int32_t sin_val = neg ? -(int32_t)abs_sin : (int32_t)abs_sin;
            uint32_t p = 32768 + (sin_val >> 1);
            uint32_t base = 19661 + (uint32_t)((uint64_t)45875 * p >> 16);
            uint32_t res = (uint32_t)(((uint64_t)glowStrength << 8) * base >> 24);
            return (uint16_t)(res > 255 ? 255 : res);
        }

        static inline uint8_t clampAlpha(int32_t value) noexcept
        {
            if (value <= 0)
                return 0;
            if (value >= 255)
                return 255;
            return static_cast<uint8_t>(value);
        }

        [[nodiscard]] static inline uint8_t glowAlphaAtDistance(float dist,
                                                                int16_t radius,
                                                                uint8_t glowSize,
                                                                uint16_t strength) noexcept
        {
            if (glowSize == 0 || strength == 0)
                return 0;

            float t = 1.0f - ((dist - static_cast<float>(radius)) / static_cast<float>(glowSize));
            if (t <= 0.0f)
                return 0;
            if (t > 1.0f)
                t = 1.0f;

            const float smooth = t * t * (3.0f - 2.0f * t);
            const float alpha = static_cast<float>(strength) * smooth;
            return clampAlpha(static_cast<int32_t>(alpha + 0.5f));
        }
    }

    void GUI::drawGlowCircle(int16_t x, int16_t y, int16_t r,
                             uint16_t fillColor, int16_t bgColor, int16_t glowColor,
                             uint8_t glowSize, uint8_t glowStrength,
                             GlowAnim anim, uint16_t pulsePeriodMs)
    {
        if (r <= 0)
            return;

        if (_flags.spriteEnabled && _disp.display && !_flags.inSpritePass)
        {
            updateGlowCircle(x, y, r, fillColor, bgColor, glowColor, glowSize, glowStrength, anim, pulsePeriodMs);
            return;
        }

        const int32_t outerR = r + glowSize, diam = outerR * 2;
        if (x == center)
            x = (int16_t)(AutoX(diam) + diam / 2);
        if (y == center)
            y = (int16_t)(AutoY(diam) + diam / 2);

        const uint16_t bg = detail::resolveOptionalColor565(bgColor, _render.bgColor565);
        const uint16_t glow = glowColor >= 0 ? (uint16_t)glowColor : detail::blend565WithWhite(fillColor, 80);
        const uint16_t strength = computeGlowStrength(glowStrength, anim, pulsePeriodMs, nowMs());

        if (glowSize == 0 || strength < 2)
        {
            fillCircle(x, y, r, fillColor);
            return;
        }

        pipcore::Sprite *spr = getDrawTarget();
        Surface565 s;
        if (!spr || !getSurface565(spr, s))
            return;

        const int16_t cx = static_cast<int16_t>(x - _render.originX);
        const int16_t cy = static_cast<int16_t>(y - _render.originY);
        const int16_t outerRi = static_cast<int16_t>(r + glowSize);
        int16_t minX = static_cast<int16_t>(cx - outerRi - 1);
        int16_t maxX = static_cast<int16_t>(cx + outerRi + 1);
        int16_t minY = static_cast<int16_t>(cy - outerRi - 1);
        int16_t maxY = static_cast<int16_t>(cy + outerRi + 1);
        if (minX < s.clipX)
            minX = static_cast<int16_t>(s.clipX);
        if (maxX > s.clipR)
            maxX = static_cast<int16_t>(s.clipR);
        if (minY < s.clipY)
            minY = static_cast<int16_t>(s.clipY);
        if (maxY > s.clipB)
            maxY = static_cast<int16_t>(s.clipB);
        if (minX > maxX || minY > maxY)
            return;

        const Color565 fill = makeColor565(fillColor);
        for (int16_t py = minY; py <= maxY; ++py)
        {
            uint16_t *row = s.buf + static_cast<int32_t>(py) * s.stride;
            const float dy = static_cast<float>(py - cy);
            for (int16_t px = minX; px <= maxX; ++px)
            {
                const float dx = static_cast<float>(px - cx);
                const float dist = sqrtf(dx * dx + dy * dy);

                const float innerEdge = static_cast<float>(r) - 0.5f;
                const float outerEdge = static_cast<float>(r) + 0.5f;

                if (dist <= innerEdge)
                {
                    row[px] = fill.fg;
                    debugRecordPaintPixelLocal(px, py);
                    continue;
                }

                if (dist <= static_cast<float>(outerR) + 0.5f)
                {
                    uint8_t glowAlpha = glowAlphaAtDistance(dist, r, glowSize, strength);
                    if (dist > static_cast<float>(outerR) - 0.5f)
                    {
                        const float fade = static_cast<float>(outerR) + 0.5f - dist;
                        glowAlpha = clampAlpha(static_cast<int32_t>(static_cast<float>(glowAlpha) * fade + 0.5f));
                    }

                    uint16_t color = (glowAlpha < 2) ? bg : detail::blend565(bg, glow, glowAlpha);
                    if (dist < outerEdge)
                    {
                        const float coverage = outerEdge - dist;
                        const uint8_t fillAlpha = clampAlpha(static_cast<int32_t>(coverage * 255.0f + 0.5f));
                        color = detail::blend565(color, fillColor, fillAlpha);
                    }

                    row[px] = makeColor565(color).fg;
                    debugRecordPaintPixelLocal(px, py);
                }
            }
        }
    }

    void GUI::updateGlowCircle(int16_t x, int16_t y, int16_t r,
                               uint16_t fillColor, int16_t bgColor, int16_t glowColor,
                               uint8_t glowSize, uint8_t glowStrength,
                               GlowAnim anim, uint16_t pulsePeriodMs)
    {
        if (!_flags.spriteEnabled || !_disp.display)
        {
            drawGlowCircle(x, y, r, fillColor, bgColor, glowColor, glowSize, glowStrength, anim, pulsePeriodMs);
            return;
        }

        const int32_t outerR = r + glowSize, diam = outerR * 2;
        if (x == center)
            x = (int16_t)(AutoX(diam) + diam / 2);
        if (y == center)
            y = (int16_t)(AutoY(diam) + diam / 2);

        const uint16_t bg = detail::resolveOptionalColor565(bgColor, _render.bgColor565);
        constexpr int16_t pad = 2;
        renderToSpriteAndInvalidate(
            (int16_t)(x - outerR - pad), (int16_t)(y - outerR - pad),
            (int16_t)(diam + pad * 2), (int16_t)(diam + pad * 2),
            [&]
            {
                drawRect()
                    .pos((int16_t)(x - outerR - pad), (int16_t)(y - outerR - pad))
                    .size((int16_t)(diam + pad * 2), (int16_t)(diam + pad * 2))
                    .fill(bg)
                    .draw();
                drawGlowCircle(x, y, r, fillColor, (int16_t)bg, glowColor, glowSize, glowStrength, anim, pulsePeriodMs);
            });
    }

}
