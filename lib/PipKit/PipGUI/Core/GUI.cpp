#include <cstring>
#include <cmath>

#include <PipCore/Platforms/Select.hpp>

#include <PipGUI/Core/Debug/Debug.hpp>
#include <PipGUI/Core/GUI.hpp>
#include <PipGUI/Core/Config/Version.hpp>
#include <PipGUI/Core/Internal/GuiAccess.hpp>
#include <PipGUI/Core/Internal/ViewModels.hpp>
#include <PipGUI/Widgets/Data/Internal.hpp>

#include <PipGUI/Graphics/Utils/Colors.hpp>
#include <PipGUI/Graphics/Utils/Easing.hpp>

#include <PipGUI/Systems/Network/Wifi.hpp>

namespace pipgui
{
    namespace detail
    {
        static GUI *g_activeGui = nullptr;
        static bool g_recoveringFromAllocFailure = false;

        pipcore::Platform *resolvePlatform(GUI *gui) noexcept
        {
            return gui ? gui->platform() : nullptr;
        }

        bool recoverFromAllocFailure(pipcore::Platform *plat, size_t bytes, pipcore::AllocCaps caps) noexcept
        {
            if (g_recoveringFromAllocFailure)
                return false;
            GUI *gui = g_activeGui;
            if (!gui)
                return false;
            if (plat && gui->platform() && plat != gui->platform())
                return false;

            g_recoveringFromAllocFailure = true;
            const bool recovered = gui->recoverFromAllocationFailure(bytes, caps);
            g_recoveringFromAllocFailure = false;
            return recovered;
        }
    }

    namespace
    {
        uint32_t hashButtonKey(const String &label, int16_t x, int16_t y, int16_t w, int16_t h,
                               uint16_t baseColor, uint8_t radius, IconId iconId) noexcept
        {
            (void)label;
            uint32_t hash = 2166136261u;
            auto mix = [&](uint32_t v)
            {
                hash ^= v;
                hash *= 16777619u;
            };

            mix(static_cast<uint16_t>(x));
            mix(static_cast<uint16_t>(y));
            mix(static_cast<uint16_t>(w));
            mix(static_cast<uint16_t>(h));
            mix(baseColor);
            mix(radius);
            mix(static_cast<uint16_t>(iconId));
            return hash ? hash : 1u;
        }

        uint32_t hashToggleKey(int16_t x, int16_t y, int16_t w, int16_t h,
                               uint16_t activeColor, int32_t inactiveColor, int32_t knobColor) noexcept
        {
            uint32_t hash = 2166136261u;
            auto mix = [&](uint32_t v)
            {
                hash ^= v;
                hash *= 16777619u;
            };

            mix(static_cast<uint16_t>(x));
            mix(static_cast<uint16_t>(y));
            mix(static_cast<uint16_t>(w));
            mix(static_cast<uint16_t>(h));
            mix(activeColor);
            mix(static_cast<uint32_t>(inactiveColor));
            mix(static_cast<uint32_t>(knobColor));
            return hash ? hash : 1u;
        }

        uint32_t hashSliderKey(int16_t x, int16_t y, int16_t w, int16_t h,
                               int16_t minValue, int16_t maxValue, int16_t step,
                               uint16_t activeColor, int32_t inactiveColor, int32_t thumbColor) noexcept
        {
            uint32_t hash = 2166136261u;
            auto mix = [&](uint32_t v)
            {
                hash ^= v;
                hash *= 16777619u;
            };

            mix(static_cast<uint16_t>(x));
            mix(static_cast<uint16_t>(y));
            mix(static_cast<uint16_t>(w));
            mix(static_cast<uint16_t>(h));
            mix(static_cast<uint16_t>(minValue));
            mix(static_cast<uint16_t>(maxValue));
            mix(static_cast<uint16_t>(step));
            mix(activeColor);
            mix(static_cast<uint32_t>(inactiveColor));
            mix(static_cast<uint32_t>(thumbColor));
            return hash ? hash : 1u;
        }

        float drumRollCurrentIndex(const detail::DrumRollAnimState &state, uint32_t now) noexcept
        {
            if (state.startMs == 0 || state.durationMs == 0 || now <= state.startMs)
                return state.fromIndex;

            const uint32_t elapsed = now - state.startMs;
            if (elapsed >= state.durationMs)
                return static_cast<float>(state.toIndex);

            const float t = static_cast<float>(elapsed) / static_cast<float>(state.durationMs);
            const float eased = detail::motion::easeInOutCubic(t);
            return state.fromIndex + (static_cast<float>(state.toIndex) - state.fromIndex) * eased;
        }

        class NullDisplay final : public pipcore::Display
        {
        public:
            bool begin(uint8_t) override { return false; }
            bool setRotation(uint8_t) override { return false; }
            uint16_t width() const noexcept override { return 0; }
            uint16_t height() const noexcept override { return 0; }
            void fillScreen565(uint16_t) override {}
            void writeRect565(int16_t, int16_t, int16_t, int16_t, const uint16_t *, int32_t) override {}
        };

    }

    void GUI::clearReportedPlatformError()
    {
        _diag.lastReportedError = pipcore::PlatformError::None;
    }

    void GUI::reportPlatformErrorOnce(const char *stage)
    {
        (void)stage;
        pipcore::Platform *plat = pipcore::GetPlatform();
        if (!plat)
            return;

        const pipcore::PlatformError error = plat->lastError();
        if (error == pipcore::PlatformError::None)
        {
            clearReportedPlatformError();
            return;
        }

        if (error == _diag.lastReportedError)
            return;

        _diag.lastReportedError = error;
    }

    bool GUI::presentSprite(int16_t x, int16_t y, int16_t w, int16_t h, const char *stage)
    {
        if (!_disp.display || !_flags.spriteEnabled || w <= 0 || h <= 0)
            return false;

        if (adaptivePreviewActive())
            return presentAdaptivePreview(stage);

        if (logicalRotationActive())
        {
            const auto *src = static_cast<const uint16_t *>(_render.sprite.getBuffer());
            return presentOrthogonalRotatedSprite(src,
                                                  _render.sprite.width(),
                                                  (int16_t)_render.screenWidth,
                                                  (int16_t)_render.screenHeight,
                                                  logicalRotationDelta(),
                                                  stage);
        }

        if (uint16_t *buf = static_cast<uint16_t *>(_render.sprite.getBuffer()))
            Debug::drawOverlay(buf, _render.sprite.width(), x, y, w, h);

        _render.sprite.writeToDisplay(*_disp.display, x, y, w, h);
        reportPlatformErrorOnce(stage);

        pipcore::Platform *plat = pipcore::GetPlatform();
        return !plat || plat->lastError() == pipcore::PlatformError::None;
    }

    detail::ButtonState &GUI::resolveButtonState(const String &label, int16_t x, int16_t y,
                                                 int16_t w, int16_t h, uint16_t baseColor, uint8_t radius,
                                                 IconId iconId)
    {
        const uint32_t key = hashButtonKey(label, x, y, w, h, baseColor, radius, iconId);
        const uint32_t now = nowMs();
        detail::ButtonCacheEntry *best = &_buttonCache.entries[0];

        for (uint8_t i = 0; i < detail::BUTTON_CACHE_MAX; ++i)
        {
            detail::ButtonCacheEntry &entry = _buttonCache.entries[i];
            if (entry.used && entry.key == key)
            {
                entry.lastUseMs = now;
                return entry.state;
            }
            if (!entry.used)
                best = &entry;
            else if (best->used && entry.lastUseMs < best->lastUseMs)
                best = &entry;
        }

        best->used = true;
        best->key = key;
        best->lastUseMs = now;
        best->state = {};
        best->state.enabled = true;
        best->state.prevEnabled = true;
        best->state.fadeLevel = 255;
        return best->state;
    }

    detail::ToggleState &GUI::resolveToggleState(int16_t x, int16_t y, int16_t w, int16_t h,
                                                 uint16_t activeColor, int32_t inactiveColor, int32_t knobColor)
    {
        const uint32_t key = hashToggleKey(x, y, w, h, activeColor, inactiveColor, knobColor);
        const uint32_t now = nowMs();
        detail::ToggleCacheEntry *best = &_toggleCache.entries[0];

        for (uint8_t i = 0; i < detail::TOGGLE_CACHE_MAX; ++i)
        {
            detail::ToggleCacheEntry &entry = _toggleCache.entries[i];
            if (entry.used && entry.key == key)
            {
                entry.lastUseMs = now;
                return entry.state;
            }
            if (!entry.used)
                best = &entry;
            else if (best->used && entry.lastUseMs < best->lastUseMs)
                best = &entry;
        }

        best->used = true;
        best->key = key;
        best->lastUseMs = now;
        best->state = {};
        return best->state;
    }

    detail::SliderState &GUI::resolveSliderState(int16_t x, int16_t y, int16_t w, int16_t h,
                                                 int16_t minValue, int16_t maxValue, int16_t step,
                                                 uint16_t activeColor, int32_t inactiveColor, int32_t thumbColor)
    {
        const uint32_t key = hashSliderKey(x, y, w, h, minValue, maxValue, step, activeColor, inactiveColor, thumbColor);
        const uint32_t now = nowMs();
        detail::SliderCacheEntry *best = &_sliderCache.entries[0];

        for (uint8_t i = 0; i < detail::SLIDER_CACHE_MAX; ++i)
        {
            detail::SliderCacheEntry &entry = _sliderCache.entries[i];
            if (entry.used && entry.key == key)
            {
                entry.lastUseMs = now;
                return entry.state;
            }
            if (!entry.used)
                best = &entry;
            else if (best->used && entry.lastUseMs < best->lastUseMs)
                best = &entry;
        }

        best->used = true;
        best->key = key;
        best->lastUseMs = now;
        best->state = {};
        best->state.enabled = true;
        return best->state;
    }

    detail::DrumRollAnimState &GUI::resolveDrumRollState(uint32_t key, uint8_t selectedIndex, uint16_t durationMs)
    {
        const uint32_t now = nowMs();
        detail::DrumRollCacheEntry *best = &_drumRollCache.entries[0];

        for (uint8_t i = 0; i < detail::DRUM_ROLL_CACHE_MAX; ++i)
        {
            detail::DrumRollCacheEntry &entry = _drumRollCache.entries[i];
            if (entry.used && entry.key == key)
            {
                entry.lastUseMs = now;
                detail::DrumRollAnimState &state = entry.state;
                state.durationMs = durationMs > 0 ? durationMs : 1;
                if (state.toIndex != selectedIndex)
                {
                    state.fromIndex = drumRollCurrentIndex(state, now);
                    state.toIndex = selectedIndex;
                    state.startMs = now;
                }
                return state;
            }
            if (!entry.used)
                best = &entry;
            else if (best->used && entry.lastUseMs < best->lastUseMs)
                best = &entry;
        }

        best->used = true;
        best->key = key;
        best->lastUseMs = now;
        best->state = {};
        best->state.fromIndex = static_cast<float>(selectedIndex);
        best->state.toIndex = selectedIndex;
        best->state.durationMs = durationMs > 0 ? durationMs : 1;
        return best->state;
    }

    GUI::InputState GUI::pollInputInternal(Button &next, Button &prev, Button *select)
    {
        next.update();
        prev.update();
        if (select)
            select->update();

        const bool nextDown = next.isDown();
        const bool prevDown = prev.isDown();
        const bool selectDown = select ? select->isDown() : false;
        const bool combo = nextDown && prevDown;

#if PIPGUI_SCREENSHOTS
        handleScreenshotShortcut(combo && !_flags.errorActive);
        if (combo && !_flags.errorActive)
        {
            (void)next.wasPressed();
            (void)prev.wasPressed();
            if (select)
                (void)select->wasPressed();
            InputState out;
            out.comboDown = true;
            out.hasSelect = (select != nullptr);
            _input = out;
            return out;
        }
#endif

        InputState out;
        out.nextDown = nextDown;
        out.prevDown = prevDown;
        out.nextPressed = next.wasPressed();
        out.prevPressed = prev.wasPressed();
        out.selectDown = selectDown;
        out.selectPressed = select ? select->wasPressed() : false;
        out.comboDown = combo;
        out.hasSelect = (select != nullptr);
        _input = out;

        if (out.hasSelect && out.selectPressed &&
            !_flags.errorActive && !_flags.notifActive && !_flags.popupActive && !_flags.screenTransition &&
            _screen.current < _screen.capacity && _screen.graphAreas && _screen.graphAreas[_screen.current])
        {
            GraphArea *area = _screen.graphAreas[_screen.current];
            if (area && area->innerW > 1 && area->innerH > 1)
                setGraphPaused(!graphPaused());
        }

        return out;
    }

    GUI::InputState GUI::pollInput(Button &next, Button &prev)
    {
        _manualInputMask = 0;
        return pollInputInternal(next, prev, nullptr);
    }

    GUI::InputState GUI::pollInput(Button &next, Button &prev, Button &select)
    {
        _manualInputMask = 0;
        return pollInputInternal(next, prev, &select);
    }

    bool GUI::graphPaused() const noexcept
    {
        if (_screen.current >= _screen.capacity || !_screen.graphAreas)
            return false;
        const GraphArea *area = _screen.graphAreas[_screen.current];
        return area ? area->paused : false;
    }

    void GUI::setGraphPaused(bool paused) noexcept
    {
        if (_screen.current >= _screen.capacity || !_screen.graphAreas)
            return;
        GraphArea *area = _screen.graphAreas[_screen.current];
        if (!area)
            return;
        if (area->paused == paused)
            return;
        area->paused = paused;
        if (!paused)
            graph_internal::invalidateGraphRenderCache(*area);
        area->pauseToggled = true;
        requestRedraw();
    }

    bool GUI::GraphPauseToggled() noexcept
    {
        if (_screen.current >= _screen.capacity || !_screen.graphAreas)
            return false;
        GraphArea *area = _screen.graphAreas[_screen.current];
        if (!area)
            return false;
        const bool v = area->pauseToggled;
        area->pauseToggled = false;
        return v;
    }

    static void backlightPlatformCallback(uint16_t level)
    {
        pipcore::Platform *p = pipcore::GetPlatform();
        if (!p)
            return;
        if (level > 100)
            level = 100;
        p->setBacklightPercent(static_cast<uint8_t>(level));
    }

    GUI::GUI()
    {
        detail::g_activeGui = this;
    }

    GUI::~GUI() noexcept
    {
        pipcore::Platform *plat = pipcore::GetPlatform();

        freeBlurBuffers(plat);
        freeGraphAreas(plat);
        freeLists(plat);
        freeTiles(plat);
        freePopupMenu(plat);
        freeErrors(plat);
        freeScreenState(plat);
#if PIPGUI_SCREENSHOTS
        freeScreenshotGallery(plat);
        freeScreenshotStream(plat);
#endif
        freeAdaptivePreviewBuffer(plat);
        freeRotationBuffer(plat);
        _render.sprite.deleteSprite();
        _flags.spriteEnabled = 0;
        if (detail::g_activeGui == this)
            detail::g_activeGui = nullptr;
    }

    template <typename T>
    static void safeFree(pipcore::Platform *plat, T *&ptr) noexcept
    {
        if (ptr)
        {
            detail::free(plat, ptr);
            ptr = nullptr;
        }
    }

    template <typename T>
    static void safeFreeArray(pipcore::Platform *plat, T *&arr, uint16_t count) noexcept
    {
        if (!arr)
            return;
        for (uint16_t i = 0; i < count; ++i)
            arr[i].~T();
        detail::free(plat, arr);
        arr = nullptr;
    }

    template <typename T>
    static void safeFreeEntryArray(pipcore::Platform *plat, T *&arr,
                                   uint8_t &capacity, uint8_t &count) noexcept
    {
        if (!arr)
            return;
        for (uint8_t i = 0; i < count; ++i)
            arr[i].~T();
        detail::free(plat, arr);
        arr = nullptr;
        capacity = 0;
        count = 0;
    }

    template <typename T>
    struct ObjectGuard
    {
        pipcore::Platform *plat;
        T *&ptr;
        bool released;

        ObjectGuard(pipcore::Platform *p, T *&obj)
            : plat(p), ptr(obj), released(false) {}

        ~ObjectGuard()
        {
            if (!released && ptr)
            {
                ptr->~T();
                detail::free(plat, ptr);
                ptr = nullptr;
            }
        }
    };

    void GUI::freeBlurBuffers(pipcore::Platform *plat) noexcept
    {
        safeFree(plat, _blur.smallIn);
        safeFree(plat, _blur.smallTmp);
        safeFree(plat, _blur.lookup);
        _blur.captureSprite.deleteSprite();
        _blur.workLen = 0;
        _blur.lookupSw = 0;
        _blur.lookupSh = 0;
        _blur.lookupW = 0;
        _blur.lookupH = 0;
        _blur.lookupRadius = 0;
        _blur.lastUseMs = 0;
    }

    void GUI::freeGraphAreas(pipcore::Platform *plat) noexcept
    {
        if (!_screen.graphAreas)
            return;

        for (uint16_t i = 0; i < _screen.capacity; ++i)
        {
            GraphArea *a = _screen.graphAreas[i];
            if (!a)
                continue;

            if (a->samples)
            {
                for (uint16_t line = 0; line < a->lineCount; ++line)
                    detail::free(plat, a->samples[line]);
                detail::free(plat, a->samples);
                a->samples = nullptr;
            }

            safeFree(plat, a->lineColors565);
            safeFree(plat, a->lineValueMins);
            safeFree(plat, a->lineValueMaxs);
            safeFree(plat, a->sampleCounts);
            safeFree(plat, a->sampleHead);

            _screen.graphAreas[i] = nullptr;
            ObjectGuard<GraphArea> guard(plat, a);
        }
    }

    bool GUI::releaseGraphCachesForRecovery(pipcore::Platform *plat) noexcept
    {
        if (!plat || !_screen.graphAreas)
            return false;

        bool released = false;

        for (uint16_t i = 0; i < _screen.capacity; ++i)
        {
            GraphArea *area = _screen.graphAreas[i];
            if (!area)
                continue;

            if (area->innerCache)
            {
                detail::free(plat, area->innerCache);
                area->innerCache = nullptr;
                released = true;
            }
            area->innerCacheW = 0;
            area->innerCacheH = 0;
            area->innerCacheDisabled = false;

            if (area->renderCache)
            {
                detail::free(plat, area->renderCache);
                area->renderCache = nullptr;
                released = true;
            }
            area->renderCacheW = 0;
            area->renderCacheH = 0;
            area->renderCacheValid = false;
            area->renderCacheTileMask = 0;
            area->renderCacheDisabled = false;
        }
        return released;
    }

    void GUI::freeLists(pipcore::Platform *plat) noexcept
    {
        if (!_screen.lists)
            return;

        for (uint16_t i = 0; i < _screen.capacity; ++i)
        {
            ListState *m = _screen.lists[i];
            if (!m)
                continue;

            safeFreeArray(plat, m->items, m->capacity);

            ObjectGuard<ListState> guard(plat, m);
            _screen.lists[i] = nullptr;
        }
    }

    void GUI::freeTiles(pipcore::Platform *plat) noexcept
    {
        if (!_screen.tiles)
            return;

        for (uint16_t i = 0; i < _screen.capacity; ++i)
        {
            TileState *t = _screen.tiles[i];
            if (!t)
                continue;

            safeFreeArray(plat, t->items, t->itemCapacity);
            t->itemCapacity = 0;

            ObjectGuard<TileState> guard(plat, t);
            _screen.tiles[i] = nullptr;
        }
    }

    void GUI::freeScreenState(pipcore::Platform *plat) noexcept
    {
        freeGraphAreas(plat);
        freeLists(plat);
        freeTiles(plat);

        detail::free(plat, _screen.callbacks);
        detail::free(plat, _screen.graphAreas);
        detail::free(plat, _screen.lists);
        detail::free(plat, _screen.tiles);
        detail::free(plat, _screen.navBindings);

        _screen.callbacks = nullptr;
        _screen.graphAreas = nullptr;
        _screen.lists = nullptr;
        _screen.tiles = nullptr;
        _screen.navBindings = nullptr;
        _screen.capacity = 0;
        _screen.current = INVALID_SCREEN_ID;
        _screen.registrySynced = false;
    }

    void GUI::freeErrors(pipcore::Platform *plat) noexcept
    {
        safeFreeEntryArray(plat, _error.entries, _error.capacity, _error.count);
    }

    void GUI::resetDisplayRuntime() noexcept
    {
        freeAdaptivePreviewBuffer(platform());
        freeRotationBuffer(platform());
        _rotationAnim.active = false;
        _rotationAnim.switched = false;
        _disp.display = nullptr;
        _render.physicalWidth = 0;
        _render.physicalHeight = 0;
        _render.screenWidth = 0;
        _render.screenHeight = 0;
        Debug::setCanvasSize(0, 0);
        _render.bgColor = 0;
        _render.bgColor565 = 0;
        _render.sprite.deleteSprite();
        _render.activeSprite = nullptr;
        _flags.spriteEnabled = 0;
        _flags.tiledMode = 0;
        _flags.autoTiledMode = 0;
        _flags.inSpritePass = 0;
        _flags.screenTransition = 0;
        _flags.needRedraw = 0;
        _flags.dirtyRedrawPending = 0;
        _dirty.count = 0;
        _clip = {};
        _nav = {};
        _screen.to = _screen.current;
        _screen.transDir = 0;
        _screen.animStartMs = 0;
        _popup.lastRectValid = false;
        _toast.lastRectValid = false;
        _diag.lastTiledPromoteTryMs = 0;
#if PIPGUI_SCREENSHOTS
        freeScreenshotStream(platform());
#endif
    }

    pipcore::Display &GUI::display()
    {
        if (_disp.display)
            return *_disp.display;

        reportPlatformErrorOnce("display");
        static NullDisplay nullDisplay;
        return nullDisplay;
    }

    ConfigDisplayFluent GUI::configDisplay()
    {
        return ConfigDisplayFluent(this);
    }

    void GUI::configDisplay(const pipcore::DisplayConfig &cfg)
    {
        pipcore::Platform *plat = pipcore::GetPlatform();
        if (!plat)
            return;

        pipcore::DisplayConfig normalized = cfg;

        if (!plat->configDisplay(normalized))
        {
            return;
        }

        _disp.cfg = normalized;
        _disp.cfgConfigured = true;
        resetDisplayRuntime();
        clearReportedPlatformError();
    }

    void GUI::configureBacklight(uint8_t pin, uint8_t channel, uint32_t freqHz, uint8_t resolutionBits)
    {
        pipcore::Platform *plat = pipcore::GetPlatform();
        if (!plat)
            return;
        plat->configureBacklightPin(pin, channel, freqHz, resolutionBits);
        setBacklightHandler(backlightPlatformCallback);
    }

    void ConfigDisplayFluent::apply()
    {
        if (_consumed || !_touched)
            return;
        _consumed = true;
        if (!_gui)
            return;
        detail::GuiAccess::configDisplay(*_gui, _cfg);
    }

    void ConfigureBacklightFluent::apply()
    {
        if (_consumed || !_touched)
            return;
        _consumed = true;
        if (!_gui)
            return;
        detail::GuiAccess::configureBacklight(*_gui, _pin, _channel, _freqHz, _resolutionBits);
    }

    void SetClipFluent::apply()
    {
        if (_consumed || !_touched)
            return;
        _consumed = true;
        if (!_gui)
            return;
        detail::GuiAccess::setClip(*_gui, _x, _y, _w, _h);
    }

    void ShowLogoFluent::apply()
    {
        if (_consumed || !_touched)
            return;
        _consumed = true;
        if (!_gui)
            return;
        detail::GuiAccess::startLogo(*_gui, _title, _subtitle, _anim);
    }

    void BindNavFluent::apply()
    {
        if (_consumed)
            return;
        _consumed = true;
        if (!_gui)
            return;
        if (!_clear && !_handler)
            return;
        const uint8_t screenId = _gui->currentScreen();
        if (screenId == INVALID_SCREEN_ID)
            return;
        _gui->bindNavHandler(screenId, _clear ? nullptr : _handler, _clear ? nullptr : _userData);
    }

    void UseListNavFluent::apply()
    {
        if (_consumed)
            return;
        _consumed = true;
        if (!_gui)
            return;
        const uint8_t screenId = _gui->currentScreen();
        if (screenId == INVALID_SCREEN_ID)
            return;
        _gui->bindListNav(screenId);
    }

    void UseTileNavFluent::apply()
    {
        if (_consumed)
            return;
        _consumed = true;
        if (!_gui)
            return;
        const uint8_t screenId = _gui->currentScreen();
        if (screenId == INVALID_SCREEN_ID)
            return;
        _gui->bindTileNav(screenId);
    }

    void PopupMenuInputFluent::apply()
    {
        if (_consumed)
            return;
        _consumed = true;
        if (!_gui)
            return;
        _gui->_manualInputMask |= GUI::ManualInput_Popup;
        GUI::InputState input;
        input.nextDown = _nextDown;
        input.prevDown = _prevDown;
        input.selectDown = _selectDown;
        input.hasSelect = _hasSelect;
        detail::GuiAccess::handlePopupMenuInput(*_gui, input);
    }

    void ConfigStatusBarFluent::apply()
    {
        if (!beginCommit())
            return;
        detail::GuiAccess::configStatusBar(*_gui, _height, _pos, _style);
    }

    void SetStatusBarTextFluent::apply()
    {
        if (!beginCommit())
            return;
        detail::GuiAccess::setStatusBarText(*_gui, _left, _center, _right);
    }

    void SetStatusBarIconFluent::apply()
    {
        if (!beginCommit() || !_hasSide || !_hasIcon)
            return;
        detail::GuiAccess::setStatusBarIcon(*_gui, _side, _iconId, detail::optionalColor32(_color565), _sizePx);
    }

    void GUI::begin(uint8_t rotation, bool forceTiles)
    {
        pipcore::Platform *plat = pipcore::GetPlatform();
        if (!plat)
            return;

        constexpr uint16_t bgColor = 0x0000;

        resetDisplayRuntime();

#if PIPGUI_STATUS_BAR
        _flags.statusBarDebugMetrics = false;
#endif

#if PIPGUI_DEBUG_DIRTY_RECTS
        Debug::setDirtyRectEnabled(true);
#endif
#if PIPGUI_DEBUG_OVERDRAW
        Debug::setOverdrawEnabled(true);
#endif
#if PIPGUI_DEBUG_LAYOUT_BOUNDS
        Debug::setLayoutBoundsEnabled(true);
#endif
#if PIPGUI_DEBUG_METRICS
        _flags.statusBarDebugMetrics = true;
        Debug::init();
        _status.dirtyMask = detail::StatusBarDirtyAll;
#elif PIPGUI_DEBUG_DIRTY_RECTS || PIPGUI_DEBUG_OVERDRAW || PIPGUI_DEBUG_LAYOUT_BOUNDS
        Debug::init();
#endif

        if (_disp.cfgConfigured)
        {
            if (!plat->configDisplay(_disp.cfg))
            {
                return;
            }
            clearReportedPlatformError();
        }

        if (!plat->beginDisplay(rotation))
        {
            return;
        }
        clearReportedPlatformError();
        _disp.physicalRotation = rotation & 3U;
        _disp.rotation = _disp.physicalRotation;

        _disp.display = plat->display();
        if (!_disp.display)
        {
            return;
        }

        _render.physicalWidth = _disp.display->width();
        _render.physicalHeight = _disp.display->height();
        _render.screenWidth = _render.physicalWidth;
        _render.screenHeight = _render.physicalHeight;
        Debug::setCanvasSize((int16_t)_render.screenWidth, (int16_t)_render.screenHeight);
        _render.bgColor = bgColor;
        _render.bgColor565 = bgColor;
        _render.originX = 0;
        _render.originY = 0;

        _render.sprite.setPlatform(plat);
        _blur.captureSprite.setPlatform(plat);

        _render.sprite.deleteSprite();
        _flags.tiledMode = 0;

        const int16_t sw = (int16_t)_render.screenWidth;
        const int16_t sh = (int16_t)_render.screenHeight;
        const auto tryCreateMainCanvas = [&](bool tiled, bool autoTiled) noexcept -> bool
        {
            if (!tiled)
            {
                releaseScreenshotGalleryCache(plat);
                freeScreenshotStream(plat);
                freeBlurBuffers(plat);
                freeRotationLineBuffer(plat);
                (void)releaseGraphCachesForRecovery(plat);
            }

            int16_t targetH = tiled ? ((sh > 1) ? (int16_t)((sh + 1) / 2) : sh) : sh;
            _render.sprite.deleteSprite();
            bool ok = _render.sprite.createSprite(sw, targetH);

            if (!ok && tiled)
            {
                targetH = (sh > 1) ? (int16_t)((sh + 3) / 4) : sh;
                ok = _render.sprite.createSprite(sw, targetH);
            }

            if (ok)
            {
                _flags.tiledMode = tiled ? 1U : 0U;
                _flags.autoTiledMode = (tiled && autoTiled) ? 1U : 0U;
                return true;
            }

            const size_t bytes = static_cast<size_t>(sw) * static_cast<size_t>(targetH) * sizeof(uint16_t);
            if (plat && bytes > 0 &&
                detail::recoverFromAllocFailure(plat, bytes, pipcore::AllocCaps::Default))
            {
                targetH = tiled ? ((sh > 1) ? (int16_t)((sh + 1) / 2) : sh) : sh;
                ok = _render.sprite.createSprite(sw, targetH);
                if (!ok && tiled)
                {
                    targetH = (sh > 1) ? (int16_t)((sh + 3) / 4) : sh;
                    ok = _render.sprite.createSprite(sw, targetH);
                }
                if (ok)
                {
                    _flags.tiledMode = tiled ? 1U : 0U;
                    _flags.autoTiledMode = (tiled && autoTiled) ? 1U : 0U;
                    return true;
                }
            }
            return false;
        };

        bool ok = false;
        if (!forceTiles)
            ok = tryCreateMainCanvas(false, false);

        if (!ok)
            ok = tryCreateMainCanvas(true, !forceTiles);

        _flags.spriteEnabled = ok ? 1U : 0U;
        _render.activeSprite = _flags.spriteEnabled ? &_render.sprite : nullptr;

        if (_flags.spriteEnabled)
        {
            if (!_flags.tiledMode)
            {
                const bool prevRender = _flags.inSpritePass;
                _flags.inSpritePass = 1;
                clear(bgColor);
                _flags.inSpritePass = prevRender;
                invalidateRect(0, 0, (int16_t)_render.screenWidth, (int16_t)_render.screenHeight);
                flushDirty();
            }
            else if (_disp.display)
            {
                _disp.display->fillScreen565(bgColor);
            }
        }

        syncRegisteredScreens();

        _disp.brightnessMax = plat->loadMaxBrightnessPercent();
        _disp.brightness = _disp.brightnessMax;

        initFonts();
    }

    void GUI::setAdaptivePreview(uint16_t minWidth, uint16_t minHeight, uint32_t cycleMs)
    {
        _adaptivePreview.enabled = true;
        _adaptivePreview.minWidth = minWidth;
        _adaptivePreview.minHeight = minHeight;
        _adaptivePreview.cycleMs = cycleMs ? cycleMs : 3600;
        _adaptivePreview.startMs = 0;
        requestRedraw();
    }

    void GUI::clearAdaptivePreview() noexcept
    {
        _adaptivePreview.enabled = false;
        _adaptivePreview.startMs = 0;
        _adaptivePreview.lastPresentedW = 0;
        _adaptivePreview.lastPresentedH = 0;
        _adaptivePreview.lastOutputW = 0;
        _adaptivePreview.lastOutputH = 0;
        if (_render.physicalWidth > 0 && _render.physicalHeight > 0)
        {
            _render.screenWidth = _render.physicalWidth;
            _render.screenHeight = _render.physicalHeight;
            Debug::setCanvasSize((int16_t)_render.screenWidth, (int16_t)_render.screenHeight);
        }
        _dirty.count = 0;
        _flags.dirtyRedrawPending = 0;
        _flags.needRedraw = 1;
    }

    bool GUI::adaptivePreviewActive() const noexcept
    {
        return _adaptivePreview.enabled &&
               _render.physicalWidth > 0 &&
               _render.physicalHeight > 0;
    }

    bool GUI::logicalRotationActive() const noexcept
    {
        return ((_disp.rotation - _disp.physicalRotation) & 3U) != 0U;
    }

    uint8_t GUI::logicalRotationDelta() const noexcept
    {
        return static_cast<uint8_t>((_disp.rotation - _disp.physicalRotation) & 3U);
    }

    float GUI::presentationAngleRad(uint8_t rotation) const noexcept
    {
        const uint8_t delta = (rotation - _disp.physicalRotation) & 3U;
        float angleDeg = 0.0f;
        if (delta == 1U)
            angleDeg = 90.0f;
        else if (delta == 2U)
            angleDeg = 180.0f;
        else if (delta == 3U)
            angleDeg = -90.0f;
        return angleDeg * (3.1415926535f / 180.0f);
    }

    bool GUI::presentOrthogonalRotatedSprite(const uint16_t *src, int16_t srcStride, int16_t srcW, int16_t srcH,
                                             uint8_t rotationDelta, const char *stage)
    {
        return presentOrthogonalRotatedSpriteRegion(src,
                                                    srcStride,
                                                    srcW,
                                                    srcH,
                                                    rotationDelta,
                                                    0,
                                                    0,
                                                    srcW,
                                                    srcH,
                                                    stage);
    }

    bool GUI::presentOrthogonalRotatedSpriteRegion(const uint16_t *src, int16_t srcStride, int16_t srcW, int16_t srcH,
                                                   uint8_t rotationDelta,
                                                   int16_t srcX, int16_t srcY, int16_t rectW, int16_t rectH,
                                                   const char *stage)
    {
        if (!_disp.display || !src || srcStride <= 0 || srcW <= 0 || srcH <= 0)
            return false;

        const uint16_t physW = _disp.display->width();
        const uint16_t physH = _disp.display->height();
        if (physW == 0 || physH == 0)
            return false;

        if (srcX < 0)
        {
            rectW += srcX;
            srcX = 0;
        }
        if (srcY < 0)
        {
            rectH += srcY;
            srcY = 0;
        }
        if (srcX + rectW > srcW)
            rectW = static_cast<int16_t>(srcW - srcX);
        if (srcY + rectH > srcH)
            rectH = static_cast<int16_t>(srcH - srcY);
        if (rectW <= 0 || rectH <= 0)
            return false;

        if (rotationDelta == 0)
        {
            _disp.display->writeRect565(srcX,
                                        srcY,
                                        rectW,
                                        rectH,
                                        src + static_cast<size_t>(srcY) * static_cast<size_t>(srcStride) + srcX,
                                        srcStride);
            reportPlatformErrorOnce(stage);
            pipcore::Platform *plat = pipcore::GetPlatform();
            return !plat || plat->lastError() == pipcore::PlatformError::None;
        }

        const uint8_t delta = rotationDelta & 3U;
        int16_t dstX = 0;
        int16_t dstY = 0;
        int16_t writeW = rectW;
        int16_t writeH = rectH;
        switch (delta)
        {
        case 1:
            dstX = static_cast<int16_t>(srcH - srcY - rectH);
            dstY = srcX;
            writeW = rectH;
            writeH = rectW;
            break;
        case 2:
            dstX = static_cast<int16_t>(srcW - srcX - rectW);
            dstY = static_cast<int16_t>(srcH - srcY - rectH);
            break;
        case 3:
            dstX = srcY;
            dstY = static_cast<int16_t>(srcW - srcX - rectW);
            writeW = rectH;
            writeH = rectW;
            break;
        default:
            return false;
        }

        if (dstX < 0 || dstY < 0)
            return false;
        if (dstX + writeW > static_cast<int16_t>(physW))
            writeW = static_cast<int16_t>(static_cast<int16_t>(physW) - dstX);
        if (dstY + writeH > static_cast<int16_t>(physH))
            writeH = static_cast<int16_t>(static_cast<int16_t>(physH) - dstY);
        if (writeW <= 0 || writeH <= 0)
            return false;

        constexpr int16_t kChunkRows = 8;
        const uint16_t lineNeed = static_cast<uint16_t>(static_cast<uint16_t>(writeW) * static_cast<uint16_t>(kChunkRows));
        if (_rotationAnim.lineBufCap < lineNeed)
        {
            pipcore::Platform *plat = platform();
            uint16_t *newBuf = plat ? static_cast<uint16_t *>(plat->alloc(static_cast<size_t>(lineNeed) * sizeof(uint16_t), pipcore::AllocCaps::PreferInternal)) : nullptr;
            if (!newBuf)
            {
                if (plat)
                    (void)detail::recoverFromAllocFailure(plat, static_cast<size_t>(lineNeed) * sizeof(uint16_t), pipcore::AllocCaps::PreferInternal);
                newBuf = plat ? static_cast<uint16_t *>(plat->alloc(static_cast<size_t>(lineNeed) * sizeof(uint16_t), pipcore::AllocCaps::Default)) : nullptr;
            }
            if (!newBuf && plat && detail::recoverFromAllocFailure(plat, static_cast<size_t>(lineNeed) * sizeof(uint16_t), pipcore::AllocCaps::Default))
                newBuf = static_cast<uint16_t *>(plat->alloc(static_cast<size_t>(lineNeed) * sizeof(uint16_t), pipcore::AllocCaps::Default));
            if (newBuf)
            {
                freeRotationLineBuffer(plat);
                _rotationAnim.lineBuf = newBuf;
                _rotationAnim.lineBufCap = lineNeed;
            }
        }

        if (!_rotationAnim.lineBuf || _rotationAnim.lineBufCap < lineNeed)
            return false;

        for (int16_t y0 = 0; y0 < writeH; y0 = static_cast<int16_t>(y0 + kChunkRows))
        {
            const int16_t chunkH = static_cast<int16_t>(std::min<int32_t>(kChunkRows, writeH - y0));
            for (int16_t row = 0; row < chunkH; ++row)
            {
                const int16_t localY = static_cast<int16_t>(y0 + row);
                uint16_t *dst = _rotationAnim.lineBuf + static_cast<size_t>(row) * writeW;
                switch (delta)
                {
                case 1:
                {
                    const int16_t sampleX = static_cast<int16_t>(srcX + localY);
                    for (int16_t x = 0; x < writeW; ++x)
                    {
                        const int16_t sampleY = static_cast<int16_t>(srcY + rectH - 1 - x);
                        dst[x] = src[static_cast<size_t>(sampleY) * static_cast<size_t>(srcStride) + sampleX];
                    }
                    break;
                }
                case 2:
                {
                    const int16_t sampleY = static_cast<int16_t>(srcY + rectH - 1 - localY);
                    const uint16_t *srcRow = src + static_cast<size_t>(sampleY) * static_cast<size_t>(srcStride);
                    for (int16_t x = 0; x < writeW; ++x)
                        dst[x] = srcRow[srcX + rectW - 1 - x];
                    break;
                }
                case 3:
                {
                    const int16_t sampleX = static_cast<int16_t>(srcX + rectW - 1 - localY);
                    for (int16_t x = 0; x < writeW; ++x)
                    {
                        const int16_t sampleY = static_cast<int16_t>(srcY + x);
                        dst[x] = src[static_cast<size_t>(sampleY) * static_cast<size_t>(srcStride) + sampleX];
                    }
                    break;
                }
                }
            }
            _disp.display->writeRect565(dstX,
                                        static_cast<int16_t>(dstY + y0),
                                        writeW,
                                        chunkH,
                                        _rotationAnim.lineBuf,
                                        writeW);
        }

        reportPlatformErrorOnce(stage);
        pipcore::Platform *plat = pipcore::GetPlatform();
        return !plat || plat->lastError() == pipcore::PlatformError::None;
    }

    bool GUI::recoverFromAllocationFailure(size_t, pipcore::AllocCaps) noexcept
    {
        pipcore::Platform *plat = platform();
        if (!plat)
            return false;

        bool reclaimed = false;

        const bool hadGalleryEntries = (_shots.entries != nullptr);
#if (PIPGUI_SCREENSHOT_MODE == 2)
        const bool hadGalleryRowBuf = (_shots.rowBuf != nullptr);
#else
        const bool hadGalleryRowBuf = false;
#endif
        if (hadGalleryEntries || hadGalleryRowBuf)
        {
            freeScreenshotGallery(plat);
            reclaimed = true;
        }

        const bool hadShotStream = (_shotStream.buffer != nullptr) || _shotStream.active;
        if (hadShotStream)
        {
            freeScreenshotStream(plat);
            reclaimed = true;
        }

        const bool hadBlur = (_blur.smallIn != nullptr) || (_blur.smallTmp != nullptr) ||
                             (_blur.lookup != nullptr) || (_blur.captureSprite.getBuffer() != nullptr);
        if (hadBlur)
        {
            freeBlurBuffers(plat);
            reclaimed = true;
        }

        const bool hadAdaptive = (_adaptivePreview.lineBuf != nullptr);
        if (hadAdaptive)
        {
            freeAdaptivePreviewBuffer(plat);
            reclaimed = true;
        }

        const bool rotationActiveAtEntry = _rotationAnim.active;
        const bool hadRotation = (_rotationAnim.lineBuf != nullptr) || _rotationAnim.active;
        if (hadRotation)
        {
            if (rotationActiveAtEntry)
            {
                if (!_rotationAnim.streamingFrame)
                    freeRotationLineBuffer(plat);
            }
            else
            {
                freeRotationBuffer(plat);
            }
            reclaimed = true;
        }

        reclaimed = releaseGraphCachesForRecovery(plat) || reclaimed;

        if (_rotationAnim.active && !rotationActiveAtEntry)
        {
            _disp.rotation = _rotationAnim.switched ? _rotationAnim.to : _rotationAnim.from;
            _rotationAnim.active = false;
            _rotationAnim.switched = false;
        }

        if (_flags.screenTransition)
        {
            if (_screen.to < _screen.capacity)
                _screen.current = _screen.to;
            _flags.screenTransition = 0;
            resetNavDispatch();
        }

        if (rotationActiveAtEntry ||
            _flags.tiledMode || !_disp.display || !_flags.spriteEnabled || _render.screenWidth == 0 || _render.screenHeight == 0)
            return reclaimed;

        const uint8_t delta = logicalRotationDelta();
        const uint16_t canvasW = (_render.physicalWidth > 0 && _render.physicalHeight > 0)
                                     ? ((delta & 1U) ? _render.physicalHeight : _render.physicalWidth)
                                     : _render.screenWidth;
        const uint16_t canvasH = (_render.physicalWidth > 0 && _render.physicalHeight > 0)
                                     ? ((delta & 1U) ? _render.physicalWidth : _render.physicalHeight)
                                     : _render.screenHeight;
        const int16_t sw = static_cast<int16_t>(canvasW);
        const int16_t sh = static_cast<int16_t>(canvasH);

        int16_t tileH = (sh > 1) ? static_cast<int16_t>((sh + 1) / 2) : sh;
        _render.sprite.deleteSprite();
        bool ok = _render.sprite.createSprite(sw, tileH);

        if (!ok)
        {
            tileH = (sh > 1) ? static_cast<int16_t>((sh + 3) / 4) : sh;
            ok = _render.sprite.createSprite(sw, tileH);
        }

        _flags.spriteEnabled = ok ? 1U : 0U;
        _flags.tiledMode = ok ? 1U : 0U;
        _flags.autoTiledMode = ok ? 1U : 0U;
        _render.activeSprite = ok ? &_render.sprite : nullptr;
        _clip = {};
        _dirty.count = 0;
        _flags.dirtyRedrawPending = 0;
        _flags.needRedraw = ok ? 1U : 0U;
        Debug::clearRects();
        return reclaimed || ok;
    }

    bool GUI::tryPromoteAutoTiledCanvas(uint32_t now) noexcept
    {
        if (!_flags.tiledMode || !_flags.autoTiledMode || !_disp.display || !_flags.spriteEnabled)
            return false;
        if (_flags.bootActive || _flags.errorActive || _flags.notifActive ||
            _flags.popupActive || _flags.toastActive || _flags.screenTransition || _rotationAnim.active)
            return false;
        if (_diag.lastTiledPromoteTryMs != 0 && (now - _diag.lastTiledPromoteTryMs) < 1200U)
            return false;
        _diag.lastTiledPromoteTryMs = now;

        pipcore::Platform *plat = platform();
        if (!plat || _render.screenWidth == 0 || _render.screenHeight == 0)
            return false;

        releaseScreenshotGalleryCache(plat);
        freeScreenshotStream(plat);
        freeBlurBuffers(plat);
        freeRotationLineBuffer(plat);
        (void)releaseGraphCachesForRecovery(plat);

        const uint8_t delta = logicalRotationDelta();
        const uint16_t fullW = (delta & 1U) ? _render.physicalHeight : _render.physicalWidth;
        const uint16_t fullH = (delta & 1U) ? _render.physicalWidth : _render.physicalHeight;
        const int16_t fullWi = static_cast<int16_t>(fullW);
        const int16_t fullHi = static_cast<int16_t>(fullH);

        const int16_t prevTileH = _render.sprite.height();
        const int16_t tileW = static_cast<int16_t>(fullW);

        _render.sprite.deleteSprite();
        if (_render.sprite.createSprite(fullWi, fullHi))
        {
            _flags.tiledMode = 0;
            _flags.autoTiledMode = 0;
            _flags.spriteEnabled = 1;
            _render.activeSprite = &_render.sprite;
            _clip = {};
            _dirty.count = 0;
            _flags.dirtyRedrawPending = 0;
            _flags.needRedraw = 1;
            Debug::clearRects();
            return true;
        }

        const bool restored = _render.sprite.createSprite(tileW, prevTileH);
        _flags.spriteEnabled = restored ? 1U : 0U;
        _flags.tiledMode = restored ? 1U : 0U;
        _flags.autoTiledMode = restored ? 1U : 0U;
        _render.activeSprite = restored ? &_render.sprite : nullptr;
        _clip = {};
        _dirty.count = 0;
        _flags.dirtyRedrawPending = 0;
        _flags.needRedraw = restored ? 1U : 0U;
        Debug::clearRects();
        return false;
    }

    void GUI::freeAdaptivePreviewBuffer(pipcore::Platform *plat) noexcept
    {
        if (_adaptivePreview.lineBuf && plat)
            plat->free(_adaptivePreview.lineBuf);
        _adaptivePreview.lineBuf = nullptr;
        _adaptivePreview.lineBufCap = 0;
        _adaptivePreview.lastPresentedW = 0;
        _adaptivePreview.lastPresentedH = 0;
        _adaptivePreview.lastOutputW = 0;
        _adaptivePreview.lastOutputH = 0;
    }

    void GUI::freeRotationBuffer(pipcore::Platform *plat) noexcept
    {
        _rotationAnim.startedTiled = false;
        _rotationAnim.startedAutoTiled = false;
        _rotationAnim.streamingFrame = false;
        freeRotationLineBuffer(plat);
    }

    void GUI::freeRotationLineBuffer(pipcore::Platform *plat) noexcept
    {
        if (_rotationAnim.lineBuf && plat)
            plat->free(_rotationAnim.lineBuf);
        _rotationAnim.lineBuf = nullptr;
        _rotationAnim.lineBufCap = 0;
    }

    bool GUI::ensureRotationLineBuffer(uint32_t pixels) noexcept
    {
        if (pixels == 0 || pixels > (UINT32_MAX / sizeof(uint16_t)))
            return false;
        if (_rotationAnim.lineBuf && _rotationAnim.lineBufCap >= pixels)
            return true;

        pipcore::Platform *plat = platform();
        if (!plat)
            return false;

        const size_t bytes = static_cast<size_t>(pixels) * sizeof(uint16_t);
        uint16_t *newBuf = static_cast<uint16_t *>(plat->alloc(bytes, pipcore::AllocCaps::PreferInternal));
        if (!newBuf)
        {
            (void)detail::recoverFromAllocFailure(plat, bytes, pipcore::AllocCaps::PreferInternal);
            newBuf = static_cast<uint16_t *>(plat->alloc(bytes, pipcore::AllocCaps::Default));
        }
        if (!newBuf && detail::recoverFromAllocFailure(plat, bytes, pipcore::AllocCaps::Default))
            newBuf = static_cast<uint16_t *>(plat->alloc(bytes, pipcore::AllocCaps::Default));
        if (!newBuf)
            return false;

        freeRotationLineBuffer(plat);
        _rotationAnim.lineBuf = newBuf;
        _rotationAnim.lineBufCap = pixels;
        return true;
    }

    void GUI::renderCurrentFrameToTileBand(int16_t tileY, int16_t, uint32_t now)
    {
        (void)now;
        const int16_t stride = _render.sprite.width();
        const int16_t tileH = _render.sprite.height();
        if (stride <= 0 || tileH <= 0)
            return;

        _render.originX = 0;
        _render.originY = tileY;
        _render.sprite.setClipRect(0, 0, stride, tileH);

        const ScreenCallback currentCb = (_screen.current < _screen.capacity && _screen.callbacks)
                                             ? _screen.callbacks[_screen.current]
                                             : nullptr;

        if (_screen.current < _screen.capacity)
        {
            if (currentCb)
                renderScreenToMainSprite(currentCb, _screen.current);
            else
                renderScreenToMainSprite(nullptr, _screen.current);
            renderStatusBar();
        }
        else
        {
            const bool prevRender = _flags.inSpritePass;
            pipcore::Sprite *prevActive = _render.activeSprite;
            _flags.inSpritePass = 1;
            _render.activeSprite = &_render.sprite;
            clear(_render.bgColor565 ? _render.bgColor565 : static_cast<uint16_t>(_render.bgColor));
            _render.activeSprite = prevActive;
            _flags.inSpritePass = prevRender;
        }
    }

    bool GUI::presentTransformedTiledFrame(float angleRad, float scale, uint32_t now, const char *stage)
    {
        if (!_disp.display || !_flags.spriteEnabled || !_flags.tiledMode)
            return false;

        const int16_t srcW = static_cast<int16_t>(_render.screenWidth);
        const int16_t srcH = static_cast<int16_t>(_render.screenHeight);
        const int16_t tileH = _render.sprite.height();
        const int16_t stride = _render.sprite.width();
        auto *tile = static_cast<uint16_t *>(_render.sprite.getBuffer());
        if (!tile || srcW <= 0 || srcH <= 0 || tileH <= 0 || stride < srcW)
            return false;

        const uint16_t physW = _disp.display->width();
        const uint16_t physH = _disp.display->height();
        if (physW == 0 || physH == 0)
            return false;

        constexpr uint16_t rowCandidates[] = {96, 80, 64, 48, 32, 16, 8, 4, 1};
        uint16_t rows = 0;
        uint16_t lastTried = 0;
        for (uint16_t candidate : rowCandidates)
        {
            const uint16_t tryRows = static_cast<uint16_t>((physH < candidate) ? physH : candidate);
            if (tryRows == 0 || tryRows == lastTried)
                continue;
            lastTried = tryRows;
            if (ensureRotationLineBuffer(static_cast<uint32_t>(physW) * tryRows))
            {
                rows = tryRows;
                break;
            }
        }
        if (rows == 0)
            return false;

        struct StreamingGuard
        {
            bool *flag;
            ~StreamingGuard() noexcept
            {
                if (flag)
                    *flag = false;
            }
        };
        _rotationAnim.streamingFrame = true;
        StreamingGuard streamingGuard{&_rotationAnim.streamingFrame};

        const ClipState savedClip = _clip;
        int32_t savedClipX = 0;
        int32_t savedClipY = 0;
        int32_t savedClipW = 0;
        int32_t savedClipH = 0;
        _render.sprite.getClipRect(&savedClipX, &savedClipY, &savedClipW, &savedClipH);
        const int16_t savedOriginX = _render.originX;
        const int16_t savedOriginY = _render.originY;
        _clip.enabled = false;

        const float safeScale = (scale < 0.08f) ? 0.08f : ((scale > 1.15f) ? 1.15f : scale);
        const float invScale = 1.0f / safeScale;
        const float cosA = cosf(angleRad);
        const float sinA = sinf(angleRad);
        const float srcCx = (static_cast<float>(srcW) - 1.0f) * 0.5f;
        const float srcCy = (static_cast<float>(srcH) - 1.0f) * 0.5f;
        const float dstCx = (static_cast<float>(physW) - 1.0f) * 0.5f;
        const float dstCy = (static_cast<float>(physH) - 1.0f) * 0.5f;
        const uint16_t bg = __builtin_bswap16(_render.bgColor565);

        for (uint16_t outY = 0; outY < physH; outY = static_cast<uint16_t>(outY + rows))
        {
            const uint16_t chunkH = static_cast<uint16_t>(((physH - outY) < rows) ? (physH - outY) : rows);
            uint16_t *out = _rotationAnim.lineBuf;
            const uint32_t outPixels = static_cast<uint32_t>(physW) * chunkH;
            for (uint32_t i = 0; i < outPixels; ++i)
                out[i] = bg;

            for (int16_t bandY = 0; bandY < srcH; bandY = static_cast<int16_t>(bandY + tileH))
            {
                const int16_t bandH = static_cast<int16_t>(std::min<int32_t>(tileH, static_cast<int32_t>(srcH) - bandY));
                renderCurrentFrameToTileBand(bandY, bandH, now);

                for (uint16_t row = 0; row < chunkH; ++row)
                {
                    const uint16_t y = static_cast<uint16_t>(outY + row);
                    const float dy = (static_cast<float>(y) - dstCy) * invScale;
                    float srcXf = (((0.0f - dstCx) * invScale) * cosA + dy * sinA) + srcCx;
                    float srcYf = (-((0.0f - dstCx) * invScale) * sinA + dy * cosA) + srcCy;
                    const float stepX = invScale * cosA;
                    const float stepY = -invScale * sinA;
                    uint16_t *dst = out + static_cast<uint32_t>(row) * physW;

                    for (uint16_t x = 0; x < physW; ++x)
                    {
                        const int16_t sx = static_cast<int16_t>(lroundf(srcXf));
                        const int16_t sy = static_cast<int16_t>(lroundf(srcYf));
                        if (sx >= 0 && sx < srcW && sy >= bandY && sy < static_cast<int16_t>(bandY + bandH))
                            dst[x] = tile[static_cast<size_t>(sy - bandY) * static_cast<size_t>(stride) + static_cast<size_t>(sx)];
                        srcXf += stepX;
                        srcYf += stepY;
                    }
                }
            }

            _disp.display->writeRect565(0,
                                        static_cast<int16_t>(outY),
                                        static_cast<int16_t>(physW),
                                        static_cast<int16_t>(chunkH),
                                        out,
                                        physW);
        }

        _clip = savedClip;
        _render.sprite.setClipRect(static_cast<int16_t>(savedClipX),
                                   static_cast<int16_t>(savedClipY),
                                   static_cast<int16_t>(savedClipW),
                                   static_cast<int16_t>(savedClipH));
        _render.originX = savedOriginX;
        _render.originY = savedOriginY;
        _dirty.count = 0;
        _flags.dirtyRedrawPending = 0;

        reportPlatformErrorOnce(stage);
        pipcore::Platform *plat = pipcore::GetPlatform();
        return !plat || plat->lastError() == pipcore::PlatformError::None;
    }

    void GUI::serviceAdaptivePreview(uint32_t now) noexcept
    {
        if (!adaptivePreviewActive())
            return;

        if (_adaptivePreview.startMs == 0)
            _adaptivePreview.startMs = now;

        const uint16_t maxW = _render.physicalWidth;
        const uint16_t maxH = _render.physicalHeight;
        uint16_t minW = _adaptivePreview.minWidth;
        uint16_t minH = _adaptivePreview.minHeight;

        if (minW == 0 || minW > maxW)
            minW = maxW;
        if (minH == 0 || minH > maxH)
            minH = maxH;

        const uint32_t cycleMs = _adaptivePreview.cycleMs ? _adaptivePreview.cycleMs : 3600;
        const uint32_t elapsedMs = now - _adaptivePreview.startMs;
        const float phase = static_cast<float>(elapsedMs % cycleMs) / static_cast<float>(cycleMs);
        const float pingPong = 0.5f + 0.5f * cosf(phase * 6.28318530718f);

        const uint16_t targetW = static_cast<uint16_t>(lroundf(static_cast<float>(minW) + (static_cast<float>(maxW - minW) * pingPong)));
        const uint16_t targetH = static_cast<uint16_t>(lroundf(static_cast<float>(minH) + (static_cast<float>(maxH - minH) * pingPong)));

        const uint16_t prevW = _render.screenWidth;
        const uint16_t prevH = _render.screenHeight;
        if (prevW == targetW && prevH == targetH)
            return;

        if ((_adaptivePreview.lastPresentedW == 0 || _adaptivePreview.lastPresentedH == 0) &&
            prevW > 0 && prevH > 0)
        {
            _adaptivePreview.lastPresentedW = prevW;
            _adaptivePreview.lastPresentedH = prevH;
            const uint8_t delta = logicalRotationActive() ? logicalRotationDelta() : 0;
            _adaptivePreview.lastOutputW = (delta & 1U) ? prevH : prevW;
            _adaptivePreview.lastOutputH = (delta & 1U) ? prevW : prevH;
        }

        _render.screenWidth = targetW;
        _render.screenHeight = targetH;
        Debug::setCanvasSize((int16_t)_render.screenWidth, (int16_t)_render.screenHeight);

        if (_flags.tiledMode)
        {
            _dirty.count = 0;
            if (targetW > 0 && targetH > 0)
            {
                const int16_t dirtyW = static_cast<int16_t>(targetW);
                const int16_t dirtyH = static_cast<int16_t>(targetH);
                if (_dirty.count < DIRTY_RECT_MAX)
                    _dirty.rects[_dirty.count++] = {0, 0, dirtyW, dirtyH};
            }
            _flags.dirtyRedrawPending = (_dirty.count > 0) ? 1 : 0;
            _flags.needRedraw = 1;
        }
        else
        {
            _dirty.count = 0;
            _flags.dirtyRedrawPending = 0;
            _flags.needRedraw = 1;
        }
        Debug::clearRects();
    }

    bool GUI::presentAdaptivePreview(const char *stage)
    {
        if (!_disp.display || !_flags.spriteEnabled)
            return false;

        const auto *src = static_cast<const uint16_t *>(_render.sprite.getBuffer());
        const uint16_t virtW = _render.screenWidth;
        const uint16_t virtH = _render.screenHeight;
        const uint16_t physW = _render.physicalWidth;
        const uint16_t physH = _render.physicalHeight;
        const int16_t stride = _render.sprite.width();
        if (!src || virtW == 0 || virtH == 0 || physW == 0 || physH == 0 || stride <= 0)
            return false;

        if (logicalRotationActive())
        {
            const uint8_t delta = logicalRotationDelta() & 3U;
            const uint16_t outW = (delta & 1U) ? virtH : virtW;
            const uint16_t outH = (delta & 1U) ? virtW : virtH;
            const uint16_t needLen = (physW > physH) ? physW : physH;

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

            if (!_adaptivePreview.lineBuf || _adaptivePreview.lineBufCap < needLen)
                return false;

            if (_adaptivePreview.lineBuf && _adaptivePreview.lineBufCap >= needLen)
            {
                const uint16_t bg = __builtin_bswap16(_render.bgColor565);
                for (uint16_t x = 0; x < needLen; ++x)
                    _adaptivePreview.lineBuf[x] = bg;

                const uint16_t prevOutW = _adaptivePreview.lastOutputW;
                const uint16_t prevOutH = _adaptivePreview.lastOutputH;
                if (prevOutW > outW)
                {
                    const int16_t clearW = static_cast<int16_t>(prevOutW - outW);
                    const int16_t clearH = static_cast<int16_t>((prevOutH > outH) ? prevOutH : outH);
                    for (int16_t y = 0; y < clearH; ++y)
                        _disp.display->writeRect565((int16_t)outW, y, clearW, 1, _adaptivePreview.lineBuf, clearW);
                }
                if (prevOutH > outH)
                {
                    const int16_t clearW = static_cast<int16_t>((prevOutW > outW) ? prevOutW : outW);
                    const int16_t clearH = static_cast<int16_t>(prevOutH - outH);
                    for (int16_t y = 0; y < clearH; ++y)
                        _disp.display->writeRect565(0, (int16_t)(outH + y), clearW, 1, _adaptivePreview.lineBuf, clearW);
                }

                switch (delta)
                {
                case 1:
                    for (uint16_t y = 0; y < virtW; ++y)
                    {
                        for (uint16_t x = 0; x < virtH; ++x)
                            _adaptivePreview.lineBuf[x] = src[(size_t)(virtH - 1U - x) * (size_t)stride + y];
                        _disp.display->writeRect565(0, (int16_t)y, (int16_t)virtH, 1, _adaptivePreview.lineBuf, virtH);
                    }
                    break;
                case 2:
                    for (uint16_t y = 0; y < virtH; ++y)
                    {
                        const uint16_t *srcRow = src + (size_t)(virtH - 1U - y) * (size_t)stride;
                        for (uint16_t x = 0; x < virtW; ++x)
                            _adaptivePreview.lineBuf[x] = srcRow[virtW - 1U - x];
                        _disp.display->writeRect565(0, (int16_t)y, (int16_t)virtW, 1, _adaptivePreview.lineBuf, virtW);
                    }
                    break;
                case 3:
                    for (uint16_t y = 0; y < virtW; ++y)
                    {
                        const uint16_t srcX = static_cast<uint16_t>(virtW - 1U - y);
                        for (uint16_t x = 0; x < virtH; ++x)
                            _adaptivePreview.lineBuf[x] = src[(size_t)x * (size_t)stride + srcX];
                        _disp.display->writeRect565(0, (int16_t)y, (int16_t)virtH, 1, _adaptivePreview.lineBuf, virtH);
                    }
                    break;
                }
            }

            _adaptivePreview.lastPresentedW = virtW;
            _adaptivePreview.lastPresentedH = virtH;
            _adaptivePreview.lastOutputW = outW;
            _adaptivePreview.lastOutputH = outH;
            reportPlatformErrorOnce(stage);
            pipcore::Platform *plat = pipcore::GetPlatform();
            return !plat || plat->lastError() == pipcore::PlatformError::None;
        }

        if (virtW == physW && virtH == physH)
        {
            _render.sprite.writeToDisplay(*_disp.display, 0, 0, (int16_t)physW, (int16_t)physH);
            _adaptivePreview.lastPresentedW = virtW;
            _adaptivePreview.lastPresentedH = virtH;
            _adaptivePreview.lastOutputW = virtW;
            _adaptivePreview.lastOutputH = virtH;
            reportPlatformErrorOnce(stage);
            pipcore::Platform *plat = pipcore::GetPlatform();
            return !plat || plat->lastError() == pipcore::PlatformError::None;
        }

        if (_adaptivePreview.lineBufCap < physW)
        {
            pipcore::Platform *plat = platform();
            uint16_t *newBuf = plat ? static_cast<uint16_t *>(plat->alloc(static_cast<size_t>(physW) * sizeof(uint16_t), pipcore::AllocCaps::PreferInternal)) : nullptr;
            if (!newBuf)
            {
                if (plat)
                    (void)detail::recoverFromAllocFailure(plat, static_cast<size_t>(physW) * sizeof(uint16_t), pipcore::AllocCaps::PreferInternal);
                newBuf = plat ? static_cast<uint16_t *>(plat->alloc(static_cast<size_t>(physW) * sizeof(uint16_t), pipcore::AllocCaps::Default)) : nullptr;
            }
            if (!newBuf && plat && detail::recoverFromAllocFailure(plat, static_cast<size_t>(physW) * sizeof(uint16_t), pipcore::AllocCaps::Default))
                newBuf = static_cast<uint16_t *>(plat->alloc(static_cast<size_t>(physW) * sizeof(uint16_t), pipcore::AllocCaps::Default));
            if (newBuf)
            {
                freeAdaptivePreviewBuffer(plat);
                _adaptivePreview.lineBuf = newBuf;
                _adaptivePreview.lineBufCap = physW;
            }
        }

        const bool canClearStrips = (_adaptivePreview.lineBuf && _adaptivePreview.lineBufCap >= physW);
        if (canClearStrips)
        {
            const uint16_t bg = __builtin_bswap16(_render.bgColor565);
            for (uint16_t x = 0; x < physW; ++x)
                _adaptivePreview.lineBuf[x] = bg;

            if (_adaptivePreview.lastPresentedW == 0 || _adaptivePreview.lastPresentedH == 0)
            {
                for (uint16_t y = 0; y < physH; ++y)
                {
                    _disp.display->writeRect565(0, static_cast<int16_t>(y), static_cast<int16_t>(physW), 1,
                                                _adaptivePreview.lineBuf, static_cast<int32_t>(physW));
                }
            }
            else
            {
                if (virtW < _adaptivePreview.lastPresentedW)
                {
                    const int16_t clearW = static_cast<int16_t>(_adaptivePreview.lastPresentedW - virtW);
                    const int16_t clearH = static_cast<int16_t>(std::max<uint16_t>(_adaptivePreview.lastPresentedH, virtH));
                    for (int16_t y = 0; y < clearH; ++y)
                    {
                        _disp.display->writeRect565((int16_t)virtW, y, clearW, 1,
                                                    _adaptivePreview.lineBuf, clearW);
                    }
                }

                if (virtH < _adaptivePreview.lastPresentedH)
                {
                    const int16_t clearW = static_cast<int16_t>(std::max<uint16_t>(_adaptivePreview.lastPresentedW, virtW));
                    const int16_t clearH = static_cast<int16_t>(_adaptivePreview.lastPresentedH - virtH);
                    for (int16_t y = 0; y < clearH; ++y)
                    {
                        _disp.display->writeRect565(0, (int16_t)(virtH + y), clearW, 1,
                                                    _adaptivePreview.lineBuf, clearW);
                    }
                }
            }
        }
        _disp.display->writeRect565(0, 0, (int16_t)virtW, (int16_t)virtH, src, stride);
        _adaptivePreview.lastPresentedW = virtW;
        _adaptivePreview.lastPresentedH = virtH;
        _adaptivePreview.lastOutputW = virtW;
        _adaptivePreview.lastOutputH = virtH;

        reportPlatformErrorOnce(stage);
        pipcore::Platform *plat = pipcore::GetPlatform();
        return !plat || plat->lastError() == pipcore::PlatformError::None;
    }

    bool GUI::presentTransformedSprite(const uint16_t *src, int16_t srcStride, int16_t srcW, int16_t srcH,
                                       int16_t logicalW, int16_t logicalH,
                                       float angleRad, float scale, const char *stage)
    {
        if (!_disp.display || !src || srcStride <= 0 || srcW <= 0 || srcH <= 0 || logicalW <= 0 || logicalH <= 0)
            return false;

        const uint16_t physW = _disp.display->width();
        const uint16_t physH = _disp.display->height();
        if (physW == 0 || physH == 0)
            return false;

        if (_rotationAnim.lineBufCap < physW)
        {
            pipcore::Platform *plat = platform();
            uint16_t *newBuf = plat ? static_cast<uint16_t *>(plat->alloc(static_cast<size_t>(physW) * sizeof(uint16_t), pipcore::AllocCaps::PreferInternal)) : nullptr;
            if (!newBuf)
            {
                if (plat)
                    (void)detail::recoverFromAllocFailure(plat, static_cast<size_t>(physW) * sizeof(uint16_t), pipcore::AllocCaps::PreferInternal);
                newBuf = plat ? static_cast<uint16_t *>(plat->alloc(static_cast<size_t>(physW) * sizeof(uint16_t), pipcore::AllocCaps::Default)) : nullptr;
            }
            if (!newBuf && plat && detail::recoverFromAllocFailure(plat, static_cast<size_t>(physW) * sizeof(uint16_t), pipcore::AllocCaps::Default))
                newBuf = static_cast<uint16_t *>(plat->alloc(static_cast<size_t>(physW) * sizeof(uint16_t), pipcore::AllocCaps::Default));
            if (newBuf)
            {
                freeRotationLineBuffer(plat);
                _rotationAnim.lineBuf = newBuf;
                _rotationAnim.lineBufCap = physW;
            }
        }

        if (!_rotationAnim.lineBuf || _rotationAnim.lineBufCap < physW)
            return false;

        const float safeScale = (scale < 0.08f) ? 0.08f : ((scale > 1.15f) ? 1.15f : scale);
        const float invScale = 1.0f / safeScale;
        const float cosA = cosf(angleRad);
        const float sinA = sinf(angleRad);
        const float srcCx = ((float)logicalW - 1.0f) * 0.5f;
        const float srcCy = ((float)logicalH - 1.0f) * 0.5f;
        const float dstCx = ((float)physW - 1.0f) * 0.5f;
        const float dstCy = ((float)physH - 1.0f) * 0.5f;
        const uint16_t bg = __builtin_bswap16(_render.bgColor565);
        const float sampleScaleX = (logicalW > 1 && srcW > 1) ? ((float)(srcW - 1) / (float)(logicalW - 1)) : 1.0f;
        const float sampleScaleY = (logicalH > 1 && srcH > 1) ? ((float)(srcH - 1) / (float)(logicalH - 1)) : 1.0f;

        for (uint16_t x = 0; x < physW; ++x)
            _rotationAnim.lineBuf[x] = bg;

        for (uint16_t y = 0; y < physH; ++y)
        {
            const float dy = ((float)y - dstCy) * invScale;
            float srcXf = (((0.0f - dstCx) * invScale) * cosA + dy * sinA) + srcCx;
            float srcYf = (-((0.0f - dstCx) * invScale) * sinA + dy * cosA) + srcCy;
            const float stepX = invScale * cosA;
            const float stepY = -invScale * sinA;
            for (uint16_t x = 0; x < physW; ++x)
            {
                const int16_t logicalXi = (int16_t)lroundf(srcXf);
                const int16_t logicalYi = (int16_t)lroundf(srcYf);
                if (logicalXi < 0 || logicalYi < 0 || logicalXi >= logicalW || logicalYi >= logicalH)
                    _rotationAnim.lineBuf[x] = bg;
                else
                {
                    const int16_t srcXi = static_cast<int16_t>(lroundf((float)logicalXi * sampleScaleX));
                    const int16_t srcYi = static_cast<int16_t>(lroundf((float)logicalYi * sampleScaleY));
                    _rotationAnim.lineBuf[x] = src[(size_t)srcYi * (size_t)srcStride + (size_t)srcXi];
                }
                srcXf += stepX;
                srcYf += stepY;
            }

            _disp.display->writeRect565(0, (int16_t)y, (int16_t)physW, 1, _rotationAnim.lineBuf, physW);
        }

        reportPlatformErrorOnce(stage);
        pipcore::Platform *plat = pipcore::GetPlatform();
        return !plat || plat->lastError() == pipcore::PlatformError::None;
    }

    bool GUI::applyLogicalRotation(uint8_t rotation, bool allowAutoTiledFallback)
    {
        const uint8_t prevRotation = _disp.rotation;
        const uint16_t prevScreenW = _render.screenWidth;
        const uint16_t prevScreenH = _render.screenHeight;
        const bool wasTiled = _flags.tiledMode != 0;
        const bool wasAutoTiled = _flags.autoTiledMode != 0;

        const auto tryCreateCanvas = [&](uint16_t screenW, uint16_t screenH, bool tiled, bool autoTiled) noexcept -> bool
        {
            _render.sprite.deleteSprite();
            _flags.tiledMode = 0;
            _flags.autoTiledMode = 0;

            const int16_t sw = static_cast<int16_t>(screenW);
            const int16_t sh = static_cast<int16_t>(screenH);

            if (!tiled)
            {
                const bool ok = _render.sprite.createSprite(sw, sh);
                _flags.spriteEnabled = ok ? 1U : 0U;
                _render.activeSprite = ok ? &_render.sprite : nullptr;
                return ok;
            }

            int16_t targetH = (sh > 1) ? static_cast<int16_t>((sh + 1) / 2) : sh;
            bool ok = _render.sprite.createSprite(sw, targetH);

            if (!ok)
            {
                targetH = (sh > 1) ? static_cast<int16_t>((sh + 3) / 4) : sh;
                ok = _render.sprite.createSprite(sw, targetH);
            }

            if (!ok)
            {
                pipcore::Platform *plat = platform();
                const size_t bytes = static_cast<size_t>(sw) * static_cast<size_t>(targetH) * sizeof(uint16_t);
                if (plat && bytes > 0 && detail::recoverFromAllocFailure(plat, bytes, pipcore::AllocCaps::Default))
                {
                    targetH = (sh > 1) ? static_cast<int16_t>((sh + 1) / 2) : sh;
                    ok = _render.sprite.createSprite(sw, targetH);
                    if (!ok)
                    {
                        targetH = (sh > 1) ? static_cast<int16_t>((sh + 3) / 4) : sh;
                        ok = _render.sprite.createSprite(sw, targetH);
                    }
                }
            }

            _flags.tiledMode = ok ? 1U : 0U;
            _flags.autoTiledMode = (ok && autoTiled) ? 1U : 0U;

            _flags.spriteEnabled = ok ? 1U : 0U;
            _render.activeSprite = _flags.spriteEnabled ? &_render.sprite : nullptr;
            return ok;
        };

        const auto recreateCanvas = [&](uint16_t screenW, uint16_t screenH, bool preferTiled) noexcept -> bool
        {
            if (preferTiled && !wasAutoTiled)
                return tryCreateCanvas(screenW, screenH, true, false);

            if (tryCreateCanvas(screenW, screenH, preferTiled, preferTiled && wasAutoTiled))
                return true;
            if (allowAutoTiledFallback && tryCreateCanvas(screenW, screenH, !preferTiled, !preferTiled))
                return true;
            return false;
        };

        if (rotation == _disp.rotation)
            return true;

        _disp.rotation = rotation & 3U;
        const bool quarterTurn = ((_disp.rotation & 1U) != 0U);
        const uint16_t targetW = quarterTurn ? _render.physicalHeight : _render.physicalWidth;
        const uint16_t targetH = quarterTurn ? _render.physicalWidth : _render.physicalHeight;

        if (recreateCanvas(targetW, targetH, wasTiled))
        {
            _render.screenWidth = targetW;
            _render.screenHeight = targetH;
            Debug::setCanvasSize((int16_t)_render.screenWidth, (int16_t)_render.screenHeight);
            _flags.needRedraw = 1;
            return true;
        }

        _disp.rotation = prevRotation;
        (void)recreateCanvas(prevScreenW, prevScreenH, wasTiled);
        return false;
    }

    void GUI::setRotation(uint8_t rotation, uint32_t durationMs)
    {
        rotation &= 3U;

        if (_rotationAnim.active)
        {
            _rotationAnim.active = false;
            _rotationAnim.switched = false;
            _rotationAnim.startedTiled = false;
            _rotationAnim.startedAutoTiled = false;
            freeRotationBuffer(platform());
        }

        if (!_disp.display || !_flags.spriteEnabled ||
            _flags.bootActive || _flags.errorActive || _flags.notifActive ||
            _flags.popupActive || _flags.toastActive || _flags.screenTransition)
        {
            if (_disp.rotation != rotation)
            {
                if (applyLogicalRotation(rotation))
                    requestRedraw();
            }
            return;
        }

        if (_disp.rotation == rotation)
            return;

        const bool startedTiled = _flags.tiledMode != 0;
        const bool startedAutoTiled = _flags.autoTiledMode != 0;
        pipcore::Platform *plat = platform();

        if (startedTiled)
        {
            freeScreenshotStream(plat);
            freeBlurBuffers(plat);
            freeRotationLineBuffer(plat);
            (void)releaseGraphCachesForRecovery(plat);
        }

        if (!startedTiled)
        {
            const ScreenCallback currentCb = (_screen.current < _screen.capacity && _screen.callbacks)
                                                 ? _screen.callbacks[_screen.current]
                                                 : nullptr;

            if (_screen.current < _screen.capacity)
            {
                if (currentCb)
                    renderScreenToMainSprite(currentCb, _screen.current);
                else
                    clear(_render.bgColor565 ? _render.bgColor565 : (uint16_t)_render.bgColor);
                renderStatusBar();
            }
            else
            {
                clear(_render.bgColor565 ? _render.bgColor565 : (uint16_t)_render.bgColor);
            }
        }

        if (!_render.sprite.getBuffer())
        {
            if (applyLogicalRotation(rotation))
                requestRedraw();
            return;
        }

        _rotationAnim.active = true;
        _rotationAnim.switched = false;
        _rotationAnim.startedTiled = startedTiled;
        _rotationAnim.startedAutoTiled = startedAutoTiled;
        _rotationAnim.from = _disp.rotation;
        _rotationAnim.to = rotation;
        _rotationAnim.startMs = nowMs();
        _rotationAnim.durationMs = durationMs ? durationMs : 520;
        _dirty.count = 0;
        _flags.dirtyRedrawPending = 0;
        _flags.needRedraw = 0;
        Debug::clearRects();
    }

    bool GUI::rotationTransitionActive() const noexcept
    {
        return _rotationAnim.active;
    }

    void GUI::renderRotationTransition(uint32_t now)
    {
        if (!_rotationAnim.active || !_disp.display || !_flags.spriteEnabled)
            return;

        const uint32_t duration = _rotationAnim.durationMs ? _rotationAnim.durationMs : 1;
        uint32_t elapsed = now - _rotationAnim.startMs;
        if (elapsed > duration)
            elapsed = duration;

        const float t = static_cast<float>(elapsed) / static_cast<float>(duration);
        const bool switchedThisFrame = (!_rotationAnim.switched && t >= 0.5f);
        if (!_rotationAnim.switched && t >= 0.5f)
        {
            if (!applyLogicalRotation(_rotationAnim.to, false))
            {
                const bool restoreForcedTiled = _rotationAnim.startedTiled && !_rotationAnim.startedAutoTiled;
                _rotationAnim.active = false;
                _rotationAnim.switched = false;
                _rotationAnim.startedTiled = false;
                _rotationAnim.startedAutoTiled = false;
                freeRotationBuffer(platform());
                if (restoreForcedTiled)
                {
                    const int16_t sw = static_cast<int16_t>(_render.screenWidth);
                    const int16_t sh = static_cast<int16_t>(_render.screenHeight);
                    const int16_t tileH = (sh > 1) ? static_cast<int16_t>((sh + 1) / 2) : sh;
                    _render.sprite.deleteSprite();
                    const bool ok = _render.sprite.createSprite(sw, tileH);
                    _flags.spriteEnabled = ok ? 1U : 0U;
                    _flags.tiledMode = ok ? 1U : 0U;
                    _flags.autoTiledMode = 0;
                    _render.activeSprite = ok ? &_render.sprite : nullptr;
                }
                requestRedraw();
                return;
            }

            _rotationAnim.switched = true;

            if (!_flags.tiledMode)
            {
                const ScreenCallback currentCb = (_screen.current < _screen.capacity && _screen.callbacks)
                                                     ? _screen.callbacks[_screen.current]
                                                     : nullptr;

                if (_screen.current < _screen.capacity)
                {
                    if (currentCb)
                        renderScreenToMainSprite(currentCb, _screen.current);
                    else
                        clear(_render.bgColor565 ? _render.bgColor565 : (uint16_t)_render.bgColor);
                    renderStatusBar();
                }
                else
                {
                    clear(_render.bgColor565 ? _render.bgColor565 : (uint16_t)_render.bgColor);
                }
            }
        }

        const bool shouldRenderLiveFrame = (_rotationAnim.switched && !switchedThisFrame && !_flags.tiledMode);
        const ScreenCallback currentCb = (_screen.current < _screen.capacity && _screen.callbacks)
                                             ? _screen.callbacks[_screen.current]
                                             : nullptr;

        if (shouldRenderLiveFrame && _screen.current < _screen.capacity)
        {
            if (currentCb)
                renderScreenToMainSprite(currentCb, _screen.current);
            else
                clear(_render.bgColor565 ? _render.bgColor565 : (uint16_t)_render.bgColor);
            renderStatusBar();
        }

        const float startAngle = presentationAngleRad(_rotationAnim.from);
        const float endAngle = presentationAngleRad(_rotationAnim.to);
        float deltaAngle = endAngle - startAngle;
        if (deltaAngle > 3.1415926535f)
            deltaAngle -= 6.28318530718f;
        else if (deltaAngle < -3.1415926535f)
            deltaAngle += 6.28318530718f;
        const float eased = detail::motion::easeInOutCubic(t);
        const float angle = startAngle + (deltaAngle * eased);
        const float scale = 1.0f - (0.10f * sinf(t * 3.1415926535f));

        bool presented = false;
        if (_flags.tiledMode)
        {
            presented = presentTransformedTiledFrame(angle, scale, now, "present");
        }
        else
        {
            const uint16_t *src = static_cast<const uint16_t *>(_render.sprite.getBuffer());
            const int16_t srcStride = _render.sprite.width();
            const int16_t srcW = static_cast<int16_t>(_render.screenWidth);
            const int16_t srcH = static_cast<int16_t>(_render.screenHeight);
            const int16_t logicalW = static_cast<int16_t>(_render.screenWidth);
            const int16_t logicalH = static_cast<int16_t>(_render.screenHeight);
            presented = presentTransformedSprite(src,
                                                 srcStride,
                                                 srcW,
                                                 srcH,
                                                 logicalW,
                                                 logicalH,
                                                 angle,
                                                 scale,
                                                 "present");
        }

        if (!presented)
        {
            _rotationAnim.active = false;
            _rotationAnim.switched = false;
            freeRotationBuffer(platform());
            if (_disp.rotation != _rotationAnim.to)
                (void)applyLogicalRotation(_rotationAnim.to);
            requestRedraw();
            return;
        }

        if (elapsed >= duration)
        {
            const bool restoreForcedTiled = _rotationAnim.startedTiled && !_rotationAnim.startedAutoTiled && !_flags.tiledMode;
            _rotationAnim.active = false;
            _rotationAnim.switched = false;
            freeRotationBuffer(platform());
            if (restoreForcedTiled)
            {
                const int16_t sw = static_cast<int16_t>(_render.screenWidth);
                const int16_t sh = static_cast<int16_t>(_render.screenHeight);
                const int16_t tileH = (sh > 1) ? static_cast<int16_t>((sh + 1) / 2) : sh;
                _render.sprite.deleteSprite();
                bool ok = _render.sprite.createSprite(sw, tileH);
                if (!ok)
                {
                    pipcore::Platform *plat = platform();
                    const size_t bytes = static_cast<size_t>(sw) * static_cast<size_t>(tileH) * sizeof(uint16_t);
                    if (plat && bytes > 0 && detail::recoverFromAllocFailure(plat, bytes, pipcore::AllocCaps::Default))
                        ok = _render.sprite.createSprite(sw, tileH);
                }
                _flags.spriteEnabled = ok ? 1U : 0U;
                _flags.tiledMode = ok ? 1U : 0U;
                _flags.autoTiledMode = 0;
                _render.activeSprite = ok ? &_render.sprite : nullptr;
                _clip = {};
            }
            _dirty.count = 0;
            _flags.dirtyRedrawPending = 0;
            _flags.needRedraw = 1;
            Debug::clearRects();
        }
    }

    void GUI::initFonts()
    {
        if (_typo.psdfSizePx == 0)
            _typo.psdfSizePx = 14;
    }

    void GUI::setBackgroundColorCache(uint16_t color565) noexcept
    {
        _render.bgColor = detail::color565To888(color565);
        _render.bgColor565 = color565;
    }

    uint32_t GUI::nowMs() const
    {
        return pipcore::GetPlatform()->nowMs();
    }

    pipcore::Platform *GUI::platform() const noexcept
    {
        return pipcore::GetPlatform();
    }

    void GUI::requestWiFi(bool enabled) noexcept
    {
        net::wifiRequest(enabled);
    }

    net::WifiState GUI::wifiState() const noexcept
    {
        return net::wifiState();
    }

    bool GUI::wifiConnected() const noexcept
    {
        return net::wifiConnected();
    }

    uint32_t GUI::wifiLocalIpV4() const noexcept
    {
        return net::wifiLocalIpV4();
    }

    namespace
    {
        [[nodiscard]] pipcore::ota::Options buildOtaOptions() noexcept
        {
            constexpr pipgui::config::FirmwareVersion fw = pipgui::config::firmwareVersion();
            pipcore::ota::Options opt;
            opt.currentVerMajor = fw.major;
            opt.currentVerMinor = fw.minor;
            opt.currentVerPatch = fw.patch;
            opt.currentBuild = pipgui::config::firmwareBuildNumber();

#if PIPGUI_OTA
#if !defined(PIPGUI_OTA_ED25519_PUBKEY_HEX)
#error "Define PIPGUI_OTA_ED25519_PUBKEY_HEX (64 hex chars) in include/config.hpp"
#endif
            static_assert(sizeof(PIPGUI_OTA_ED25519_PUBKEY_HEX) == 65, "PIPGUI_OTA_ED25519_PUBKEY_HEX must be 64 hex chars");
            opt.ed25519PubkeyHex = PIPGUI_OTA_ED25519_PUBKEY_HEX;
#else
            opt.ed25519PubkeyHex = "";
#endif
            return opt;
        }
    }

    void GUI::otaConfigure(OtaStatusCallback cb, void *user) noexcept
    {
#if PIPGUI_OTA
        pipcore::ota::configure(buildOtaOptions(), cb, user);
#else
        (void)cb;
        (void)user;
#endif
    }

    void GUI::otaRequestCheck() noexcept
    {
#if PIPGUI_OTA
        pipcore::ota::requestCheck();
#endif
    }

    void GUI::otaRequestCheck(OtaCheckMode mode) noexcept
    {
#if PIPGUI_OTA
        pipcore::ota::requestCheck(mode);
#else
        (void)mode;
#endif
    }

    void GUI::otaRequestInstall() noexcept
    {
#if PIPGUI_OTA
        pipcore::ota::requestInstall();
#endif
    }

    void GUI::otaRequestStableList() noexcept
    {
#if PIPGUI_OTA
        pipcore::ota::requestStableList();
#endif
    }

    bool GUI::otaStableListReady() const noexcept
    {
#if PIPGUI_OTA
        return pipcore::ota::stableListReady();
#else
        return false;
#endif
    }

    uint8_t GUI::otaStableListCount() const noexcept
    {
#if PIPGUI_OTA
        return pipcore::ota::stableListCount();
#else
        return 0;
#endif
    }

    const char *GUI::otaStableListVersion(uint8_t idx) const noexcept
    {
#if PIPGUI_OTA
        return pipcore::ota::stableListVersion(idx);
#else
        (void)idx;
        return "";
#endif
    }

    void GUI::otaRequestInstallStableVersion(const char *version) noexcept
    {
#if PIPGUI_OTA
        pipcore::ota::requestInstallStableVersion(version);
#else
        (void)version;
#endif
    }

    void GUI::otaCancel() noexcept
    {
#if PIPGUI_OTA
        pipcore::ota::cancel();
#endif
    }

    void GUI::otaService() noexcept
    {
#if PIPGUI_OTA
        pipcore::ota::service();
#endif
    }

    void GUI::otaMarkAppValid() noexcept
    {
#if PIPGUI_OTA
        pipcore::ota::markAppValid();
#endif
    }

    const OtaStatus &GUI::otaStatus() const noexcept
    {
#if PIPGUI_OTA
        return pipcore::ota::status();
#else
        static const OtaStatus disabledStatus = {};
        return disabledStatus;
#endif
    }

    void GUI::setBacklightHandler(BacklightHandler handler) noexcept
    {
        _disp.backlightHandler = handler;
    }

    void GUI::setBrightness(uint8_t percent)
    {
        if (percent > 100)
            percent = 100;
        if (percent > _disp.brightnessMax)
            percent = _disp.brightnessMax;
        _disp.brightness = percent;
        if (_disp.backlightHandler)
            _disp.backlightHandler(_disp.brightness);
    }

    void GUI::setMaxBrightness(uint8_t percent)
    {
        if (percent > 100)
            percent = 100;
        _disp.brightnessMax = percent;
        if (_disp.brightness > _disp.brightnessMax)
            _disp.brightness = _disp.brightnessMax;
        pipcore::GetPlatform()->storeMaxBrightnessPercent(_disp.brightnessMax);
        if (_disp.backlightHandler)
            _disp.backlightHandler(_disp.brightness);
    }

    void GUI::setScreenAnim(ScreenAnim anim, uint32_t durationMs)
    {
        _screen.anim = anim;
        _screen.animDurationMs = durationMs;
    }
}
