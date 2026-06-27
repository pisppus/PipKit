#include "Internal.hpp"
#include <cstring>

namespace pipgui
{
    namespace
    {
        struct RGB8
        {
            uint8_t r, g, b;
        };

        static inline bool intersectRectWithClip(int16_t &x, int16_t &y, int16_t &w, int16_t &h,
                                                 int32_t clipX, int32_t clipY, int32_t clipW, int32_t clipH)
        {
            if (w <= 0 || h <= 0 || clipW <= 0 || clipH <= 0)
                return false;
            int32_t x0 = max<int32_t>(x, clipX), y0 = max<int32_t>(y, clipY);
            int32_t x1 = min<int32_t>(x + w, clipX + clipW), y1 = min<int32_t>(y + h, clipY + clipH);
            if (x0 >= x1 || y0 >= y1)
                return false;
            x = (int16_t)x0;
            y = (int16_t)y0;
            w = (int16_t)(x1 - x0);
            h = (int16_t)(y1 - y0);
            return true;
        }

        static inline size_t alignLookupOffset(size_t offset, size_t align) noexcept
        {
            return (offset + align - 1) & ~(align - 1);
        }

        static inline void unpack565(uint16_t c, uint8_t &r, uint8_t &g, uint8_t &b) noexcept
        {
            r = (c >> 11) & 0x1F;
            g = (c >> 5) & 0x3F;
            b = c & 0x1F;
        }

        [[nodiscard]] static inline uint16_t pack565(uint32_t r, uint32_t g, uint32_t b) noexcept
        {
            return (uint16_t)((r << 11) | (g << 5) | b);
        }

        [[nodiscard]] static inline uint16_t avg2Rgb565(uint16_t a, uint16_t b) noexcept
        {
            return (uint16_t)((((a ^ b) & 0xF7DEu) >> 1) + (a & b));
        }

        [[nodiscard]] static inline uint16_t avg4Rgb565(uint16_t a, uint16_t b, uint16_t c, uint16_t d) noexcept
        {
            return avg2Rgb565(avg2Rgb565(a, b), avg2Rgb565(c, d));
        }

        [[nodiscard]] static inline uint16_t sampleBilinear2xRgb565(const uint16_t *src,
                                                                    uint32_t r0, uint32_t r1,
                                                                    uint16_t x0, uint16_t x1,
                                                                    bool xH, bool yH) noexcept
        {
            uint16_t c0 = src[r0 + x0];
            if (!yH)
                return xH ? avg2Rgb565(c0, src[r0 + x1]) : c0;
            uint16_t c1 = src[r1 + x0];
            return xH ? avg4Rgb565(c0, src[r0 + x1], c1, src[r1 + x1]) : avg2Rgb565(c0, c1);
        }

        static constexpr uint8_t kBlurReciprocalShift = 24;

        [[nodiscard]] static inline uint32_t blurReciprocal24(uint32_t divsum) noexcept
        {
            return ((1u << kBlurReciprocalShift) + (divsum >> 1)) / divsum;
        }

        [[nodiscard]] static inline uint32_t blurScale24(uint32_t value, uint32_t reciprocal) noexcept
        {
            return (value * reciprocal) >> kBlurReciprocalShift;
        }

        [[nodiscard]] static inline uint8_t scale255(uint8_t v, uint8_t a) noexcept
        {
            return (uint8_t)((v * (a + 1u)) >> 8);
        }

        [[nodiscard]] static inline uint8_t smoothstep8(uint32_t t) noexcept
        {
            uint32_t res = (t * t * (768u - (t << 1))) >> 16;
            return res > 255 ? 255 : (uint8_t)res;
        }

        static void buildSmoothAlpha(uint8_t *dst, int16_t len, bool reverse) noexcept
        {
            if (!dst || len <= 0)
                return;
            if (len == 1)
            {
                dst[0] = 255;
                return;
            }
            const uint32_t step = (256u << 16) / (len - 1);
            uint32_t acc = 0;
            for (int16_t i = 0; i < len; ++i, acc += step)
            {
                uint32_t t = (acc + 0x8000u) >> 16;
                uint8_t val = smoothstep8(t > 256u ? 256u : t);
                dst[i] = reverse ? (uint8_t)(255u - val) : val;
            }
        }

        static void stackBlurPassHorizontal(const uint16_t *src, uint16_t *dst,
                                            int16_t width, int16_t height, uint8_t radius,
                                            RGB8 *stack) noexcept
        {
            if (width <= 0 || height <= 0)
                return;

            const uint32_t wm = width - 1;
            const uint32_t div = radius * 2u + 1u;
            const uint32_t radiusPlus1 = radius + 1u;
            const uint32_t sumFactor = radiusPlus1 * (radiusPlus1 + 1u) >> 1;
            const uint32_t divsum = radiusPlus1 * radiusPlus1;
            const uint32_t divHalf = divsum >> 1;
            const uint32_t divReciprocal = blurReciprocal24(divsum);

            for (int16_t y = 0; y < height; ++y)
            {
                const uint16_t *srcRow = src + (uint32_t)y * width;
                uint16_t *dstRow = dst + (uint32_t)y * width;

                uint8_t firstR = 0, firstG = 0, firstB = 0;
                unpack565(srcRow[0], firstR, firstG, firstB);

                uint32_t sumR = firstR * sumFactor, sumG = firstG * sumFactor, sumB = firstB * sumFactor;
                uint32_t sumOutR = firstR * radiusPlus1, sumOutG = firstG * radiusPlus1, sumOutB = firstB * radiusPlus1;
                uint32_t sumInR = 0, sumInG = 0, sumInB = 0;

                for (uint32_t i = 0; i < radiusPlus1; ++i)
                    stack[i] = {firstR, firstG, firstB};

                for (uint32_t i = 1; i <= radius; ++i)
                {
                    const uint32_t srcIndex = (i < (uint32_t)width) ? i : wm;
                    uint8_t r = 0, g = 0, b = 0;
                    unpack565(srcRow[srcIndex], r, g, b);
                    stack[radius + i] = {r, g, b};
                    const uint32_t weight = radiusPlus1 - i;
                    sumR += r * weight;
                    sumG += g * weight;
                    sumB += b * weight;
                    sumInR += r;
                    sumInG += g;
                    sumInB += b;
                }

                uint32_t stackPointer = radius;
                for (int16_t x = 0; x < width; ++x)
                {
                    dstRow[x] = pack565(blurScale24(sumR + divHalf, divReciprocal),
                                        blurScale24(sumG + divHalf, divReciprocal),
                                        blurScale24(sumB + divHalf, divReciprocal));

                    sumR -= sumOutR;
                    sumG -= sumOutG;
                    sumB -= sumOutB;

                    const uint32_t stackStart = (stackPointer < radius) ? (stackPointer + radius + 1) : (stackPointer - radius);
                    const RGB8 &outVal = stack[stackStart];
                    sumOutR -= outVal.r;
                    sumOutG -= outVal.g;
                    sumOutB -= outVal.b;

                    const uint32_t nextIndex = ((uint32_t)x + radiusPlus1 < wm) ? ((uint32_t)x + radiusPlus1) : wm;
                    uint8_t nextR = 0, nextG = 0, nextB = 0;
                    unpack565(srcRow[nextIndex], nextR, nextG, nextB);
                    stack[stackStart] = {nextR, nextG, nextB};
                    sumInR += nextR;
                    sumInG += nextG;
                    sumInB += nextB;
                    sumR += sumInR;
                    sumG += sumInG;
                    sumB += sumInB;

                    if (++stackPointer == div)
                        stackPointer = 0;

                    const RGB8 &inVal = stack[stackPointer];
                    sumOutR += inVal.r;
                    sumOutG += inVal.g;
                    sumOutB += inVal.b;
                    sumInR -= inVal.r;
                    sumInG -= inVal.g;
                    sumInB -= inVal.b;
                }
            }
        }

        static void stackBlurPassVertical(const uint16_t *src, uint16_t *dst,
                                          int16_t width, int16_t height, uint8_t radius,
                                          RGB8 *stack) noexcept
        {
            if (width <= 0 || height <= 0)
                return;

            const uint32_t hm = height - 1;
            const uint32_t stride = width;
            const uint32_t div = radius * 2u + 1u;
            const uint32_t radiusPlus1 = radius + 1u;
            const uint32_t sumFactor = radiusPlus1 * (radiusPlus1 + 1u) >> 1;
            const uint32_t divsum = radiusPlus1 * radiusPlus1;
            const uint32_t divHalf = divsum >> 1;
            const uint32_t divReciprocal = blurReciprocal24(divsum);

            for (int16_t x = 0; x < width; ++x)
            {
                const uint32_t base = x;
                uint8_t firstR = 0, firstG = 0, firstB = 0;
                unpack565(src[base], firstR, firstG, firstB);

                uint32_t sumR = firstR * sumFactor, sumG = firstG * sumFactor, sumB = firstB * sumFactor;
                uint32_t sumOutR = firstR * radiusPlus1, sumOutG = firstG * radiusPlus1, sumOutB = firstB * radiusPlus1;
                uint32_t sumInR = 0, sumInG = 0, sumInB = 0;

                for (uint32_t i = 0; i < radiusPlus1; ++i)
                    stack[i] = {firstR, firstG, firstB};

                for (uint32_t i = 1; i <= radius; ++i)
                {
                    const uint32_t srcIndex = ((i < hm) ? i : hm) * stride + base;
                    uint8_t r = 0, g = 0, b = 0;
                    unpack565(src[srcIndex], r, g, b);
                    stack[radius + i] = {r, g, b};
                    const uint32_t weight = radiusPlus1 - i;
                    sumR += r * weight;
                    sumG += g * weight;
                    sumB += b * weight;
                    sumInR += r;
                    sumInG += g;
                    sumInB += b;
                }

                uint32_t stackPointer = radius;
                for (int16_t y = 0; y < height; ++y)
                {
                    dst[(uint32_t)y * stride + base] = pack565(blurScale24(sumR + divHalf, divReciprocal),
                                                               blurScale24(sumG + divHalf, divReciprocal),
                                                               blurScale24(sumB + divHalf, divReciprocal));

                    sumR -= sumOutR;
                    sumG -= sumOutG;
                    sumB -= sumOutB;

                    const uint32_t stackStart = (stackPointer < radius) ? (stackPointer + radius + 1) : (stackPointer - radius);
                    const RGB8 &outVal = stack[stackStart];
                    sumOutR -= outVal.r;
                    sumOutG -= outVal.g;
                    sumOutB -= outVal.b;

                    const uint32_t nextIndex = ((uint32_t)y + radiusPlus1 < hm ? (uint32_t)y + radiusPlus1 : hm) * stride + base;
                    uint8_t nextR = 0, nextG = 0, nextB = 0;
                    unpack565(src[nextIndex], nextR, nextG, nextB);
                    stack[stackStart] = {nextR, nextG, nextB};
                    sumInR += nextR;
                    sumInG += nextG;
                    sumInB += nextB;
                    sumR += sumInR;
                    sumG += sumInG;
                    sumB += sumInB;

                    if (++stackPointer == div)
                        stackPointer = 0;

                    const RGB8 &inVal = stack[stackPointer];
                    sumOutR += inVal.r;
                    sumOutG += inVal.g;
                    sumOutB += inVal.b;
                    sumInR -= inVal.r;
                    sumInG -= inVal.g;
                    sumInB -= inVal.b;
                }
            }
        }
    }

    bool GUI::ensureBlurWorkBuffers(uint32_t smallLen, int16_t sw, int16_t sh,
                                    int16_t w, int16_t h, uint8_t radiusSmall) noexcept
    {
        if (smallLen == 0 || sw <= 0 || sh <= 0 || w <= 0 || h <= 0)
            return false;

        pipcore::Platform *plat = platform();

        auto alloc565Pair = [&](uint16_t *&a, uint16_t *&b, size_t n) -> bool
        {
            void *na = detail::alloc(plat, n * sizeof(uint16_t), pipcore::AllocCaps::Default);
            void *nb = detail::alloc(plat, n * sizeof(uint16_t), pipcore::AllocCaps::Default);
            if (!na || !nb)
            {
                detail::free(plat, na);
                detail::free(plat, nb);
                return false;
            }
            detail::free(plat, a);
            detail::free(plat, b);
            a = (uint16_t *)na;
            b = (uint16_t *)nb;
            return true;
        };

        auto ensureLookup = [&](int16_t needSw, int16_t needSh, int16_t needW,
                                int16_t needH, uint8_t needRadius) -> bool
        {
            if (_blur.lookup && _blur.lookupSw >= needSw && _blur.lookupSh >= needSh &&
                _blur.lookupW >= needW && _blur.lookupH >= needH && _blur.lookupRadius >= needRadius)
                return true;

            size_t bytes = 0;
            bytes = alignLookupOffset(bytes, alignof(int16_t));
            bytes += (size_t)needSw * sizeof(int16_t) * 2u;
            bytes = alignLookupOffset(bytes, alignof(int32_t));
            bytes += (size_t)needSh * sizeof(int32_t) * 2u;
            bytes = alignLookupOffset(bytes, alignof(uint8_t));
            bytes += (size_t)max<int16_t>(needW, needH) * sizeof(uint8_t);
            bytes += (size_t)(((uint32_t)needRadius * 2u) + 1u) * sizeof(RGB8);

            void *lookup = detail::alloc(plat, bytes, pipcore::AllocCaps::Default);
            if (!lookup)
                return false;

            detail::free(plat, _blur.lookup);
            _blur.lookup = static_cast<uint8_t *>(lookup);
            _blur.lookupSw = (uint16_t)needSw;
            _blur.lookupSh = (uint16_t)needSh;
            _blur.lookupW = (uint16_t)needW;
            _blur.lookupH = (uint16_t)needH;
            _blur.lookupRadius = needRadius;
            return true;
        };

        if ((!_blur.smallIn || !_blur.smallTmp || _blur.workLen < smallLen) &&
            !alloc565Pair(_blur.smallIn, _blur.smallTmp, smallLen))
            return false;
        _blur.workLen = smallLen;

        return ensureLookup(sw, sh, w, h, radiusSmall);
    }

    bool GUI::ensureBlurCaptureSprite(int16_t w, int16_t h) noexcept
    {
        if (w <= 0 || h <= 0)
            return false;
        _blur.captureSprite.setPlatform(platform());
        if (_blur.captureSprite.width() >= w && _blur.captureSprite.height() >= h && _blur.captureSprite.getBuffer())
            return true;

        _blur.captureSprite.deleteSprite();
        if (_blur.captureSprite.createSprite(w, h))
            return true;

        pipcore::Platform *plat = platform();
        const size_t bytes = static_cast<size_t>(w) * static_cast<size_t>(h) * sizeof(uint16_t);
        if (plat && detail::recoverFromAllocFailure(plat, bytes, pipcore::AllocCaps::Default))
            return _blur.captureSprite.createSprite(w, h);
        return false;
    }

    bool GUI::renderBlurBackdropToSprite(pipcore::Sprite &dst, int16_t x, int16_t y, int16_t w, int16_t h)
    {
        if (w <= 0 || h <= 0 || !dst.getBuffer())
            return false;

        const bool prevRender = _flags.inSpritePass, prevBlurCapture = _flags.blurCapturePass;
        pipcore::Sprite *prevActive = _render.activeSprite;
        const ClipState prevClip = _clip;
        const uint8_t prevCurrent = _screen.current;
        const int16_t prevOriginX = _render.originX, prevOriginY = _render.originY;
        int32_t prevClipX = 0, prevClipY = 0, prevClipW = 0, prevClipH = 0;
        dst.getClipRect(&prevClipX, &prevClipY, &prevClipW, &prevClipH);

        _flags.inSpritePass = _flags.blurCapturePass = 1;
        _render.activeSprite = &dst;
        _render.originX = _clip.x = x;
        _render.originY = _clip.y = y;
        _clip.enabled = true;
        _clip.w = w;
        _clip.h = h;
        dst.setClipRect(0, 0, w, h);

        setBackgroundColorCache(0x0000);
        const bool prevPaintCaptureSuspended = Debug::paintCaptureSuspended();
        Debug::setPaintCaptureSuspended(true);
        clear(_render.bgColor565 ? _render.bgColor565 : (uint16_t)_render.bgColor);
        Debug::setPaintCaptureSuspended(prevPaintCaptureSuspended);

        const uint8_t screenId = _screen.current;
        const ScreenCallback currentCb = (screenId < _screen.capacity && _screen.callbacks) ? _screen.callbacks[screenId] : nullptr;

        if (_flags.bootActive)
            renderBootFrame(nowMs());
        else if (_flags.errorActive)
            renderErrorFrame(nowMs());
        else if (screenId < _screen.capacity)
        {
            ListState *list = getList(screenId);
            if (list && list->configured && list->itemCount > 0)
                updateListScreen(screenId);
            else
            {
                TileState *tile = getTile(screenId);
                if (tile && tile->configured && tile->itemCount > 0)
                    renderTileScreen(screenId);
                else
                {
                    beginGraphFrame(screenId);
                    if (currentCb)
                        currentCb(*this);
                    endGraphFrame(screenId);
                }
            }
        }

        _screen.current = prevCurrent;
        _render.originX = prevOriginX;
        _render.originY = prevOriginY;
        _clip = prevClip;
        dst.setClipRect((int16_t)prevClipX, (int16_t)prevClipY, (int16_t)prevClipW, (int16_t)prevClipH);
        _render.activeSprite = prevActive;
        _flags.blurCapturePass = prevBlurCapture;
        _flags.inSpritePass = prevRender;
        return true;
    }

    void GUI::drawBlurRegion(int16_t x, int16_t y, int16_t w, int16_t h,
                             uint8_t radius, BlurDirection dir, bool gradient,
                             uint8_t materialStrength, int32_t materialColor)
    {
        if (_flags.blurCapturePass)
            return;
        if (w <= 0 || h <= 0)
            return;
        if (x == -1)
            x = AutoX(w);
        if (y == -1)
            y = AutoY(h);
        if (radius < 1)
            radius = 1;
        _blur.lastUseMs = nowMs();

        if (_flags.spriteEnabled && _disp.display && !_flags.inSpritePass)
        {
            updateBlurRegion(x, y, w, h, radius, dir, gradient, materialStrength, materialColor);
            return;
        }

        pipcore::Sprite *spr = getDrawTarget();
        if (!spr)
            return;

        uint16_t *targetBuf = (uint16_t *)spr->getBuffer();
        const int16_t targetStride = spr->width();
        if (!targetBuf || targetStride <= 0)
            return;

        const int16_t originX = _render.originX, originY = _render.originY;
        const int16_t tileX = originX, tileY = originY, tileW = spr->width(), tileH = spr->height();

        int16_t drawX = x, drawY = y;
        int32_t globalClipX = 0, globalClipY = 0, globalClipW = _render.screenWidth, globalClipH = _render.screenHeight;
        if (_clip.enabled)
        {
            globalClipX = _clip.x;
            globalClipY = _clip.y;
            globalClipW = _clip.w;
            globalClipH = _clip.h;
        }

        if (!intersectRectWithClip(drawX, drawY, w, h, globalClipX, globalClipY, globalClipW, globalClipH))
            return;
        if (!intersectRectWithClip(drawX, drawY, w, h, tileX, tileY, tileW, tileH))
            return;

        const int16_t targetX = (int16_t)(drawX - originX), targetY = (int16_t)(drawY - originY);
        const int16_t sampleGlobalX = (int16_t)(drawX - radius), sampleGlobalY = (int16_t)(drawY - radius);
        const int16_t sampleW = (int16_t)(w + radius * 2), sampleH = (int16_t)(h + radius * 2);
        if (sampleW <= 0 || sampleH <= 0)
            return;

        const bool captureNeeded = _flags.tiledMode && _flags.inSpritePass &&
                                   (sampleGlobalX < tileX || sampleGlobalY < tileY ||
                                    sampleGlobalX + sampleW > tileX + tileW ||
                                    sampleGlobalY + sampleH > tileY + tileH);

        uint16_t *sourceBuf = targetBuf;
        int16_t sourceStride = targetStride, sourceOriginX = originX, sourceOriginY = originY;

        if (captureNeeded)
        {
            if (!ensureBlurCaptureSprite(sampleW, sampleH))
                return;
            if (!renderBlurBackdropToSprite(_blur.captureSprite, sampleGlobalX, sampleGlobalY, sampleW, sampleH))
                return;
            sourceBuf = static_cast<uint16_t *>(_blur.captureSprite.getBuffer());
            sourceStride = _blur.captureSprite.width();
            sourceOriginX = sampleGlobalX;
            sourceOriginY = sampleGlobalY;
            if (!sourceBuf || sourceStride <= 0)
                return;
        }

        const int32_t clipR = globalClipX + globalClipW - 1, clipB = globalClipY + globalClipH - 1;

        auto swap16 = [](uint16_t v)
        { return (uint16_t)((v >> 8) | (v << 8)); };
        auto readSource565 = [&](int32_t i)
        { return swap16(sourceBuf[i]); };
        auto writeTarget565 = [&](int32_t i, uint16_t c)
        { targetBuf[i] = swap16(c); };

        const int16_t sw = (int16_t)((sampleW + 1) >> 1), sh = (int16_t)((sampleH + 1) >> 1);
        const uint32_t smallLen = (uint32_t)sw * sh;
        const uint8_t radiusSmall = (uint8_t)((radius + 1) >> 1);
        if (!smallLen || !ensureBlurWorkBuffers(smallLen, sw, sh, w, h, radiusSmall))
            return;

        uint16_t *smallBlur = _blur.smallIn, *smallBase = _blur.smallTmp;
        uint8_t *lookup = _blur.lookup;

        auto getOffset = [&](size_t &off, size_t sz, size_t align)
        {
            off = alignLookupOffset(off, align);
            uint8_t *res = lookup + off;
            off += sz;
            return res;
        };

        size_t off = 0;
        int16_t *sampleX0 = reinterpret_cast<int16_t *>(getOffset(off, sw * sizeof(int16_t), alignof(int16_t)));
        int16_t *sampleX1 = reinterpret_cast<int16_t *>(getOffset(off, sw * sizeof(int16_t), alignof(int16_t)));
        int32_t *sampleRow0 = reinterpret_cast<int32_t *>(getOffset(off, sh * sizeof(int32_t), alignof(int32_t)));
        int32_t *sampleRow1 = reinterpret_cast<int32_t *>(getOffset(off, sh * sizeof(int32_t), alignof(int32_t)));
        uint8_t *gradAlpha = getOffset(off, max<int16_t>(w, h), alignof(uint8_t));
        RGB8 *stack = reinterpret_cast<RGB8 *>(getOffset(off, 0, alignof(RGB8)));

        auto clampVal = [](int32_t val, int32_t low, int32_t high)
        {
            return (int16_t)(val < low ? low : (val > high ? high : val));
        };

        for (int16_t sx = 0; sx < sw; ++sx)
        {
            int16_t x0 = sampleGlobalX + (sx << 1);
            int16_t x1 = (x0 + 1 < sampleGlobalX + sampleW) ? (int16_t)(x0 + 1) : x0;
            sampleX0[sx] = clampVal(x0, globalClipX, clipR) - sourceOriginX;
            sampleX1[sx] = clampVal(x1, globalClipX, clipR) - sourceOriginX;
        }

        for (int16_t sy = 0; sy < sh; ++sy)
        {
            int16_t y0 = sampleGlobalY + (sy << 1);
            int16_t y1 = (y0 + 1 < sampleGlobalY + sampleH) ? (int16_t)(y0 + 1) : y0;
            sampleRow0[sy] = (clampVal(y0, globalClipY, clipB) - sourceOriginY) * sourceStride;
            sampleRow1[sy] = (clampVal(y1, globalClipY, clipB) - sourceOriginY) * sourceStride;
        }

        for (int16_t sy = 0; sy < sh; ++sy)
        {
            const int32_t r0 = sampleRow0[sy], r1 = sampleRow1[sy];
            for (int16_t sx = 0; sx < sw; ++sx)
            {
                const int16_t x0 = sampleX0[sx], x1 = sampleX1[sx];
                smallBase[(uint32_t)sy * sw + sx] = avg4Rgb565(
                    readSource565(r0 + x0), readSource565(r0 + x1),
                    readSource565(r1 + x0), readSource565(r1 + x1));
            }
        }

        const bool useMaterial = (materialStrength > 0);
        const uint16_t mat565 = useMaterial ? detail::resolveOptionalColor565(materialColor, _render.bgColor565) : 0;
        const bool gradH = gradient && (dir == LeftRight || dir == RightLeft);
        const bool gradV = gradient && (dir == TopDown || dir == BottomUp);
        const int32_t sampleBaseX = drawX - sampleGlobalX;

        std::memcpy(smallBlur, smallBase, (size_t)smallLen * sizeof(uint16_t));

        if (radiusSmall > 0)
        {
            stackBlurPassHorizontal(smallBlur, smallBlur, sw, sh, radiusSmall, stack);
            stackBlurPassVertical(smallBlur, smallBlur, sw, sh, radiusSmall, stack);
        }

        uint16_t *smallFinal = smallBlur;
        uint16_t *smallAxis = smallBase;

        if (gradH)
            buildSmoothAlpha(gradAlpha, w, dir == LeftRight);
        if (gradV)
            buildSmoothAlpha(gradAlpha, h, dir == TopDown);

        if (!gradient && !useMaterial)
        {
            for (int16_t iy = 0; iy < h; ++iy)
            {
                const int32_t screenOff = (int32_t)(targetY + iy) * targetStride + targetX;
                const int32_t sampleY = (int32_t)(drawY + iy) - sampleGlobalY;
                const uint16_t sy0 = (uint16_t)min<int32_t>(sampleY >> 1, sh - 1);
                const uint16_t sy1 = (uint16_t)min<int32_t>(sy0 + 1, sh - 1);
                const uint32_t rOff0 = (uint32_t)sy0 * sw, rOff1 = (uint32_t)sy1 * sw;
                const bool yHalf = (sampleY & 1) != 0;

                for (int16_t ix = 0; ix < w; ++ix)
                {
                    const int32_t curX = sampleBaseX + ix;
                    const uint16_t sx0 = curX >> 1;
                    const uint16_t sx1 = (sx0 + 1 < sw) ? (sx0 + 1) : (sw - 1);
                    const bool xHalf = (curX & 1) != 0;
                    writeTarget565(screenOff + ix, sampleBilinear2xRgb565(smallFinal, rOff0, rOff1, sx0, sx1, xHalf, yHalf));
                }
            }
        }
        else
        {
            for (int16_t iy = 0; iy < h; ++iy)
            {
                const int32_t screenOff = (int32_t)(targetY + iy) * targetStride + targetX;
                const int32_t sampleY = (int32_t)(drawY + iy) - sampleGlobalY;
                const uint16_t sy0 = (uint16_t)min<int32_t>(sampleY >> 1, sh - 1);
                const uint16_t sy1 = (uint16_t)min<int32_t>(sy0 + 1, sh - 1);
                const uint32_t rOff0 = (uint32_t)sy0 * sw, rOff1 = (uint32_t)sy1 * sw;
                const bool yHalf = (sampleY & 1) != 0;

                const uint8_t blurAlphaRow = gradV ? gradAlpha[iy] : 255;
                const uint8_t matAlphaRow = useMaterial ? (gradV ? scale255(materialStrength, blurAlphaRow) : materialStrength) : 0;

                for (int16_t ix = 0; ix < w; ++ix)
                {
                    const int32_t curX = sampleBaseX + ix;
                    const uint16_t sx0 = curX >> 1;
                    const uint16_t sx1 = (sx0 + 1 < sw) ? (sx0 + 1) : (sw - 1);
                    const bool xHalf = (curX & 1) != 0;

                    const uint8_t axisAlpha = gradH ? gradAlpha[ix] : 255;
                    const uint8_t blurAlpha = gradH ? axisAlpha : blurAlphaRow;
                    const uint8_t materialAlpha = useMaterial ? scale255(matAlphaRow, axisAlpha) : 0;

                    const uint16_t blurred = sampleBilinear2xRgb565(smallFinal, rOff0, rOff1, sx0, sx1, xHalf, yHalf);
                    uint16_t mixed = blurred;
                    if (blurAlpha != 255)
                    {
                        const uint16_t base = sampleBilinear2xRgb565(smallAxis, rOff0, rOff1, sx0, sx1, xHalf, yHalf);
                        mixed = (blurAlpha != 0) ? detail::blend565(base, blurred, blurAlpha) : base;
                    }
                    if (materialAlpha)
                        mixed = detail::blend565(mixed, mat565, materialAlpha);
                    writeTarget565(screenOff + ix, mixed);
                }
            }
        }
    }

    void GUI::updateBlurRegion(int16_t x, int16_t y, int16_t w, int16_t h,
                               uint8_t radius, BlurDirection dir, bool gradient,
                               uint8_t materialStrength, int32_t materialColor)
    {
        if (!_flags.spriteEnabled || !_disp.display)
            drawBlurRegion(x, y, w, h, radius, dir, gradient, materialStrength, materialColor);
        else
            renderToSpriteAndInvalidate(x, y, w, h, [&]
                                        { drawBlurRegion(x, y, w, h, radius, dir, gradient, materialStrength, materialColor); });
    }
}
