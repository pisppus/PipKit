#include <pipGUI/Core/pipGUI.hpp>
#include <pipGUI/Core/Debug.hpp>
#include <pipGUI/Systems/Network/Wifi.hpp>

namespace pipgui
{
    namespace
    {
        constexpr uint32_t kIdleBlurCacheMs = 250;
        constexpr uint32_t kIdleShotGalleryCacheMs = 250;
    }

    void GUI::loopTiled(uint32_t now)
    {
        if (!_disp.display || !_flags.spriteEnabled)
            return;

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
        // Keep screenshot gallery loading alive even when this function exits early
        // (full redraw / no dirty rects).
        const bool galleryHot = (_shots.lastUseMs != 0) && ((now - _shots.lastUseMs) <= kIdleShotGalleryCacheMs);
        if (galleryHot)
        {
            serviceScreenshotGalleryFlash();
        }
        else if (_shots.lastUseMs != 0)
        {
            releaseScreenshotGalleryCache(platform());
        }
#endif

        // Notification overlay dims the whole screen: keep it full-frame in tiled mode.
        const bool overlaysFullFrame = _flags.notifActive;

        // Popup/toast support dirty redraw in tiled mode: include their bounds in the dirty set.
        DirtyRect curPopup = {};
        const uint32_t popupDur = _popup.animDurationMs ? _popup.animDurationMs : 1;
        const bool popupCloseFinished = _flags.popupClosing && (now - _popup.startMs) >= popupDur;
        if (popupCloseFinished)
        {
            _flags.popupActive = 0;
            _flags.popupClosing = 0;
        }
        const bool curPopupVisible = (_flags.popupActive || _popup.lastRectValid) ? computePopupBounds(now, curPopup) : false;

        DirtyRect curToast = {};
        const bool curToastVisible = (_flags.toastActive || _toast.lastRectValid) ? computeToastBounds(now, curToast) : false;

        auto pushDirty = [&](const DirtyRect &r) noexcept
        {
            if (r.w <= 0 || r.h <= 0)
                return;
            invalidateRect(r.x, r.y, r.w, r.h);
        };
        if (curPopupVisible)
            pushDirty(curPopup);
        if (_popup.lastRectValid)
            pushDirty(_popup.lastRect);
        if (curToastVisible)
            pushDirty(curToast);
        if (_toast.lastRectValid)
            pushDirty(_toast.lastRect);

        if ((curPopupVisible || _popup.lastRectValid || curToastVisible || _toast.lastRectValid) && _dirty.count > 0)
            _flags.dirtyRedrawPending = 1;

        // Persist overlay rects for the next frame (for proper erase when closing/moving).
        _popup.lastRect = curPopup;
        _popup.lastRectValid = curPopupVisible;
        _toast.lastRect = curToast;
        _toast.lastRectValid = curToastVisible;

        const bool overlaysActive = overlaysFullFrame;
        const bool canDirtyRedraw = (_flags.needRedraw && _flags.dirtyRedrawPending && _dirty.count > 0 &&
                                     !_flags.bootActive && !_flags.errorActive && !overlaysActive);
        const bool fullRedraw = (_flags.bootActive || _flags.errorActive || overlaysActive || (_flags.needRedraw && !canDirtyRedraw));
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

            for (int16_t tileY = 0; tileY < sh; tileY = (int16_t)(tileY + tileH))
            {
                const int16_t h = (int16_t)std::min<int32_t>(tileH, (int32_t)sh - tileY);
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

                _disp.display->writeRect565(0, tileY, sw, h, buf, stride);
                reportPlatformErrorOnce("tiled-present");
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

        // Tiled dirty present: re-render dirty regions into each tile and present them.
        for (int16_t tileY = 0; tileY < sh; tileY = (int16_t)(tileY + tileH))
        {
            _render.originX = 0;
            _render.originY = tileY;
            _render.sprite.setClipRect(0, 0, stride, tileH);

            const int16_t tileBottom = (int16_t)std::min<int32_t>(sh, (int32_t)tileY + tileH);

            // Render the tile once using a union clip of all dirty rects that touch this tile.
            // This avoids calling the screen callback multiple times per frame, which can
            // advance stateful widgets (graph samples, animations) more than once and produce
            // visual artifacts (gaps, jitter) in tiled dirty mode.
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
                if (dirty.x < clipUx1)
                    clipUx1 = dirty.x;
                if (dirty.y < clipUy1)
                    clipUy1 = dirty.y;
                const int16_t ex = (int16_t)((int32_t)dirty.x + dirty.w);
                const int16_t ey = (int16_t)((int32_t)dirty.y + dirty.h);
                if (ex > clipUx2)
                    clipUx2 = ex;
                if (ey > clipUy2)
                    clipUy2 = ey;
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
                _disp.display->writeRect565(x1, y1, w, h, buf + (size_t)srcY * stride + x1, stride);
                reportPlatformErrorOnce("tiled-present-dirty");
            }
        }

        _dirty.count = 0;
        Debug::clearRects();

        // Restore external clip state (loopTiled forces clip off during render passes).
        restoreAfterPresent(savedClip, savedClipX, savedClipY, savedClipW, savedClipH);
    }

    void GUI::loop()
    {
        uint32_t now = nowMs();
        _navConsumed = false;
        if (!_flags.tiledMode)
            serviceAdaptivePreview(now);

        if (!_flags.tiledMode && rotationTransitionActive())
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
                            flushDirty();
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
                            flushDirty();
                        return;
                    }

                    if (!overlaysActive && _flags.dirtyRedrawPending && _dirty.count > 0)
                    {
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
                            flushDirty();
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
            flushDirty();

        if (_blur.lastUseMs != 0 && (now - _blur.lastUseMs) > kIdleBlurCacheMs)
        {
            freeBlurBuffers(platform());
            _blur.lastUseMs = 0;
        }
    }

    void GUI::loopWithInput(Button &next, Button &prev)
    {
        const InputState input = pollInput(next, prev);
        bool handled = false;
        const uint8_t manual = _manualInputMask;

        if (_flags.errorActive)
        {
            if (!(manual & ManualInput_Error))
                setErrorButtonsDown(input.nextDown, input.prevDown, input.comboDown);
            handled = true;
        }
        else if (_flags.notifActive)
        {
            if (!(manual & ManualInput_Notif))
            {
                const bool confirmDown = input.hasSelect ? (input.selectDown || input.prevDown) : input.prevDown;
                setNotificationButtonDown(confirmDown);
            }
            handled = true;
        }
        else if (_flags.popupActive)
        {
            if (!(manual & ManualInput_Popup))
                handlePopupMenuInput(input);
            handled = true;
        }
        else if (_screen.current < _screen.capacity)
        {
            ListState *list = getList(_screen.current);
            if (list && list->configured && list->itemCount > 0)
            {
                if (!(manual & ManualInput_List))
                    handleListInput(_screen.current, input);
                handled = true;
            }
            else
            {
                TileState *tile = getTile(_screen.current);
                if (tile && tile->configured && tile->itemCount > 0)
                {
                    if (!(manual & ManualInput_Tile))
                        handleTileInput(_screen.current, input);
                    handled = true;
                }
            }
        }

        if (!handled && !_flags.screenTransition && !_navConsumed)
        {
            if (input.nextPressed)
                nextScreen();
            if (input.prevPressed)
                prevScreen();
        }

        loop();
    }

    void GUI::loopWithInput(Button &next, Button &prev, Button &select)
    {
        const InputState input = pollInput(next, prev, select);
        bool handled = false;
        const uint8_t manual = _manualInputMask;

        if (_flags.errorActive)
        {
            if (!(manual & ManualInput_Error))
                setErrorButtonsDown(input.nextDown, input.prevDown, input.comboDown);
            handled = true;
        }
        else if (_flags.notifActive)
        {
            if (!(manual & ManualInput_Notif))
            {
                const bool confirmDown = input.hasSelect ? (input.selectDown || input.prevDown) : input.prevDown;
                setNotificationButtonDown(confirmDown);
            }
            handled = true;
        }
        else if (_flags.popupActive)
        {
            if (!(manual & ManualInput_Popup))
                handlePopupMenuInput(input);
            handled = true;
        }
        else if (_screen.current < _screen.capacity)
        {
            ListState *list = getList(_screen.current);
            if (list && list->configured && list->itemCount > 0)
            {
                if (!(manual & ManualInput_List))
                    handleListInput(_screen.current, input);
                handled = true;
            }
            else
            {
                TileState *tile = getTile(_screen.current);
                if (tile && tile->configured && tile->itemCount > 0)
                {
                    if (!(manual & ManualInput_Tile))
                        handleTileInput(_screen.current, input);
                    handled = true;
                }
            }
        }

        if (!handled && !_flags.screenTransition && !_navConsumed)
        {
            if (input.nextPressed)
                nextScreen();
            if (input.prevPressed)
                prevScreen();
        }

        loop();
    }

    void GUI::loopWithPolledInput()
    {
        const InputState input = _input;
        bool handled = false;
        const uint8_t manual = _manualInputMask;

        if (_flags.errorActive)
        {
            if (!(manual & ManualInput_Error))
                setErrorButtonsDown(input.nextDown, input.prevDown, input.comboDown);
            handled = true;
        }
        else if (_flags.notifActive)
        {
            if (!(manual & ManualInput_Notif))
            {
                const bool confirmDown = input.hasSelect ? (input.selectDown || input.prevDown) : input.prevDown;
                setNotificationButtonDown(confirmDown);
            }
            handled = true;
        }
        else if (_flags.popupActive)
        {
            if (!(manual & ManualInput_Popup))
                handlePopupMenuInput(input);
            handled = true;
        }
        else if (_screen.current < _screen.capacity)
        {
            ListState *list = getList(_screen.current);
            if (list && list->configured && list->itemCount > 0)
            {
                if (!(manual & ManualInput_List))
                    handleListInput(_screen.current, input);
                handled = true;
            }
            else
            {
                TileState *tile = getTile(_screen.current);
                if (tile && tile->configured && tile->itemCount > 0)
                {
                    if (!(manual & ManualInput_Tile))
                        handleTileInput(_screen.current, input);
                    handled = true;
                }
            }
        }

        if (!handled && !_flags.screenTransition && !_navConsumed)
        {
            if (input.nextPressed)
                nextScreen();
            if (input.prevPressed)
                prevScreen();
        }

        loop();
    }
}
