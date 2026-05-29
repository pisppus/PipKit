SCREEN(rotation, ScreenRotate)
{
    const uint16_t bg565 = bg(ui);
    const uint16_t panel565 = panel(ui);
    const uint16_t panelAlt565 = panelAlt(ui);
    const uint16_t text565 = fg(ui);
    const uint16_t muted565 = muted(ui);
    const uint16_t accent565 = accent2(ui);
    const auto body = bodyInset(ui, 10, 8, 8);
    auto &st = runtimeState();

    struct RotationRow
    {
        const char *title;
        const char *subtitle;
        uint8_t rotation;
        bool isAction;
    };

    const RotationRow rows[5] = {
        {"Rotation 0", "Logical canvas aligned to physical orientation", 0, true},
        {"Rotation 90", "Quarter turn clockwise via runtime transform", 1, true},
        {"Rotation 180", "Upside-down logical orientation", 2, true},
        {"Rotation 270", "Quarter turn counter-clockwise", 3, true},
        {"Slow animation", "1200 ms instead of the default 520 ms", 0, false},
    };

    drawHeader(ui, "Runtime rotation", "setRotation(rotation, durationMs) with live state and mode coverage", accent565);

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

    const uint8_t currentRotation = ui.screenRotation() & 3u;
    const int16_t rowH = 34;
    for (uint8_t i = 0; i < 5; ++i)
    {
        const int16_t y = body.y + i * (rowH + 6);
        const uint16_t fill565 = (st.rotationRow == i) ? panelAlt565 : panel565;
        const uint16_t border565 = (st.rotationRow == i) ? accent565 : line(ui);
        drawPanel(ui, body.x, y, body.w, rowH, fill565, border565, 10);

        labelBase.derive()
            .pos(body.x + 12, y + 8)
            .bgColor(fill565)
            .text(rows[i].title);

        subtitleBase.derive()
            .pos(body.x + 12, y + 20)
            .bgColor(fill565)
            .text(rows[i].subtitle);

        bool value = rows[i].isAction ? (currentRotation == rows[i].rotation) : st.rotationSlowAnim;
        ui.updateToggleSwitch()
            .pos(body.x + body.w - 54, y + 6)
            .size(40, 22)
            .value(value)
            .enabled(false)
            .activeColor(accent565)
            .inactiveColor(line(ui))
            .knobColor(text565);
    }

    const int16_t infoY = body.y + 5 * (rowH + 6);
    const uint16_t infoBg = panel565;
    drawPanel(ui, body.x, infoY, body.w, 78, infoBg, line(ui), 10);

    ui.drawText()
        .font(WixMadeForDisplay, 11)
        .weight(Semibold)
        .text(String("Current rotation: ") + currentRotation * 90 + " deg")
        .pos(body.x + 12, infoY + 10)
        .color(text565)
        .bgColor(infoBg);

    ui.drawText()
        .font(WixMadeForDisplay, 10)
        .weight(Medium)
        .text(ui.tiledMode() ? "Tiled mode: streaming rotation, no full framebuffer" : "Fullscreen sprite: runtime rotation animates")
        .pos(body.x + 12, infoY + 26)
        .color(muted565)
        .bgColor(infoBg);

    ui.drawText()
        .font(WixMadeForDisplay, 10)
        .weight(Medium)
        .text(ui.rotationTransitionActive() ? "Transition active" : "Transition idle")
        .pos(body.x + 12, infoY + 42)
        .color(ui.rotationTransitionActive() ? accent(ui) : muted565)
        .bgColor(infoBg);

    drawFooter(ui, "NEXT/PREV move  ENTER apply  PREV hold back");
}
