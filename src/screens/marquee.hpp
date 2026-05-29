SCREEN(marquee, ScreenMarquee)
{
    const uint16_t bg565 = bg(ui);
    const uint16_t panel565 = panel(ui);
    const uint16_t alt565 = panelAlt(ui);
    const uint16_t text565 = fg(ui);
    const uint16_t muted565 = muted(ui);
    const auto body = bodyInset(ui, 12, 10, 8);
    const uint32_t nowMs = millis();

    ui.clear(bg565);
    drawHeader(ui, "Marquee", "Marquee and ellipsized variants, static and dynamic", accent2(ui));

    const int16_t rowH = 46;
    for (uint8_t i = 0; i < 4; ++i)
    {
        const int16_t y = body.y + i * (rowH + 8);
        const uint16_t fill565 = (i & 1u) ? alt565 : panel565;
        drawPanel(ui, body.x, y, body.w, rowH, fill565, line(ui), 12);
    }

    ui.drawText()
        .font(WixMadeForDisplay, 10)
        .weight(Semibold)
        .text("Static marquee / speed 28")
        .pos(body.x + 12, body.y + 8)
        .color(muted565)
        .bgColor(panel565);

    ui.drawTextMarquee()
        .font(WixMadeForDisplay, 14)
        .weight(Semibold)
        .text("This marquee keeps sliding inside a narrow lane.")
        .pos(body.x + 12, body.y + 24)
        .width(body.w - 24)
        .color(text565)
        .align(Left)
        .speed(28)
        .holdStart(400)
        .phaseStart(0);

    const int16_t row2Y = body.y + rowH + 8;
    ui.drawText()
        .font(WixMadeForDisplay, 10)
        .weight(Semibold)
        .text("Static ellipsized / center")
        .pos(body.x + 12, row2Y + 8)
        .color(muted565)
        .bgColor(alt565);

    ui.drawTextEllipsized()
        .font(WixMadeForDisplay, 14)
        .weight(Semibold)
        .text("This ellipsized string is intentionally longer than the slot width.")
        .pos(body.x + body.w / 2, row2Y + 24)
        .width(body.w - 24)
        .color(text565)
        .align(Center);

    const int16_t row3Y = row2Y + rowH + 8;
    ui.drawText()
        .font(WixMadeForDisplay, 10)
        .weight(Semibold)
        .text("Dynamic marquee / moving counter")
        .pos(body.x + 12, row3Y + 8)
        .color(muted565)
        .bgColor(panel565);

    char frameBuf[96];
    const unsigned long seconds = nowMs / 1000u;
    snprintf(frameBuf, sizeof(frameBuf), "Dynamic status  %06lu s  marquee remains phase-stable",
             seconds);

    ui.drawTextMarquee()
        .font(WixMadeForDisplay, 14)
        .weight(Medium)
        .text(String(frameBuf))
        .pos(body.x + 12, row3Y + 24)
        .width(body.w - 24)
        .color(accent(ui))
        .align(Left)
        .speed(34)
        .holdStart(120)
        .phaseStart(runtimeState().marqueePhaseStartMs);

    const int16_t row4Y = row3Y + rowH + 8;
    ui.drawText()
        .font(WixMadeForDisplay, 10)
        .weight(Semibold)
        .text("Dynamic ellipsized / right")
        .pos(body.x + 12, row4Y + 8)
        .color(muted565)
        .bgColor(alt565);

    ui.drawTextEllipsized()
        .font(WixMadeForDisplay, 14)
        .weight(Medium)
        .text(String("Runtime mode ") + screenshotModeLabel(runtimeState().screenshotModeChoice) + " / second " + String(nowMs / 1000u))
        .pos(body.x + body.w - 12, row4Y + 24)
        .width(body.w - 24)
        .color(accent2(ui))
        .align(Right);

    drawFooter(ui, "PREV back");
}
