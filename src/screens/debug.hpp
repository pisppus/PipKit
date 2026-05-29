SCREEN(debug, ScreenDebug)
{
    const uint16_t bg565 = bg(ui);
    const uint16_t panel565 = panel(ui);
    const uint16_t panelAlt565 = panelAlt(ui);
    const uint16_t text565 = fg(ui);
    const uint16_t muted565 = muted(ui);
    const uint16_t accent565 = accent(ui);
    const auto body = bodyInset(ui, 10, 8, 8);
    auto &st = runtimeState();

    struct ToggleRow
    {
        const char *title;
        const char *subtitle;
        bool *value;
    };

    const ToggleRow rows[5] = {
        {"Tile mode", "Re-begin display with tiled sprite fallback", &st.forceTiles},
        {"Layout bounds", "Runtime overlay for layout boxes", &st.debugLayoutBounds},
        {"Overdraw", "Paint-count heatmap overlay", &st.debugOverdraw},
        {"Metrics", "Status-bar heap metrics via Debug::formatStatusBar", &st.debugMetrics},
        {"Status bar", "Ordinary icon slots when metrics overlay is off", &st.normalStatusBar},
    };

    drawHeader(ui, "Runtime", "Render mode, debug overlays and screenshot backend", accent565);

    auto labelBase = ui.drawText()
                         .font(WixMadeForDisplay, 13)
                         .weight(Semibold)
                         .color(text565)
                         .bgColor(panel565);
    auto subtitleBase = ui.drawText()
                            .font(WixMadeForDisplay, 10)
                            .weight(Medium)
                            .color(muted565)
                            .bgColor(panel565);

    const int16_t rowH = 30;
    for (uint8_t i = 0; i < 5; ++i)
    {
        const int16_t y = body.y + i * (rowH + 4);
        const uint16_t fill565 = (st.debugRow == i) ? panelAlt565 : panel565;
        const uint16_t border565 = (st.debugRow == i) ? accent565 : line(ui);
        drawPanel(ui, body.x, y, body.w, rowH, fill565, border565, 10);

        labelBase.derive()
            .pos(body.x + 12, y + 8)
            .bgColor(fill565)
            .text(rows[i].title);

        subtitleBase.derive()
            .pos(body.x + 12, y + 20)
            .bgColor(fill565)
            .text(rows[i].subtitle);

        ui.updateToggleSwitch()
            .pos(body.x + body.w - 54, y + 6)
            .size(40, 22)
            .value(*rows[i].value)
            .enabled(false)
            .activeColor(accent565)
            .inactiveColor(line(ui))
            .knobColor(text565);
    }

    const int16_t shotY = body.y + 5 * (rowH + 4);
    const uint16_t shotBg = (st.debugRow == 5) ? panelAlt565 : panel565;
    drawPanel(ui, body.x, shotY, body.w, 74, shotBg, (st.debugRow == 5) ? accent565 : line(ui), 10);

    labelBase.derive()
        .pos(body.x + 12, shotY + 8)
        .bgColor(shotBg)
        .text("Screenshot mode");

    subtitleBase.derive()
        .pos(body.x + 12, shotY + 20)
        .bgColor(shotBg)
        .text("Choice is demo-local; compiled backend stays fixed");

    ui.drawDrumRoll()
        .pos(body.x + 10, shotY + 36)
        .size(body.w - 20, 24)
        .options(12, "Off", "Serial", "LittleFS")
        .selected(st.screenshotModeChoice)
        .fgColor(text565)
        .bgColor(shotBg);

    ui.drawText()
        .font(WixMadeForDisplay, 10)
        .weight(Medium)
        .text(String("Compiled: ") + screenshotModeLabel(PIPGUI_SCREENSHOT_MODE))
        .pos(body.x + body.w - 12, shotY + 62)
        .color(muted565)
        .bgColor(shotBg)
        .align(Right);
}
