#pragma once

#include <Arduino.h>
#include <PipKit.hpp>
#include <PipGUI/Core/Debug/Debug.hpp>
#include <PipGUI/Core/Config/Version.hpp>
#include <PipGUI/Systems/Network/Wifi.hpp>
#include <PipGUI/Systems/Update/Ota.hpp>

#include "screens/ids.hpp"

extern pipgui::GUI ui;

struct DemoState
{
    bool forceTiles = true;
    bool debugLayoutBounds = (PIPGUI_DEBUG_LAYOUT_BOUNDS != 0);
    bool debugOverdraw = (PIPGUI_DEBUG_OVERDRAW != 0);
    bool debugMetrics = (PIPGUI_DEBUG_METRICS != 0);
    bool normalStatusBar = true;
    bool rotationSlowAnim = false;
    uint8_t screenshotModeChoice = (PIPGUI_SCREENSHOT_MODE <= 2) ? PIPGUI_SCREENSHOT_MODE : 2;
    uint8_t debugRow = 0;
    uint8_t rotationRow = 0;
    uint8_t controlRow = 0;
    uint8_t lightRow = 0;
    uint8_t brightness = 80;
    uint8_t maxBrightness = 100;
    uint8_t dots7 = 0;
    uint8_t dotsMany = 0;
    uint8_t drumH = 2;
    uint8_t drumV = 2;
    bool toggleUser = false;
    bool toggleAuto = false;
    int16_t sliderUser = 52;
    uint8_t progressUser = 42;
    uint8_t buttonValue = 20;
    bool buttonDown = false;
    uint8_t plainChecked = 2;
    uint8_t overlayStep = 0;
    uint8_t batteryLevel = 100;
    bool batteryDown = true;
    uint32_t marqueePhaseStartMs = 0;
    uint8_t lastScreen = pipgui::INVALID_SCREEN_ID;
    uint32_t lastBatteryMs = 0;
    uint32_t lastClockMinute = 0xFFFFFFFFu;
    uint32_t lastAnimRedrawMs = 0;
};

DemoState &runtimeState() noexcept;

void demoSetup();
void demoLoop();

[[nodiscard]] pipgui::UiRect bodyRect(const pipgui::GUI &ui) noexcept;
[[nodiscard]] pipgui::UiRect bodyInset(const pipgui::GUI &ui,
                                       int16_t padX = 12,
                                       int16_t padTop = 10,
                                       int16_t padBottom = 10) noexcept;
[[nodiscard]] pipgui::UiRect splitLeft(const pipgui::UiRect &rect, int16_t gap = 8) noexcept;
[[nodiscard]] pipgui::UiRect splitRight(const pipgui::UiRect &rect, int16_t gap = 8) noexcept;

void drawHeader(pipgui::GUI &ui, const char *title, const char *subtitle, uint16_t accent565);
void drawFooter(pipgui::GUI &ui, const char *hint);
void drawPanel(pipgui::GUI &ui, int16_t x, int16_t y, int16_t w, int16_t h,
               uint16_t fill565, uint16_t border565, uint8_t radius = 12);

[[nodiscard]] uint16_t bg(pipgui::GUI &ui) noexcept;
[[nodiscard]] uint16_t panel(pipgui::GUI &ui) noexcept;
[[nodiscard]] uint16_t panelAlt(pipgui::GUI &ui) noexcept;
[[nodiscard]] uint16_t fg(pipgui::GUI &ui) noexcept;
[[nodiscard]] uint16_t muted(pipgui::GUI &ui) noexcept;
[[nodiscard]] uint16_t line(pipgui::GUI &ui) noexcept;
[[nodiscard]] uint16_t accent(pipgui::GUI &ui) noexcept;
[[nodiscard]] uint16_t accent2(pipgui::GUI &ui) noexcept;
[[nodiscard]] uint16_t success(pipgui::GUI &ui) noexcept;
[[nodiscard]] uint16_t warn(pipgui::GUI &ui) noexcept;
[[nodiscard]] uint16_t danger(pipgui::GUI &ui) noexcept;

[[nodiscard]] const char *screenshotModeLabel(uint8_t mode) noexcept;
