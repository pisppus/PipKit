#include <Arduino.h>
#include <math.h>

#include "demo.hpp"

#if PIPCORE_ENABLE_PREFS && PIPCORE_TARGET_ESP32
#include <Preferences.h>
#endif

using namespace pipgui;

namespace
{

#ifndef PIPGUI_DEMO_BTN_NEXT_PIN
#define PIPGUI_DEMO_BTN_NEXT_PIN 40
#endif

#ifndef PIPGUI_DEMO_BTN_PREV_PIN
#define PIPGUI_DEMO_BTN_PREV_PIN 18
#endif

#ifndef PIPGUI_DEMO_BTN_SELECT_PIN
#define PIPGUI_DEMO_BTN_SELECT_PIN -1
#endif

#ifndef PIPGUI_DEMO_BACKLIGHT_PIN
#define PIPGUI_DEMO_BACKLIGHT_PIN 255
#endif

    constexpr int16_t kBtnNextPinRaw = PIPGUI_DEMO_BTN_NEXT_PIN;
    constexpr int16_t kBtnPrevPinRaw = PIPGUI_DEMO_BTN_PREV_PIN;
    constexpr int16_t kBtnSelectPinRaw = PIPGUI_DEMO_BTN_SELECT_PIN;
    constexpr bool kHasSelectButton = kBtnSelectPinRaw >= 0;
    constexpr uint8_t kBtnNextPin = static_cast<uint8_t>(kBtnNextPinRaw);
    constexpr uint8_t kBtnPrevPin = static_cast<uint8_t>(kBtnPrevPinRaw);
    constexpr uint8_t kBtnSelectPin = kHasSelectButton ? static_cast<uint8_t>(kBtnSelectPinRaw) : 0;
    constexpr uint8_t kBacklightPin = static_cast<uint8_t>(PIPGUI_DEMO_BACKLIGHT_PIN);

    static_assert(kBtnNextPinRaw >= 0 && kBtnPrevPinRaw >= 0, "Demo buttons must be valid");
    static_assert(kBtnNextPin != kBtnPrevPin, "Demo buttons must be distinct");
    static_assert(!kHasSelectButton || kBtnNextPin != kBtnSelectPin, "Demo buttons must be distinct");
    static_assert(!kHasSelectButton || kBtnPrevPin != kBtnSelectPin, "Demo buttons must be distinct");

    Button btnNext(kBtnNextPin, Pullup);
    Button btnPrev(kBtnPrevPin, Pullup);
    Button btnSelect(kBtnSelectPin, Pullup);

    DemoState gState;

#if PIPCORE_ENABLE_PREFS && PIPCORE_TARGET_ESP32
    constexpr const char *kDemoPrefsNs = "pipdemo";
    constexpr const char *kDemoPrefsKey = "runtime";

    uint16_t packRuntimePrefs() noexcept
    {
        uint16_t bits = 0;
        if (gState.forceTiles)
            bits |= 0x0001u;
        if (gState.debugLayoutBounds)
            bits |= 0x0002u;
        if (gState.debugOverdraw)
            bits |= 0x0004u;
        if (gState.debugMetrics)
            bits |= 0x0008u;
        if (gState.normalStatusBar)
            bits |= 0x0010u;
        bits |= static_cast<uint16_t>((gState.screenshotModeChoice & 0x03u) << 5u);
        return bits;
    }

    void unpackRuntimePrefs(uint16_t bits) noexcept
    {
        gState.forceTiles = (bits & 0x0001u) != 0;
        gState.debugLayoutBounds = (bits & 0x0002u) != 0;
        gState.debugOverdraw = (bits & 0x0004u) != 0;
        gState.debugMetrics = (bits & 0x0008u) != 0;
        gState.normalStatusBar = (bits & 0x0010u) != 0;
        gState.screenshotModeChoice = static_cast<uint8_t>((bits >> 5u) & 0x03u);
        if (gState.screenshotModeChoice > 2u)
            gState.screenshotModeChoice = (PIPGUI_SCREENSHOT_MODE <= 2) ? PIPGUI_SCREENSHOT_MODE : 2;
    }

    void loadRuntimePrefs() noexcept
    {
        Preferences prefs;
        if (!prefs.begin(kDemoPrefsNs, true))
            return;
        unpackRuntimePrefs(prefs.getUShort(kDemoPrefsKey, packRuntimePrefs()));
        prefs.end();
    }

    void storeRuntimePrefs() noexcept
    {
        Preferences prefs;
        if (!prefs.begin(kDemoPrefsNs, false))
            return;
        (void)prefs.putUShort(kDemoPrefsKey, packRuntimePrefs());
        prefs.end();
    }
#else
    void loadRuntimePrefs() noexcept {}
    void storeRuntimePrefs() noexcept {}
#endif

    void stepBattery(uint8_t &level, bool &down) noexcept
    {
        if (down)
        {
            if (level > 0)
                --level;
            else
                down = false;
        }
        else
        {
            if (level < 100)
                ++level;
            else
                down = true;
        }
    }

    void configureStatusBar()
    {
#if (PIPGUI_STATUS_BAR != 0)
        ui.configStatusBar()
            .height(20)
            .pos(Top)
            .style(Solid);
#endif
    }

    void updateStatusBarText(uint32_t nowMs)
    {
#if (PIPGUI_STATUS_BAR != 0)
        if (gState.debugMetrics || !gState.normalStatusBar)
        {
            ui.setStatusBarText().left("").center("").right("");
            ui.clearStatusBarIcon(Left);
            ui.clearStatusBarIcon(Center);
            ui.clearStatusBarIcon(Right);
            ui.setStatusBarBattery(-1, Hidden);
            if (!ui.notificationActive())
                ui.updateStatusBar();
            return;
        }

        const uint32_t minute = (nowMs / 1000u) / 60u;
        if (minute == gState.lastClockMinute)
            return;

        gState.lastClockMinute = minute;
        char clockBuf[6];
        const uint8_t hh = static_cast<uint8_t>((minute / 60u) % 24u);
        const uint8_t mm = static_cast<uint8_t>(minute % 60u);
        snprintf(clockBuf, sizeof(clockBuf), "%02u:%02u", static_cast<unsigned>(hh), static_cast<unsigned>(mm));

        ui.setStatusBarText()
            .left("")
            .center(String(clockBuf))
            .right("");
        ui.setStatusBarIcon().side(Left).icon(checkmark).color(accent(ui)).size(13);
        ui.setStatusBarIcon().side(Right).icon(arrow).color(accent2(ui)).size(13);
        if (!ui.notificationActive())
            ui.updateStatusBar();
#else
        (void)nowMs;
#endif
    }

    void updateStatusBarBattery(uint32_t nowMs)
    {
#if (PIPGUI_STATUS_BAR != 0)
        if (gState.debugMetrics || !gState.normalStatusBar)
            return;

        if ((nowMs - gState.lastBatteryMs) < 280)
            return;

        gState.lastBatteryMs = nowMs;
        stepBattery(gState.batteryLevel, gState.batteryDown);
        ui.setStatusBarBattery(gState.batteryLevel, Bar);
        if (!ui.notificationActive())
            ui.updateStatusBar();
#else
        (void)nowMs;
#endif
    }

    void applyDebugRuntime()
    {
        Debug::setLayoutBoundsEnabled(gState.debugLayoutBounds);
        Debug::setOverdrawEnabled(gState.debugOverdraw);
        Debug::setEnabled(gState.debugMetrics);
        ui.requestRedraw();
    }

    void rebeginDisplay()
    {
        const uint8_t current = ui.currentScreen();
        ui.begin(3, gState.forceTiles);
        configureStatusBar();
        updateStatusBarText(millis());
        ui.setStatusBarBattery(gState.batteryLevel, Bar);
        if (kBacklightPin != 255)
            ui.setBacklight().pin(kBacklightPin);
        ui.setMaxBrightness(gState.maxBrightness);
        ui.setBrightness(gState.brightness);
        applyDebugRuntime();
        if (current != INVALID_SCREEN_ID)
        {
            ui.requestRedraw();
            ui.loop();
        }
    }

    void showBuildTimeScreenshotToast()
    {
        ui.showToast()
            .text(String("Screenshot mode is build-time: ") + screenshotModeLabel(PIPGUI_SCREENSHOT_MODE))
            .pos(down)
            .icon(warning);
    }

    void onScreenLeave(uint8_t screenId)
    {
        if (screenId == ScreenAdapt)
            ui.clearAdaptivePreview();
    }

    void onScreenEnter(uint8_t screenId)
    {
        if (screenId == ScreenAdapt)
            ui.setAdaptivePreview(150, 108, 5200);

        switch (screenId)
        {
        case ScreenMenu:
            (void)ui.listNav();
            break;

        case ScreenDebug:
            ui.nav().handler([](GUI &, const NavEvent &event, void *) -> bool
                             {
                                 if (event.button == NavButton::Prev && event.code == NavEventCode::LongPressed)
                                 {
                                     ui.backScreen();
                                     return true;
                                 }

                                 if (event.button == NavButton::Prev && event.code == NavEventCode::Released && !event.longPress)
                                 {
                                     gState.debugRow = (gState.debugRow > 0u) ? static_cast<uint8_t>(gState.debugRow - 1u) : 5u;
                                     ui.requestRedraw();
                                     return true;
                                 }

                                 if (event.button == NavButton::Next && event.code == NavEventCode::Released && !event.longPress)
                                 {
                                     gState.debugRow = static_cast<uint8_t>((gState.debugRow + 1u) % 6u);
                                     ui.requestRedraw();
                                     return true;
                                 }

                                 const bool activate = (event.button == NavButton::Select && event.code == NavEventCode::Released && !event.longPress && event.hasSelect) ||
                                                       (event.button == NavButton::Next && event.code == NavEventCode::LongPressed && !event.hasSelect) ||
                                                       (event.button == NavButton::Combo && event.code == NavEventCode::Released && !event.longPress);
                                 if (!activate)
                                     return false;

                                 switch (gState.debugRow)
                                 {
                                 case 0:
                                     gState.forceTiles = !gState.forceTiles;
                                     storeRuntimePrefs();
                                     rebeginDisplay();
                                     ui.showToast()
                                         .text(ui.tiledMode() ? "Tiled mode enabled" : "Fullscreen sprite enabled")
                                         .icon(checkmark);
                                     return true;
                                 case 1:
                                     gState.debugLayoutBounds = !gState.debugLayoutBounds;
                                     storeRuntimePrefs();
                                     applyDebugRuntime();
                                     return true;
                                 case 2:
                                     gState.debugOverdraw = !gState.debugOverdraw;
                                     storeRuntimePrefs();
                                     applyDebugRuntime();
                                     return true;
                                 case 3:
                                     gState.debugMetrics = !gState.debugMetrics;
                                     storeRuntimePrefs();
                                     applyDebugRuntime();
                                     gState.lastClockMinute = 0xFFFFFFFFu;
                                     updateStatusBarText(millis());
                                     return true;
                                 case 4:
                                     gState.normalStatusBar = !gState.normalStatusBar;
                                     storeRuntimePrefs();
                                     gState.lastClockMinute = 0xFFFFFFFFu;
                                     updateStatusBarText(millis());
                                     ui.requestRedraw();
                                     return true;
                                 case 5:
                                     gState.screenshotModeChoice = static_cast<uint8_t>((gState.screenshotModeChoice + 1u) % 3u);
                                     storeRuntimePrefs();
                                     showBuildTimeScreenshotToast();
                                     ui.requestRedraw();
                                     return true;
                                 default:
                                     return false;
                                 } });
            break;
        case ScreenRotate:
            ui.nav().handler([](GUI &, const NavEvent &event, void *) -> bool
                             {
                                 if (event.button == NavButton::Prev && event.code == NavEventCode::LongPressed)
                                 {
                                     ui.backScreen();
                                     return true;
                                 }

                                 if (event.button == NavButton::Prev && event.code == NavEventCode::Released && !event.longPress)
                                 {
                                     gState.rotationRow = (gState.rotationRow > 0u) ? static_cast<uint8_t>(gState.rotationRow - 1u) : 4u;
                                     ui.requestRedraw();
                                     return true;
                                 }

                                 if (event.button == NavButton::Next && event.code == NavEventCode::Released && !event.longPress)
                                 {
                                     gState.rotationRow = static_cast<uint8_t>((gState.rotationRow + 1u) % 5u);
                                     ui.requestRedraw();
                                     return true;
                                 }

                                 const bool activate = (event.button == NavButton::Select && event.code == NavEventCode::Released && !event.longPress && event.hasSelect) ||
                                                       (event.button == NavButton::Next && event.code == NavEventCode::LongPressed && !event.hasSelect) ||
                                                       (event.button == NavButton::Combo && event.code == NavEventCode::Released && !event.longPress);
                                 if (!activate)
                                     return false;

                                 if (gState.rotationRow < 4u)
                                 {
                                     const uint32_t durationMs = gState.rotationSlowAnim ? 1200u : 520u;
                                     ui.setRotation(gState.rotationRow, durationMs);
                                     ui.requestRedraw();
                                     return true;
                                 }

                                 gState.rotationSlowAnim = !gState.rotationSlowAnim;
                                 ui.requestRedraw();
                                 return true;
            });
            break;
        case ScreenLight:
            ui.nav().handler([](GUI &, const NavEvent &event, void *) -> bool
                             {
                                 if (event.button == NavButton::Prev && event.code == NavEventCode::LongPressed)
                                 {
                                     ui.backScreen();
                                     return true;
                                 }
                                 if (event.button == NavButton::Prev && event.code == NavEventCode::Released && !event.longPress)
                                 {
                                     gState.lightRow ^= 1u;
                                     ui.requestRedraw();
                                     return true;
                                 }
                                 if (event.button == NavButton::Next && event.code == NavEventCode::Released && !event.longPress)
                                 {
                                     uint8_t &v = (gState.lightRow == 0u) ? gState.brightness : gState.maxBrightness;
                                     v = static_cast<uint8_t>((v >= 100u) ? 0u : (v + 10u));
                                     if (gState.brightness > gState.maxBrightness)
                                         gState.brightness = gState.maxBrightness;
                                     ui.setMaxBrightness(gState.maxBrightness);
                                     ui.setBrightness(gState.brightness);
                                     ui.requestRedraw();
                                     return true;
                                 }
                                 return false;
            });
            break;
        case ScreenDots7:
        case ScreenDotsMany:
        case ScreenDrumH:
        case ScreenDrumV:
        case ScreenListPlain:
        case ScreenButtonUp:
        case ScreenToggleUp:
        case ScreenSliderUp:
        case ScreenProgressUp:
        case ScreenCircleProgUp:
            ui.nav().handler([](GUI &, const NavEvent &event, void *) -> bool
                             {
                                 if (event.button == NavButton::Prev && event.code == NavEventCode::LongPressed)
                                 {
                                     ui.backScreen();
                                     return true;
                                 }

                                 if (event.code != NavEventCode::Released || event.longPress)
                                     return false;

                                 const uint8_t current = ui.currentScreen();
                                 if (current == ScreenDots7)
                                 {
                                     if (event.button == NavButton::Next)
                                         gState.dots7 = static_cast<uint8_t>((gState.dots7 + 1u) % 7u);
                                     else if (event.button == NavButton::Prev)
                                         gState.dots7 = static_cast<uint8_t>((gState.dots7 == 0u) ? 6u : (gState.dots7 - 1u));
                                 }
                                 else if (current == ScreenDotsMany)
                                 {
                                     if (event.button == NavButton::Next)
                                         gState.dotsMany = static_cast<uint8_t>((gState.dotsMany + 1u) % 12u);
                                     else if (event.button == NavButton::Prev)
                                         gState.dotsMany = static_cast<uint8_t>((gState.dotsMany == 0u) ? 11u : (gState.dotsMany - 1u));
                                 }
                                 else if (current == ScreenToggleUp)
                                 {
                                     if (event.button == NavButton::Next)
                                         gState.toggleUser = !gState.toggleUser;
                                     else if (event.button == NavButton::Prev)
                                         gState.toggleAuto = !gState.toggleAuto;
                                 }
                                 else if (current == ScreenSliderUp)
                                 {
                                     if (event.button == NavButton::Next && gState.sliderUser < 100)
                                         gState.sliderUser = static_cast<int16_t>(gState.sliderUser + 4);
                                     else if (event.button == NavButton::Prev && gState.sliderUser > 0)
                                         gState.sliderUser = static_cast<int16_t>(gState.sliderUser - 4);
                                 }
                                 else if (current == ScreenButtonUp)
                                 {
                                     if (event.button == NavButton::Next)
                                     {
                                         gState.buttonDown = !gState.buttonDown;
                                         gState.buttonValue = static_cast<uint8_t>((gState.buttonValue + 13u) % 101u);
                                     }
                                     else if (event.button == NavButton::Prev)
                                     {
                                         gState.controlRow = static_cast<uint8_t>((gState.controlRow + 1u) % 3u);
                                     }
                                 }
                                 else if (current == ScreenProgressUp || current == ScreenCircleProgUp)
                                 {
                                     if (event.button == NavButton::Next)
                                         gState.progressUser = static_cast<uint8_t>((gState.progressUser + 5u) % 101u);
                                     else if (event.button == NavButton::Prev)
                                         gState.progressUser = static_cast<uint8_t>((gState.progressUser >= 5u) ? (gState.progressUser - 5u) : 100u);
                                 }
                                 else if (current == ScreenListPlain)
                                 {
                                     if (event.button == NavButton::Next)
                                         gState.plainChecked = static_cast<uint8_t>((gState.plainChecked + 1u) % 6u);
                                     else if (event.button == NavButton::Prev)
                                         gState.plainChecked = static_cast<uint8_t>((gState.plainChecked == 0u) ? 5u : (gState.plainChecked - 1u));
                                 }
                                 else if (current == ScreenDrumH)
                                 {
                                     if (event.button == NavButton::Next)
                                         gState.drumH = static_cast<uint8_t>((gState.drumH + 1u) % 6u);
                                     else if (event.button == NavButton::Prev)
                                         gState.drumH = static_cast<uint8_t>((gState.drumH == 0u) ? 5u : (gState.drumH - 1u));
                                 }
                                 else if (current == ScreenDrumV)
                                 {
                                     if (event.button == NavButton::Next)
                                         gState.drumV = static_cast<uint8_t>((gState.drumV + 1u) % 5u);
                                     else if (event.button == NavButton::Prev)
                                         gState.drumV = static_cast<uint8_t>((gState.drumV == 0u) ? 4u : (gState.drumV - 1u));
                                 }
                                 else
                                 {
                                     return false;
                                 }

                                 ui.requestRedraw();
                                 return true;
                             });
            break;
        case ScreenTileA:
        case ScreenTileB:
        case ScreenTileCustom:
            (void)ui.tileNav();
            break;
        case ScreenToast:
        case ScreenNotifA:
        case ScreenNotifB:
        case ScreenPopupA:
        case ScreenPopupB:
        case ScreenErrWarn:
        case ScreenErrCrash:
        case ScreenShotA:
        case ScreenShotB:
        case ScreenWifi:
        case ScreenOta:
            ui.nav().handler([](GUI &, const NavEvent &event, void *) -> bool
                             {
                                 if (event.button == NavButton::Prev && event.code == NavEventCode::LongPressed)
                                 {
                                     ui.backScreen();
                                     return true;
                                 }
                                 if (event.button != NavButton::Next || event.code != NavEventCode::Released || event.longPress)
                                     return false;

                                 const uint8_t current = ui.currentScreen();
                                 if (current == ScreenToast)
                                 {
                                     const char *texts[4] = {"Top saved", "Bottom warning", "Network ready", "Capture queued"};
                                     const IconId icons[4] = {checkmark, warning, arrow, checkmark};
                                     ui.showToast().text(texts[gState.overlayStep & 3u]).pos((gState.overlayStep & 1u) ? down : top).icon(icons[gState.overlayStep & 3u]);
                                     ++gState.overlayStep;
                                 }
                                 else if (current == ScreenNotifA)
                                 {
                                     const NotificationType type = (gState.overlayStep & 1u) ? NotificationType::Warning : NotificationType::Normal;
                                     ui.showNotification().text("Timed notice", "Auto closes after delay").button("OK").delay(3).type(type).icon((gState.overlayStep & 1u) ? warning : checkmark);
                                     ++gState.overlayStep;
                                 }
                                 else if (current == ScreenNotifB)
                                 {
                                     const NotificationType type = (gState.overlayStep & 1u) ? NotificationType::Normal : NotificationType::Error;
                                     ui.showNotification().text("Action required", "Button text/type/icon variant").button((gState.overlayStep & 1u) ? "Install" : "Retry").type(type).icon((gState.overlayStep & 1u) ? arrow : error);
                                     ++gState.overlayStep;
                                 }
                                 else if (current == ScreenPopupA || current == ScreenPopupB)
                                 {
                                     if (current == ScreenPopupA)
                                     {
                                         static const char *const items[] = {"Copy", "Paste", "Rename", "Delete"};
                                         auto anchor = ui.drawButton().label("Anchor").pos(center, 178).size(142, 34).baseColor(accent2(ui)).radius(11);
                                         ui.showPopupMenu().items(items).anchor(anchor).width(156).selected(gState.overlayStep & 3u);
                                     }
                                     else
                                     {
                                         static const char *const items[] = {"Stable", "Beta", "Factory", "Rollback", "Diagnostics", "Cancel"};
                                         auto anchor = ui.drawButton().label("Wide anchor").pos(22, 174).size(196, 34).baseColor(warn(ui)).radius(8);
                                         ui.showPopupMenu().items(items).anchor(anchor).width(198).selected(gState.overlayStep % 6u);
                                     }
                                     ++gState.overlayStep;
                                 }
                                 else if (current == ScreenErrWarn)
                                 {
                                     ui.showError().message("Warning path test").code("WARN_UI_001").button("Close").type(Warning);
                                 }
                                 else if (current == ScreenErrCrash)
                                 {
                                     ui.showError().message("Error path test").code("ERR_UI_001").button("Acknowledge").type(Crash);
                                 }
                                 else if (current == ScreenShotA || current == ScreenShotB)
                                 {
                                     const bool ok = ui.startScreenshot();
                                     ui.showToast().text(ok ? "Screenshot started" : "Screenshot unavailable").pos(top).icon(ok ? checkmark : warning);
                                     ui.requestRedraw();
                                 }
                                 else if (current == ScreenWifi)
                                 {
                                     gState.toggleUser = !gState.toggleUser;
                                     ui.requestWiFi(gState.toggleUser);
                                     ui.requestRedraw();
                                 }
                                 else if (current == ScreenOta)
                                 {
                                     const uint8_t step = gState.overlayStep++ % 4u;
                                     if (step == 0u)
                                         ui.otaRequestCheck();
                                     else if (step == 1u)
                                         ui.otaRequestCheck(AllowDowngrade);
                                     else if (step == 2u)
                                         ui.otaRequestStableList();
                                     else
                                         ui.otaCancel();
                                     ui.requestRedraw();
                                 }
                                 return true;
                             });
            break;
        default:
            ui.nav().handler([](GUI &, const NavEvent &event, void *) -> bool
                             {
                                 if (event.button == NavButton::Prev && event.code == NavEventCode::Released && !event.longPress)
                                 {
                                     ui.backScreen();
                                     return true;
                                 }
                                 return false;
                             });
            break;
        }

        ui.requestRedraw();
    }

    bool screenNeedsAnimation(uint8_t screenId) noexcept
    {
        switch (screenId)
        {
        case ScreenAdapt:
        case ScreenText:
        case ScreenMarquee:
        case ScreenAnimIcons:
        case ScreenBlurUp:
        case ScreenBlurDUp:
        case ScreenGlowUp:
        case ScreenButtonUp:
        case ScreenToggleUp:
        case ScreenSliderUp:
        case ScreenProgressUp:
        case ScreenCircleProgUp:
        case ScreenShotA:
        case ScreenShotB:
        case ScreenGraphUpdate:
        case ScreenGraphScope:
        case ScreenGraphLine:
        case ScreenWifi:
        case ScreenOta:
            return true;
        default:
            return false;
        }
    }


    void runBootAnimation()
    {
        const uint32_t startMs = millis();
        ui.showLogo()
            .text("PipGUI", "API verification deck")
            .anim(FadeIn);

        while ((millis() - startMs) < 1880u)
            ui.loop();
    }
}

GUI ui;

DemoState &runtimeState() noexcept
{
    return gState;
}

void demoSetup()
{
    Serial.begin(115200);
    gState.marqueePhaseStartMs = millis();
    loadRuntimePrefs();

#if PIPGUI_OTA
        ui.otaConfigure();
#endif

        ui.configDisplay()
            .pins({6, 5, 7, 8, -1})
            .size(320, 480);

        ui.begin(1, gState.forceTiles);
        configureStatusBar();
        updateStatusBarText(0);
        ui.setStatusBarBattery(gState.batteryLevel, Bar);
        if (kBacklightPin != 255)
            ui.setBacklight().pin(kBacklightPin);

        applyDebugRuntime();

        btnNext.begin();
        btnPrev.begin();
        if (kHasSelectButton)
            btnSelect.begin();

        runBootAnimation();

        ui.setScreenAnim(SlideY, 260);
    ui.setScreen(ScreenMenu);
    gState.lastScreen = ScreenMenu;
    onScreenEnter(ScreenMenu);
}

void demoLoop()
{
    (void)(kHasSelectButton ? ui.pollInput(btnNext, btnPrev, btnSelect)
                            : ui.pollInput(btnNext, btnPrev));
    const uint32_t nowMs = millis();

        if (ui.screenTransitionActive())
        {
            ui.loopWithPolledInput();
            return;
        }

        const uint8_t current = ui.currentScreen();
        if (current != gState.lastScreen)
        {
            onScreenLeave(gState.lastScreen);
            onScreenEnter(current);
            gState.lastScreen = current;
        }

        updateStatusBarText(nowMs);
        updateStatusBarBattery(nowMs);

        if (screenNeedsAnimation(current) && (nowMs - gState.lastAnimRedrawMs) >= 33)
        {
            gState.lastAnimRedrawMs = nowMs;
            ui.requestRedraw();
        }

        ui.loopWithPolledInput();
}

UiRect bodyRect(const GUI &gui) noexcept
{
    const int16_t top = 34;
    const int16_t bottom = 14;
    return UiRect{0, top, int16_t(gui.screenWidth()), int16_t(gui.screenHeight() - top - bottom)};
}

UiRect bodyInset(const GUI &gui, int16_t padX, int16_t padTop, int16_t padBottom) noexcept
{
    UiRect rect = bodyRect(gui);
    rect.x += padX;
    rect.y += padTop;
    rect.w -= padX * 2;
    rect.h -= padTop + padBottom;
    return rect;
}

UiRect splitLeft(const UiRect &rect, int16_t gap) noexcept
{
    const int16_t half = (rect.w - gap) / 2;
    return UiRect{rect.x, rect.y, half, rect.h};
}

UiRect splitRight(const UiRect &rect, int16_t gap) noexcept
{
    const int16_t half = (rect.w - gap) / 2;
    return UiRect{int16_t(rect.x + half + gap), rect.y, half, rect.h};
}

void drawHeader(GUI &gui, const char *title, const char *subtitle, uint16_t accent565)
{
    const uint16_t title565 = accent565 ? accent565 : fg(gui);

    gui.drawText()
        .font(WixMadeForDisplay, 16)
        .weight(Semibold)
        .text(title)
        .pos(12, 8)
        .color(title565)
        .bgColor(bg(gui));

    gui.drawText()
        .font(WixMadeForDisplay, 10)
        .weight(Medium)
        .text(subtitle)
        .pos(12, 22)
        .color(muted(gui))
        .bgColor(bg(gui));
}

void drawFooter(GUI &gui, const char *hint)
{
    const int16_t y = gui.screenHeight() - 10;

    gui.drawText()
        .font(WixMadeForDisplay, 10)
        .weight(Medium)
        .text(hint)
        .pos(center, y)
        .color(muted(gui))
        .bgColor(bg(gui))
        .align(Center);
}

void drawPanel(GUI &gui, int16_t x, int16_t y, int16_t w, int16_t h,
               uint16_t fill565, uint16_t border565, uint8_t radius)
{
    gui.drawRect()
        .pos(x, y)
        .size(w, h)
        .radius(radius)
        .fill(fill565)
        .border(1, border565);
}

uint16_t bg(GUI &gui) noexcept { return gui.rgb(5, 7, 10); }
uint16_t panel(GUI &gui) noexcept { return gui.rgb(15, 20, 26); }
uint16_t panelAlt(GUI &gui) noexcept { return gui.rgb(24, 31, 38); }
uint16_t fg(GUI &gui) noexcept { return gui.rgb(244, 247, 250); }
uint16_t muted(GUI &gui) noexcept { return gui.rgb(136, 147, 161); }
uint16_t line(GUI &gui) noexcept { return gui.rgb(52, 62, 74); }
uint16_t accent(GUI &gui) noexcept { return gui.rgb(84, 208, 140); }
uint16_t accent2(GUI &gui) noexcept { return gui.rgb(96, 174, 255); }
uint16_t success(GUI &gui) noexcept { return gui.rgb(72, 232, 138); }
uint16_t warn(GUI &gui) noexcept { return gui.rgb(255, 184, 68); }
uint16_t danger(GUI &gui) noexcept { return gui.rgb(255, 92, 92); }

const char *screenshotModeLabel(uint8_t mode) noexcept
{
    switch (mode)
    {
    case 0:
        return "Off";
    case 1:
        return "Serial";
    case 2:
        return "LittleFS";
    default:
        return "Unknown";
    }
}
