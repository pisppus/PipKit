#include <PipGUI/Core/GUI.hpp>
#include <PipGUI/Core/Debug/Debug.hpp>
#include <PipGUI/Systems/Network/Wifi.hpp>

namespace pipgui
{
    namespace
    {
        using DirtyRect = detail::DirtyRect;

        [[nodiscard]] inline bool normalizeDirtyRect(DirtyRect &r, int16_t sw, int16_t sh) noexcept
        {
            if (r.w <= 0 || r.h <= 0 || sw <= 0 || sh <= 0)
                return false;

            int32_t x1 = r.x;
            int32_t y1 = r.y;
            int32_t x2 = x1 + r.w;
            int32_t y2 = y1 + r.h;

            if (x1 < 0)
                x1 = 0;
            if (y1 < 0)
                y1 = 0;
            if (x2 > sw)
                x2 = sw;
            if (y2 > sh)
                y2 = sh;

            if (x2 <= x1 || y2 <= y1)
                return false;

            r.x = (int16_t)x1;
            r.y = (int16_t)y1;
            r.w = (int16_t)(x2 - x1);
            r.h = (int16_t)(y2 - y1);
            return true;
        }

        [[nodiscard]] inline bool rectContains(const DirtyRect &a, const DirtyRect &b) noexcept
        {
            const int32_t ax2 = (int32_t)a.x + a.w;
            const int32_t ay2 = (int32_t)a.y + a.h;
            const int32_t bx2 = (int32_t)b.x + b.w;
            const int32_t by2 = (int32_t)b.y + b.h;
            return b.x >= a.x && b.y >= a.y && bx2 <= ax2 && by2 <= ay2;
        }

        [[nodiscard]] inline bool rectOverlapsOrEdgeTouches(const DirtyRect &a, const DirtyRect &b) noexcept
        {
            const int32_t ax1 = a.x;
            const int32_t ay1 = a.y;
            const int32_t ax2 = (int32_t)a.x + a.w;
            const int32_t ay2 = (int32_t)a.y + a.h;
            const int32_t bx1 = b.x;
            const int32_t by1 = b.y;
            const int32_t bx2 = (int32_t)b.x + b.w;
            const int32_t by2 = (int32_t)b.y + b.h;

            const int32_t ix1 = (ax1 > bx1) ? ax1 : bx1;
            const int32_t iy1 = (ay1 > by1) ? ay1 : by1;
            const int32_t ix2 = (ax2 < bx2) ? ax2 : bx2;
            const int32_t iy2 = (ay2 < by2) ? ay2 : by2;

            const int32_t iw = ix2 - ix1;
            const int32_t ih = iy2 - iy1;
            if (iw > 0 && ih > 0)
                return true;

            if (iw == 0 && ih > 0)
                return true;
            if (ih == 0 && iw > 0)
                return true;
            return false;
        }

        [[nodiscard]] inline DirtyRect rectUnion(const DirtyRect &a, const DirtyRect &b) noexcept
        {
            const int16_t x1 = (a.x < b.x) ? a.x : b.x;
            const int16_t y1 = (a.y < b.y) ? a.y : b.y;
            const int32_t ax2 = (int32_t)a.x + a.w;
            const int32_t ay2 = (int32_t)a.y + a.h;
            const int32_t bx2 = (int32_t)b.x + b.w;
            const int32_t by2 = (int32_t)b.y + b.h;
            const int32_t x2 = (ax2 > bx2) ? ax2 : bx2;
            const int32_t y2 = (ay2 > by2) ? ay2 : by2;
            return DirtyRect{x1, y1, (int16_t)(x2 - x1), (int16_t)(y2 - y1)};
        }

        void compactDirty(detail::DirtyState &dirty, int16_t sw, int16_t sh) noexcept
        {
            if (dirty.count <= 1)
                return;

            uint8_t out = 0;
            for (uint8_t i = 0; i < dirty.count; ++i)
            {
                DirtyRect r = dirty.rects[i];
                if (!normalizeDirtyRect(r, sw, sh))
                    continue;
                dirty.rects[out++] = r;
            }
            dirty.count = out;
            if (dirty.count <= 1)
                return;

            for (uint8_t i = 0; i < dirty.count; ++i)
            {
                for (uint8_t j = 0; j < dirty.count;)
                {
                    if (i == j)
                    {
                        ++j;
                        continue;
                    }
                    if (rectContains(dirty.rects[i], dirty.rects[j]))
                    {
                        dirty.rects[j] = dirty.rects[dirty.count - 1];
                        --dirty.count;
                        continue;
                    }
                    ++j;
                }
            }
            if (dirty.count <= 1)
                return;

            bool changed = true;
            while (changed && dirty.count > 1)
            {
                changed = false;
                for (uint8_t i = 0; i < dirty.count && !changed; ++i)
                {
                    for (uint8_t j = (uint8_t)(i + 1); j < dirty.count; ++j)
                    {
                        if (!rectOverlapsOrEdgeTouches(dirty.rects[i], dirty.rects[j]))
                            continue;
                        dirty.rects[i] = rectUnion(dirty.rects[i], dirty.rects[j]);
                        dirty.rects[j] = dirty.rects[dirty.count - 1];
                        --dirty.count;
                        changed = true;
                        break;
                    }
                }
            }
        }
    }

    void GUI::loopTiled(uint32_t now)
    {
        if (!_disp.display || !_flags.spriteEnabled)
            return;

        const bool adaptivePreview = adaptivePreviewActive();
        const uint8_t rotationDelta = logicalRotationActive() ? logicalRotationDelta() : 0;
        const bool tiledRotation = rotationDelta != 0;
        const int16_t sw = (int16_t)_render.screenWidth;
        const int16_t sh = (int16_t)_render.screenHeight;
        const int16_t tileH = _render.sprite.height();
        const int16_t stride = _render.sprite.width();
        auto *buf = static_cast<uint16_t *>(_render.sprite.getBuffer());
        if (!buf || sw <= 0 || sh <= 0 || tileH <= 0 || stride <= 0)
            return;

        ScreenCallback currentCb = (_screen.current < _screen.capacity && _screen.callbacks)
                                       ? _screen.callbacks[_screen.current]
                                       : nullptr;

        if (_screen.current < _screen.capacity)
            flushPendingGraphRender(_screen.current);

#if PIPGUI_SCREENSHOTS && (PIPGUI_SCREENSHOT_MODE == 2)
        const bool galleryHot = (_shots.lastUseMs != 0) && ((now - _shots.lastUseMs) <= 250);
        if (galleryHot)
        {
            serviceScreenshotGalleryFlash();
        }
        else if (_shots.lastUseMs != 0)
        {
            releaseScreenshotGalleryCache(platform());
        }
#endif
        const bool overlaysFullFrame = _flags.notifActive;
        const DirtyRect prevPopup = _popup.lastRect;
        const bool hadPrevPopup = _popup.lastRectValid;
        const DirtyRect prevToast = _toast.lastRect;
        const bool hadPrevToast = _toast.lastRectValid;
        DirtyRect curPopup = {};
        const uint32_t popupDur = _popup.animDurationMs ? _popup.animDurationMs : 1;
        const bool popupCloseFinished = _flags.popupClosing && (now - _popup.startMs) >= popupDur;
        if (popupCloseFinished)
        {
            _flags.popupActive = 0;
            _flags.popupClosing = 0;
            requestRedraw();
        }
        const bool curPopupVisible = (_flags.popupActive || hadPrevPopup) ? computePopupBounds(now, curPopup) : false;

        DirtyRect curToast = {};
        const bool curToastVisible = (_flags.toastActive || hadPrevToast) ? computeToastBounds(now, curToast) : false;

        if (_dirty.count > 1)
            compactDirty(_dirty, sw, sh);

        const auto rectEmpty = [](const DirtyRect &r) noexcept
        {
            return r.w <= 0 || r.h <= 0;
        };
        const auto rectUnion = [&](const DirtyRect &a, const DirtyRect &b) noexcept -> DirtyRect
        {
            if (rectEmpty(a))
                return b;
            if (rectEmpty(b))
                return a;
            const int16_t x1 = (a.x < b.x) ? a.x : b.x;
            const int16_t y1 = (a.y < b.y) ? a.y : b.y;
            const int16_t x2 = (int16_t)std::max<int32_t>((int32_t)a.x + a.w, (int32_t)b.x + b.w);
            const int16_t y2 = (int16_t)std::max<int32_t>((int32_t)a.y + a.h, (int32_t)b.y + b.h);
            return DirtyRect{x1, y1, (int16_t)(x2 - x1), (int16_t)(y2 - y1)};
        };
        const auto ensureRotationLineBuf = [&](uint16_t needLen) noexcept -> bool
        {
            if (_rotationAnim.lineBufCap < needLen)
            {
                pipcore::Platform *plat = platform();
                uint16_t *newBuf = plat ? static_cast<uint16_t *>(plat->alloc(static_cast<size_t>(needLen) * sizeof(uint16_t), pipcore::AllocCaps::PreferInternal)) : nullptr;
                if (!newBuf)
                {
                    if (plat)
                        (void)detail::recoverFromAllocFailure(plat, static_cast<size_t>(needLen) * sizeof(uint16_t), pipcore::AllocCaps::PreferInternal);
                    newBuf = plat ? static_cast<uint16_t *>(plat->alloc(static_cast<size_t>(needLen) * sizeof(uint16_t), pipcore::AllocCaps::Default)) : nullptr;
                }
                if (!newBuf && plat && detail::recoverFromAllocFailure(plat, static_cast<size_t>(needLen) * sizeof(uint16_t), pipcore::AllocCaps::Default))
                    newBuf = static_cast<uint16_t *>(plat->alloc(static_cast<size_t>(needLen) * sizeof(uint16_t), pipcore::AllocCaps::Default));
                if (newBuf)
                {
                    freeRotationLineBuffer(plat);
                    _rotationAnim.lineBuf = newBuf;
                    _rotationAnim.lineBufCap = needLen;
                }
            }
            return _rotationAnim.lineBuf && _rotationAnim.lineBufCap >= needLen;
        };
        const auto ensureAdaptiveLineBuf = [&](uint16_t needLen) noexcept -> bool
        {
            if (_adaptivePreview.lineBufCap < needLen)
            {
                pipcore::Platform *plat = platform();
                uint16_t *newBuf = plat ? static_cast<uint16_t *>(plat->alloc(static_cast<size_t>(needLen) * sizeof(uint16_t), pipcore::AllocCaps::PreferInternal)) : nullptr;
                if (!newBuf)
                {
                    if (plat)
                        (void)detail::recoverFromAllocFailure(plat, static_cast<size_t>(needLen) * sizeof(uint16_t), pipcore::AllocCaps::PreferInternal);
                    newBuf = plat ? static_cast<uint16_t *>(plat->alloc(static_cast<size_t>(needLen) * sizeof(uint16_t), pipcore::AllocCaps::Default)) : nullptr;
                }
                if (!newBuf && plat && detail::recoverFromAllocFailure(plat, static_cast<size_t>(needLen) * sizeof(uint16_t), pipcore::AllocCaps::Default))
                    newBuf = static_cast<uint16_t *>(plat->alloc(static_cast<size_t>(needLen) * sizeof(uint16_t), pipcore::AllocCaps::Default));
                if (newBuf)
                {
                    freeAdaptivePreviewBuffer(plat);
                    _adaptivePreview.lineBuf = newBuf;
                    _adaptivePreview.lineBufCap = needLen;
                }
            }
            return _adaptivePreview.lineBuf && _adaptivePreview.lineBufCap >= needLen;
        };
        const auto clearAdaptivePreviewShrunkStrips = [&]() noexcept
        {
            if (!adaptivePreview)
                return;

            const uint16_t curOutW = tiledRotation && (rotationDelta & 1U) ? _render.screenHeight : _render.screenWidth;
            const uint16_t curOutH = tiledRotation && (rotationDelta & 1U) ? _render.screenWidth : _render.screenHeight;
            const uint16_t prevW = _adaptivePreview.lastOutputW ? _adaptivePreview.lastOutputW : _adaptivePreview.lastPresentedW;
            const uint16_t prevH = _adaptivePreview.lastOutputH ? _adaptivePreview.lastOutputH : _adaptivePreview.lastPresentedH;
            if ((prevW == 0 && prevH == 0) || (prevW <= curOutW && prevH <= curOutH))
                return;
            const uint16_t clearLineLen = (_render.physicalWidth > _render.physicalHeight) ? _render.physicalWidth : _render.physicalHeight;
            if (!ensureAdaptiveLineBuf(clearLineLen))
                return;

            const uint16_t bg = pipcore::Sprite::swap16(_render.bgColor565);
            for (uint16_t x = 0; x < clearLineLen; ++x)
                _adaptivePreview.lineBuf[x] = bg;

            if (curOutW < prevW)
            {
                const int16_t clearW = static_cast<int16_t>(prevW - curOutW);
                const int16_t clearH = static_cast<int16_t>((prevH > curOutH) ? prevH : curOutH);
                for (int16_t y = 0; y < clearH; ++y)
                {
                    _disp.display->writeRect565((int16_t)curOutW, y, clearW, 1,
                                                _adaptivePreview.lineBuf, clearW);
                }
            }

            if (curOutH < prevH)
            {
                const int16_t clearW = static_cast<int16_t>((prevW > curOutW) ? prevW : curOutW);
                const int16_t clearH = static_cast<int16_t>(prevH - curOutH);
                for (int16_t y = 0; y < clearH; ++y)
                {
                    _disp.display->writeRect565(0, (int16_t)(curOutH + y), clearW, 1,
                                                _adaptivePreview.lineBuf, clearW);
                }
            }
        };
        auto renderTileBand = [&](int16_t tileY, int16_t h)
        {
            _render.originX = 0;
            _render.originY = tileY;
            _render.sprite.setClipRect(0, 0, stride, tileH);

            if (_flags.bootActive)
            {
                renderBootFrame(now);
            }
            else if (_flags.errorActive)
            {
                renderErrorFrame(now);
            }
            else if (_screen.current < _screen.capacity)
            {
                if (currentCb)
                    renderScreenToMainSprite(currentCb, _screen.current);
                else
                    renderScreenToMainSprite(nullptr, _screen.current);

                renderStatusBar();
                if (_flags.notifActive)
                    renderNotificationOverlay();
                if (_flags.popupActive)
                    renderPopupMenuOverlay(now);
                if (_flags.toastActive)
                    renderToastOverlay(now);
            }
            else
            {
                const bool prevRender = _flags.inSpritePass;
                pipcore::Sprite *prevActive = _render.activeSprite;
                _flags.inSpritePass = 1;
                _render.activeSprite = &_render.sprite;
                clear(_render.bgColor565 ? _render.bgColor565 : (uint16_t)_render.bgColor);
                _render.activeSprite = prevActive;
                _flags.inSpritePass = prevRender;
            }
            (void)h;
        };
        const auto presentRotatedTiledFrame = [&](const char *stage) noexcept
        {
            const int16_t physW = (int16_t)_render.physicalWidth;
            const int16_t physH = (int16_t)_render.physicalHeight;
            constexpr int16_t kChunkRows = 8;
            const uint16_t chunkLineLen = static_cast<uint16_t>(std::max<int32_t>(physW,
                                                                                  std::max<int32_t>(sw, tileH) * kChunkRows));
            if (!ensureRotationLineBuf(chunkLineLen))
                return false;

            const int16_t maxBandY = (sh > 0) ? static_cast<int16_t>(((sh - 1) / tileH) * tileH) : 0;
            const bool fullCoverage = ((rotationDelta & 1U) != 0U)
                                          ? (sw == physH && sh == physW)
                                          : (sw == physW && sh == physH);

            (void)fullCoverage;

            switch (rotationDelta & 3U)
            {
            case 1:
                for (int16_t bandY = maxBandY; bandY >= 0; bandY = (bandY >= tileH) ? static_cast<int16_t>(bandY - tileH) : static_cast<int16_t>(-1))
                {
                    const int16_t bandH = (int16_t)std::min<int32_t>(tileH, (int32_t)sh - bandY);
                    const int16_t xStart = (int16_t)(sh - bandY - bandH);
                    renderTileBand(bandY, bandH);

                    const int16_t dstH = static_cast<int16_t>(std::min<int32_t>(physH, sw));
                    for (int16_t y0 = 0; y0 < dstH; y0 = static_cast<int16_t>(y0 + kChunkRows))
                    {
                        const int16_t chunkH = static_cast<int16_t>(std::min<int32_t>(kChunkRows, dstH - y0));
                        for (int16_t row = 0; row < chunkH; ++row)
                        {
                            const int16_t y = static_cast<int16_t>(y0 + row);
                            uint16_t *dst = _rotationAnim.lineBuf + static_cast<size_t>(row) * bandH;
                            for (int16_t i = 0; i < bandH; ++i)
                                dst[i] = buf[(size_t)(bandH - 1 - i) * stride + (size_t)y];
                        }
                        _disp.display->writeRect565(xStart, y0, bandH, chunkH, _rotationAnim.lineBuf, bandH);
                    }
                }
                break;

            case 2:
                for (int16_t bandY = 0; bandY < sh; bandY = (int16_t)(bandY + tileH))
                {
                    const int16_t bandH = (int16_t)std::min<int32_t>(tileH, (int32_t)sh - bandY);
                    renderTileBand(bandY, bandH);

                    const int16_t dstStartY = static_cast<int16_t>(sh - bandY - bandH);
                    for (int16_t y0 = 0; y0 < bandH; y0 = static_cast<int16_t>(y0 + kChunkRows))
                    {
                        const int16_t chunkH = static_cast<int16_t>(std::min<int32_t>(kChunkRows, bandH - y0));
                        for (int16_t row = 0; row < chunkH; ++row)
                        {
                            const int16_t dstY = static_cast<int16_t>(dstStartY + y0 + row);
                            const int16_t localY = static_cast<int16_t>(sh - 1 - dstY - bandY);
                            const uint16_t *srcRow = buf + (size_t)localY * stride;
                            uint16_t *dst = _rotationAnim.lineBuf + static_cast<size_t>(row) * sw;
                            for (int16_t x = 0; x < sw; ++x)
                                dst[x] = srcRow[sw - 1 - x];
                        }
                        _disp.display->writeRect565(0, static_cast<int16_t>(dstStartY + y0), sw, chunkH, _rotationAnim.lineBuf, sw);
                    }
                }
                break;

            case 3:
                for (int16_t bandY = 0; bandY < sh; bandY = (int16_t)(bandY + tileH))
                {
                    const int16_t bandH = (int16_t)std::min<int32_t>(tileH, (int32_t)sh - bandY);
                    const int16_t xStart = bandY;
                    renderTileBand(bandY, bandH);

                    const int16_t dstH = static_cast<int16_t>(std::min<int32_t>(physH, sw));
                    for (int16_t y0 = 0; y0 < dstH; y0 = static_cast<int16_t>(y0 + kChunkRows))
                    {
                        const int16_t chunkH = static_cast<int16_t>(std::min<int32_t>(kChunkRows, dstH - y0));
                        for (int16_t row = 0; row < chunkH; ++row)
                        {
                            const int16_t y = static_cast<int16_t>(y0 + row);
                            const int16_t srcX = (int16_t)(sw - 1 - y);
                            uint16_t *dst = _rotationAnim.lineBuf + static_cast<size_t>(row) * bandH;
                            for (int16_t i = 0; i < bandH; ++i)
                                dst[i] = buf[(size_t)i * stride + (size_t)srcX];
                        }
                        _disp.display->writeRect565(xStart, y0, bandH, chunkH, _rotationAnim.lineBuf, bandH);
                    }
                }
                break;
            }

            reportPlatformErrorOnce(stage);
            pipcore::Platform *plat = platform();
            return !plat || plat->lastError() == pipcore::PlatformError::None;
        };

        if (!adaptivePreview && !overlaysFullFrame && (curPopupVisible || hadPrevPopup || curToastVisible || hadPrevToast))
        {
            DirtyRect paint = {};
            bool paintSet = false;
            const auto expandPaint = [&](const DirtyRect &r) noexcept
            {
                if (rectEmpty(r))
                    return;
                paint = paintSet ? rectUnion(paint, r) : r;
                paintSet = true;
            };

            for (uint8_t i = 0; i < _dirty.count; ++i)
                expandPaint(_dirty.rects[i]);
            if (curPopupVisible)
                expandPaint(curPopup);
            if (hadPrevPopup)
                expandPaint(prevPopup);
            if (curToastVisible)
                expandPaint(curToast);
            if (hadPrevToast)
                expandPaint(prevToast);

            if (paintSet)
            {
                tiledRenderAndPresentRect(paint.x, paint.y, paint.w, paint.h, "tiled-overlay", [&]()
                                          {
                                              if (_screen.current < _screen.capacity)
                                              {
                                                  if (currentCb)
                                                      renderScreenToMainSprite(currentCb, _screen.current);
                                                  else
                                                      clear(_render.bgColor565 ? _render.bgColor565 : (uint16_t)_render.bgColor);
                                                  renderStatusBar();
                                                  if (curPopupVisible)
                                                      renderPopupMenuOverlay(now);
                                                  if (curToastVisible)
                                                      renderToastOverlay(now);
                                              }
                                              else
                                              {
                                                  clear(_render.bgColor565 ? _render.bgColor565 : (uint16_t)_render.bgColor);
                                              } });

                _dirty.count = 0;
                Debug::clearRects();
                _popup.lastRect = curPopup;
                _popup.lastRectValid = curPopupVisible;
                _toast.lastRect = curToast;
                _toast.lastRectValid = curToastVisible;
                return;
            }
        }

        _popup.lastRect = curPopup;
        _popup.lastRectValid = curPopupVisible;
        _toast.lastRect = curToast;
        _toast.lastRectValid = curToastVisible;

        if (!_flags.needRedraw && _screen.current < _screen.capacity)
            flushPendingGraphRender(_screen.current);
        if (!_flags.needRedraw && statusBarAnimationActive())
            updateStatusBar();

        const bool overlaysActive = overlaysFullFrame;
        const bool canDirtyRedraw = (_flags.needRedraw && _flags.dirtyRedrawPending && _dirty.count > 0 &&
                                     !_flags.bootActive && !_flags.errorActive && !overlaysActive);
        const bool rotatedPresentNeeded = tiledRotation && (_flags.needRedraw || _flags.dirtyRedrawPending || _dirty.count > 0);
        const bool fullRedraw = (_flags.bootActive || _flags.errorActive || overlaysActive || rotatedPresentNeeded || (_flags.needRedraw && !canDirtyRedraw));
        const bool debugDirty = Debug::dirtyRectEnabled();
        const uint16_t debugCol = debugDirty ? pipcore::Sprite::swap16(Debug::dirtyRectActiveColor()) : 0;

        auto drawDirtyOverlayTile = [&](const DirtyRect &dr, int16_t tileY, int16_t tileBottom) noexcept
        {
            if (!debugDirty)
                return;
            if (dr.w <= 0 || dr.h <= 0)
                return;

            const int32_t rectX0 = dr.x;
            const int32_t rectY0 = dr.y;
            const int32_t rectX1 = (int32_t)dr.x + dr.w;
            const int32_t rectY1 = (int32_t)dr.y + dr.h;
            if (rectX1 <= 0 || rectY1 <= 0 || rectX0 >= sw || rectY0 >= sh)
                return;

            const int16_t ix0 = (int16_t)std::max<int32_t>(0, rectX0);
            const int16_t ix1 = (int16_t)std::min<int32_t>(sw, rectX1);
            const int16_t iy0 = (int16_t)std::max<int32_t>(tileY, rectY0);
            const int16_t iy1 = (int16_t)std::min<int32_t>(tileBottom, rectY1);
            if (ix1 <= ix0 || iy1 <= iy0)
                return;

            const int16_t localY0 = (int16_t)(iy0 - tileY);
            const int16_t localY1 = (int16_t)(iy1 - tileY);
            if (localY1 <= 0 || localY0 >= tileH)
                return;

            const int16_t topY = (int16_t)rectY0;
            const int16_t bottomY = (int16_t)(rectY1 - 1);
            const int16_t leftX = (int16_t)rectX0;
            const int16_t rightX = (int16_t)(rectX1 - 1);

            auto plot = [&](int16_t x, int16_t y) noexcept
            {
                if ((uint16_t)x >= (uint16_t)sw)
                    return;
                if ((uint16_t)y >= (uint16_t)tileH)
                    return;
                buf[(int32_t)y * stride + x] = debugCol;
            };

            if (topY >= tileY && topY < tileBottom)
            {
                const int16_t ly = (int16_t)(topY - tileY);
                for (int16_t x = ix0; x < ix1; ++x)
                    plot(x, ly);
            }
            if (bottomY >= tileY && bottomY < tileBottom)
            {
                const int16_t ly = (int16_t)(bottomY - tileY);
                for (int16_t x = ix0; x < ix1; ++x)
                    plot(x, ly);
            }

            if (leftX >= 0 && leftX < sw)
            {
                if (leftX >= ix0 && leftX < ix1)
                {
                    for (int16_t y = localY0; y < localY1; ++y)
                        plot(leftX, y);
                }
            }
            if (rightX >= 0 && rightX < sw)
            {
                if (rightX >= ix0 && rightX < ix1)
                {
                    for (int16_t y = localY0; y < localY1; ++y)
                        plot(rightX, y);
                }
            }
        };

        auto restoreAfterPresent = [&](const ClipState &prevClip,
                                       int32_t prevClipX, int32_t prevClipY,
                                       int32_t prevClipW, int32_t prevClipH) noexcept
        {
            _clip = prevClip;
            _render.sprite.setClipRect((int16_t)prevClipX, (int16_t)prevClipY, (int16_t)prevClipW, (int16_t)prevClipH);
            _render.originX = 0;
            _render.originY = 0;
        };

        const ClipState savedClip = _clip;
        int32_t savedClipX = 0, savedClipY = 0, savedClipW = 0, savedClipH = 0;
        _render.sprite.getClipRect(&savedClipX, &savedClipY, &savedClipW, &savedClipH);
        _clip.enabled = false;

        if (fullRedraw)
        {
            _flags.needRedraw = 0;
            _flags.dirtyRedrawPending = 0;
            _dirty.count = 0;
            Debug::clearRects();
            clearAdaptivePreviewShrunkStrips();

            if (tiledRotation)
            {
                (void)presentRotatedTiledFrame("tiled-rotated-present");
                if (adaptivePreview)
                {
                    _adaptivePreview.lastPresentedW = _render.screenWidth;
                    _adaptivePreview.lastPresentedH = _render.screenHeight;
                    _adaptivePreview.lastOutputW = (rotationDelta & 1U) ? _render.screenHeight : _render.screenWidth;
                    _adaptivePreview.lastOutputH = (rotationDelta & 1U) ? _render.screenWidth : _render.screenHeight;
                }
                restoreAfterPresent(savedClip, savedClipX, savedClipY, savedClipW, savedClipH);
                return;
            }

            for (int16_t tileY = 0; tileY < sh; tileY = (int16_t)(tileY + tileH))
            {
                const int16_t h = (int16_t)std::min<int32_t>(tileH, (int32_t)sh - tileY);
                renderTileBand(tileY, h);

                Debug::drawOverlay(buf, stride, 0, tileY, sw, h, tileY);
                _disp.display->writeRect565(0, tileY, sw, h, buf, stride);
                reportPlatformErrorOnce("tiled-present");
            }

            if (adaptivePreview)
            {
                _adaptivePreview.lastPresentedW = _render.screenWidth;
                _adaptivePreview.lastPresentedH = _render.screenHeight;
                _adaptivePreview.lastOutputW = _render.screenWidth;
                _adaptivePreview.lastOutputH = _render.screenHeight;
            }

            restoreAfterPresent(savedClip, savedClipX, savedClipY, savedClipW, savedClipH);
            return;
        }

        if (canDirtyRedraw)
        {
            _flags.needRedraw = 0;
            _flags.dirtyRedrawPending = 0;
        }

        if (_dirty.count == 0)
        {
            restoreAfterPresent(savedClip, savedClipX, savedClipY, savedClipW, savedClipH);
            return;
        }

        const ClipState prevClip = _clip;
        int32_t prevClipX = 0;
        int32_t prevClipY = 0;
        int32_t prevClipW = 0;
        int32_t prevClipH = 0;
        _render.sprite.getClipRect(&prevClipX, &prevClipY, &prevClipW, &prevClipH);

        clearAdaptivePreviewShrunkStrips();

        for (int16_t tileY = 0; tileY < sh; tileY = (int16_t)(tileY + tileH))
        {
            _render.originX = 0;
            _render.originY = tileY;
            _render.sprite.setClipRect(0, 0, stride, tileH);

            const int16_t tileBottom = (int16_t)std::min<int32_t>(sh, (int32_t)tileY + tileH);
            bool tileHasDirty = false;
            int16_t clipUx1 = sw;
            int16_t clipUy1 = sh;
            int16_t clipUx2 = 0;
            int16_t clipUy2 = 0;
            for (uint8_t i = 0; i < _dirty.count; ++i)
            {
                const DirtyRect &dirty = _dirty.rects[i];
                if (dirty.w <= 0 || dirty.h <= 0)
                    continue;

                const int16_t x1 = dirty.x < 0 ? (int16_t)0 : dirty.x;
                const int16_t y1 = dirty.y < tileY ? tileY : dirty.y;
                const int16_t x2 = (int16_t)std::min<int32_t>(sw, (int32_t)dirty.x + dirty.w);
                const int16_t y2 = (int16_t)std::min<int32_t>(tileBottom, (int32_t)dirty.y + dirty.h);
                if (x2 <= x1 || y2 <= y1)
                    continue;

                tileHasDirty = true;
                if (x1 < clipUx1)
                    clipUx1 = x1;
                if (y1 < clipUy1)
                    clipUy1 = y1;
                if (x2 > clipUx2)
                    clipUx2 = x2;
                if (y2 > clipUy2)
                    clipUy2 = y2;
            }

            if (!tileHasDirty)
                continue;

            if (clipUx1 < 0)
                clipUx1 = 0;
            if (clipUy1 < 0)
                clipUy1 = 0;
            if (clipUx2 > sw)
                clipUx2 = sw;
            if (clipUy2 > sh)
                clipUy2 = sh;

            const int16_t clipUw = (int16_t)(clipUx2 - clipUx1);
            const int16_t clipUh = (int16_t)(clipUy2 - clipUy1);
            _clip = prevClip;
            if (clipUw > 0 && clipUh > 0)
                applyClip(clipUx1, clipUy1, clipUw, clipUh);

            if (_screen.current < _screen.capacity)
            {
                if (currentCb)
                    renderScreenToMainSprite(currentCb, _screen.current);
                else
                    renderScreenToMainSprite(nullptr, _screen.current);
                renderStatusBar();
                if (_flags.popupActive)
                    renderPopupMenuOverlay(now);
                if (_flags.toastActive)
                    renderToastOverlay(now);
            }
            else
            {
                const bool prevRender = _flags.inSpritePass;
                pipcore::Sprite *prevActive = _render.activeSprite;
                _flags.inSpritePass = 1;
                _render.activeSprite = &_render.sprite;
                clear(_render.bgColor565 ? _render.bgColor565 : (uint16_t)_render.bgColor);
                _render.activeSprite = prevActive;
                _flags.inSpritePass = prevRender;
            }

            for (uint8_t i = 0; i < _dirty.count; ++i)
            {
                const DirtyRect &dirty = _dirty.rects[i];
                if (dirty.w <= 0 || dirty.h <= 0)
                    continue;

                const int16_t x1 = dirty.x < 0 ? (int16_t)0 : dirty.x;
                const int16_t y1 = dirty.y < tileY ? tileY : dirty.y;
                const int16_t x2 = (int16_t)std::min<int32_t>(sw, (int32_t)dirty.x + dirty.w);
                const int16_t y2 = (int16_t)std::min<int32_t>(tileBottom, (int32_t)dirty.y + dirty.h);
                const int16_t w = (int16_t)(x2 - x1);
                const int16_t h = (int16_t)(y2 - y1);
                if (w <= 0 || h <= 0)
                    continue;

                drawDirtyOverlayTile(dirty, tileY, tileBottom);

                const int16_t srcY = (int16_t)(y1 - tileY);
                Debug::drawOverlay(buf, stride, x1, y1, w, h, tileY);
                _disp.display->writeRect565(x1, y1, w, h, buf + (size_t)srcY * stride + x1, stride);
                reportPlatformErrorOnce("tiled-present-dirty");
            }
        }

        _dirty.count = 0;
        Debug::clearRects();
        if (adaptivePreview)
        {
            _adaptivePreview.lastPresentedW = _render.screenWidth;
            _adaptivePreview.lastPresentedH = _render.screenHeight;
            _adaptivePreview.lastOutputW = (rotationDelta & 1U) ? _render.screenHeight : _render.screenWidth;
            _adaptivePreview.lastOutputH = (rotationDelta & 1U) ? _render.screenWidth : _render.screenHeight;
        }
        restoreAfterPresent(savedClip, savedClipX, savedClipY, savedClipW, savedClipH);
    }

    void GUI::loop()
    {
        uint32_t now = nowMs();
        serviceAdaptivePreview(now);

        if (rotationTransitionActive())
        {
            renderRotationTransition(now);
            return;
        }

        Debug::update();

#if PIPGUI_OTA
        otaService();
#elif PIPGUI_WIFI
        net::wifiService();
#endif

#if PIPGUI_SCREENSHOTS
        serviceScreenshotStream();
#endif

#if PIPGUI_OTA
        {
            constexpr uint8_t kOtaConfirmOkFrames = 30;
            const OtaStatus &st = otaStatus();
            if (!st.pendingVerify)
            {
                _diag.otaOkFrames = 0;
                _diag.otaAutoConfirmed = false;
            }
            else if (!_diag.otaAutoConfirmed)
            {
                pipcore::Platform *plat = pipcore::GetPlatform();
                const bool ok = (!_flags.bootActive &&
                                 !_flags.errorActive &&
                                 !_flags.screenTransition &&
                                 _disp.display &&
                                 (!plat || plat->lastError() == pipcore::PlatformError::None));

                if (ok)
                {
                    if (_diag.otaOkFrames < 255)
                        ++_diag.otaOkFrames;
                    if (_diag.otaOkFrames >= kOtaConfirmOkFrames)
                    {
                        otaMarkAppValid();
                        _diag.otaOkFrames = 0;
                        _diag.otaAutoConfirmed = !otaStatus().pendingVerify;
                    }
                }
                else
                {
                    _diag.otaOkFrames = 0;
                }
            }
        }
#endif

        const auto presentOverlaysFull = [&]()
        {
            bool wroteOverlay = false;
            if (_flags.notifActive)
            {
                renderNotificationOverlay();
                wroteOverlay = true;
            }
            if (_flags.toastActive)
            {
                renderToastOverlay(now);
                wroteOverlay = true;
                DirtyRect r = {};
                const bool vis = computeToastBounds(now, r);
                _toast.lastRect = r;
                _toast.lastRectValid = vis;
            }
            if (wroteOverlay && _flags.spriteEnabled && _disp.display)
            {
                presentSprite(0, 0, (int16_t)_render.screenWidth, (int16_t)_render.screenHeight, "present");
                _dirty.count = 0;
                Debug::clearRects();
            }
        };

        (void)tryPromoteAutoTiledCanvas(now);

        if (_flags.tiledMode)
        {
            loopTiled(now);
            return;
        }

        if (_flags.bootActive)
        {
            renderBootFrame(now);
            if (_disp.display && _flags.spriteEnabled)
                presentSprite(0, 0, (int16_t)_render.screenWidth, (int16_t)_render.screenHeight, "present");
            presentOverlaysFull();
            return;
        }
        if (_flags.errorActive)
        {
            renderErrorFrame(now);
            if (_disp.display && _flags.spriteEnabled)
                presentSprite(0, 0, (int16_t)_render.screenWidth, (int16_t)_render.screenHeight, "present");
            presentOverlaysFull();
            return;
        }
        if (_flags.screenTransition)
        {
            renderScreenTransition(now);
            return;
        }

        if (!_flags.needRedraw && _screen.current < _screen.capacity)
            flushPendingGraphRender(_screen.current);
        if (!_flags.needRedraw && statusBarAnimationActive())
            updateStatusBar();

        ScreenCallback currentCb = (_screen.current < _screen.capacity && _screen.callbacks)
                                       ? _screen.callbacks[_screen.current]
                                       : nullptr;
        const auto renderCurrentScreenDirty = [&](ScreenCallback cb, uint8_t screenId)
        {
            if (_dirty.count == 0)
                return;

            const bool prevRender = _flags.inSpritePass;
            pipcore::Sprite *prevActive = _render.activeSprite;
            const uint8_t prevCurrent = _screen.current;
            const ClipState prevClip = _clip;
            int32_t prevClipX = 0;
            int32_t prevClipY = 0;
            int32_t prevClipW = 0;
            int32_t prevClipH = 0;
            _render.sprite.getClipRect(&prevClipX, &prevClipY, &prevClipW, &prevClipH);

            _flags.inSpritePass = 1;
            _render.activeSprite = &_render.sprite;
            _screen.current = screenId;

            beginGraphFrame(screenId);
            for (uint8_t i = 0; i < _dirty.count; ++i)
            {
                const DirtyRect &dirty = _dirty.rects[i];
                if (dirty.w <= 0 || dirty.h <= 0)
                    continue;

                _clip = prevClip;
                applyClip(dirty.x, dirty.y, dirty.w, dirty.h);
                clear(_render.bgColor565 ? _render.bgColor565 : (uint16_t)_render.bgColor);
                if (cb)
                    cb(*this);
            }
            endGraphFrame(screenId);

            _clip = prevClip;
            _render.sprite.setClipRect((int16_t)prevClipX, (int16_t)prevClipY, (int16_t)prevClipW, (int16_t)prevClipH);
            _screen.current = prevCurrent;
            _render.activeSprite = prevActive;
            _flags.inSpritePass = prevRender;
        };

        const auto serviceOverlays = [&](bool forceFullPresent)
        {
            if (_flags.notifActive)
                return false;
            if (!_flags.spriteEnabled || !_disp.display)
                return false;

            const uint32_t popupDur = _popup.animDurationMs ? _popup.animDurationMs : 1;
            const bool popupCloseFinished = _flags.popupClosing && (now - _popup.startMs) >= popupDur;
            if (popupCloseFinished)
            {
                _flags.popupActive = 0;
                _flags.popupClosing = 0;
            }

            DirtyRect curPopup = {};
            const bool curVisible = computePopupBounds(now, curPopup);
            DirtyRect curToast = {};
            const bool curToastVisible = computeToastBounds(now, curToast);
            if (!curVisible && !_popup.lastRectValid && !curToastVisible && !_toast.lastRectValid && !forceFullPresent && _dirty.count == 0)
                return false;

            bool paintSet = forceFullPresent;
            DirtyRect paint = {0, 0, (int16_t)_render.screenWidth, (int16_t)_render.screenHeight};
            const auto expandPaint = [&](const DirtyRect &rect)
            {
                if (rect.w <= 0 || rect.h <= 0)
                    return;
                if (!paintSet)
                {
                    paint = rect;
                    paintSet = true;
                    return;
                }
                const int16_t x1 = (paint.x < rect.x) ? paint.x : rect.x;
                const int16_t y1 = (paint.y < rect.y) ? paint.y : rect.y;
                const int16_t x2a = (int16_t)(paint.x + paint.w);
                const int16_t x2b = (int16_t)(rect.x + rect.w);
                const int16_t y2a = (int16_t)(paint.y + paint.h);
                const int16_t y2b = (int16_t)(rect.y + rect.h);
                paint.x = x1;
                paint.y = y1;
                paint.w = ((x2a > x2b) ? x2a : x2b) - x1;
                paint.h = ((y2a > y2b) ? y2a : y2b) - y1;
            };

            for (uint8_t i = 0; i < _dirty.count; ++i)
                expandPaint(_dirty.rects[i]);
            if (curVisible)
                expandPaint(curPopup);
            if (_popup.lastRectValid)
                expandPaint(_popup.lastRect);
            if (curToastVisible)
                expandPaint(curToast);
            if (_toast.lastRectValid)
                expandPaint(_toast.lastRect);

            if (!paintSet)
                return false;

            if (currentCb)
                renderScreenToMainSprite(currentCb, _screen.current);
            else
                clear(_render.bgColor565 ? _render.bgColor565 : (uint16_t)_render.bgColor);
            renderStatusBar();
            if (curVisible)
                renderPopupMenuOverlay(now);
            if (curToastVisible)
                renderToastOverlay(now);

            presentSprite(paint.x, paint.y, paint.w, paint.h, "present");
            _dirty.count = 0;
            Debug::clearRects();

            _popup.lastRect = curPopup;
            _popup.lastRectValid = curVisible;
            _toast.lastRect = curToast;
            _toast.lastRectValid = curToastVisible;
            return true;
        };

        if (_flags.notifActive && _flags.spriteEnabled)
        {
            if (currentCb)
            {
                renderScreenToMainSprite(currentCb, _screen.current);
                renderStatusBar();
            }
            presentOverlaysFull();
            return;
        }

        if (_flags.needRedraw && currentCb)
        {
            _flags.needRedraw = 0;
            if (_flags.spriteEnabled)
            {
                if (_disp.display)
                {
                    ListState *list = getList(_screen.current);
                    TileState *tile = getTile(_screen.current);
                    const bool currentIsList = list && list->configured && list->itemCount > 0;
                    const bool currentIsTile = tile && tile->configured && tile->itemCount > 0;
                    const bool overlaysActive = _flags.toastActive || _toast.lastRectValid || _flags.notifActive || _flags.popupActive;

                    if (!overlaysActive && currentIsList)
                    {
                        updateListScreen(_screen.current);
                        renderStatusBar();
                        _flags.dirtyRedrawPending = 0;
                        if (logicalRotationActive())
                        {
                            presentSprite(0, 0, (int16_t)_render.screenWidth, (int16_t)_render.screenHeight, "present");
                            _dirty.count = 0;
                            Debug::clearRects();
                            return;
                        }
                        if (_dirty.count > 0)
                        {
                            if (_dirty.count > 1)
                                compactDirty(_dirty, (int16_t)_render.screenWidth, (int16_t)_render.screenHeight);
                            flushDirty();
                        }
                        return;
                    }

                    if (!overlaysActive && currentIsTile)
                    {
                        updateTile(_screen.current, tile->selectedIndex);
                        renderStatusBar();
                        _flags.dirtyRedrawPending = 0;
                        if (logicalRotationActive())
                        {
                            presentSprite(0, 0, (int16_t)_render.screenWidth, (int16_t)_render.screenHeight, "present");
                            _dirty.count = 0;
                            Debug::clearRects();
                            return;
                        }
                        if (_dirty.count > 0)
                        {
                            if (_dirty.count > 1)
                                compactDirty(_dirty, (int16_t)_render.screenWidth, (int16_t)_render.screenHeight);
                            flushDirty();
                        }
                        return;
                    }

                    if (!overlaysActive && _flags.dirtyRedrawPending && _dirty.count > 0)
                    {
                        if (_dirty.count > 1)
                            compactDirty(_dirty, (int16_t)_render.screenWidth, (int16_t)_render.screenHeight);
                        renderCurrentScreenDirty(currentCb, _screen.current);
                        renderStatusBar();
                        _flags.dirtyRedrawPending = 0;
                        if (logicalRotationActive())
                        {
                            presentSprite(0, 0, (int16_t)_render.screenWidth, (int16_t)_render.screenHeight, "present");
                            _dirty.count = 0;
                            Debug::clearRects();
                            return;
                        }
                        if (_dirty.count > 0)
                        {
                            if (_dirty.count > 1)
                                compactDirty(_dirty, (int16_t)_render.screenWidth, (int16_t)_render.screenHeight);
                            flushDirty();
                        }
                        return;
                    }

                    _flags.dirtyRedrawPending = 0;
                    renderScreenToMainSprite(currentCb, _screen.current);
                    renderStatusBar();

                    if (_flags.popupActive || _popup.lastRectValid || _flags.toastActive || _toast.lastRectValid)
                    {
                        serviceOverlays(true);
                        return;
                    }

                    presentSprite(0, 0, (int16_t)_render.screenWidth, (int16_t)_render.screenHeight, "present");
                    if (!_flags.dirtyRedrawPending || _dirty.count == 0)
                    {
                        _dirty.count = 0;
                        Debug::clearRects();
                    }
                    return;
                }
                else
                {
                    beginGraphFrame(_screen.current);
                    currentCb(*this);
                    endGraphFrame(_screen.current);
                    renderStatusBar();
                }
            }
            else
            {
                beginGraphFrame(_screen.current);
                currentCb(*this);
                endGraphFrame(_screen.current);
                renderStatusBar();
            }
        }

        if (_flags.notifActive && !_flags.spriteEnabled)
            renderNotificationOverlay();
        if (_flags.popupActive && !_flags.spriteEnabled)
            renderPopupMenuOverlay(now);
        if (_flags.popupActive || _popup.lastRectValid || _flags.toastActive || _toast.lastRectValid)
            serviceOverlays(false);
        if (!_flags.needRedraw && _dirty.count > 0 && _flags.spriteEnabled && _disp.display &&
            !_flags.toastActive && !_toast.lastRectValid && !_flags.popupActive && !_popup.lastRectValid)
        {
            if (_dirty.count > 1)
                compactDirty(_dirty, (int16_t)_render.screenWidth, (int16_t)_render.screenHeight);
            flushDirty();
        }

        if (_blur.lastUseMs != 0 && (now - _blur.lastUseMs) > 250)
        {
            freeBlurBuffers(platform());
            _blur.lastUseMs = 0;
        }
    }

    void GUI::loopWithInput(Button &next, Button &prev)
    {
        const InputState input = pollInput(next, prev);
        const uint8_t manual = _manualInputMask;

        if (_flags.errorActive)
        {
            if (!(manual & ManualInput_Error))
                setErrorButtonsDown(input.nextDown, input.prevDown, input.comboDown);
        }
        else if (_flags.notifActive)
        {
            if (!(manual & ManualInput_Notif))
            {
                const bool confirmDown = input.hasSelect ? (input.selectDown || input.prevDown) : input.prevDown;
                setNotificationButtonDown(confirmDown);
            }
        }
        else if (_flags.popupActive)
        {
            if (!(manual & ManualInput_Popup))
                handlePopupMenuInput(input);
        }
        else if (_screen.current < _screen.capacity)
        {
            (void)dispatchNavInput(_screen.current, input);
        }

        loop();
    }

    void GUI::loopWithInput(Button &next, Button &prev, Button &select)
    {
        const InputState input = pollInput(next, prev, select);
        const uint8_t manual = _manualInputMask;

        if (_flags.errorActive)
        {
            if (!(manual & ManualInput_Error))
                setErrorButtonsDown(input.nextDown, input.prevDown, input.comboDown);
        }
        else if (_flags.notifActive)
        {
            if (!(manual & ManualInput_Notif))
            {
                const bool confirmDown = input.hasSelect ? (input.selectDown || input.prevDown) : input.prevDown;
                setNotificationButtonDown(confirmDown);
            }
        }
        else if (_flags.popupActive)
        {
            if (!(manual & ManualInput_Popup))
                handlePopupMenuInput(input);
        }
        else if (_screen.current < _screen.capacity)
        {
            (void)dispatchNavInput(_screen.current, input);
        }

        loop();
    }

    void GUI::loopWithPolledInput()
    {
        const InputState input = _input;
        const uint8_t manual = _manualInputMask;

        if (_flags.errorActive)
        {
            if (!(manual & ManualInput_Error))
                setErrorButtonsDown(input.nextDown, input.prevDown, input.comboDown);
        }
        else if (_flags.notifActive)
        {
            if (!(manual & ManualInput_Notif))
            {
                const bool confirmDown = input.hasSelect ? (input.selectDown || input.prevDown) : input.prevDown;
                setNotificationButtonDown(confirmDown);
            }
        }
        else if (_flags.popupActive)
        {
            if (!(manual & ManualInput_Popup))
                handlePopupMenuInput(input);
        }
        else if (_screen.current < _screen.capacity)
        {
            (void)dispatchNavInput(_screen.current, input);
        }

        loop();
    }
}
