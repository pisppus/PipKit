SCREEN(text, ScreenText)
{
    const uint16_t bg565 = bg(ui);
    const uint16_t panel565 = panel(ui);
    const uint16_t alt565 = panelAlt(ui);
    const uint16_t text565 = fg(ui);
    const uint16_t muted565 = muted(ui);
    const uint32_t nowMs = millis();
    const auto body = bodyInset(ui, 12, 10, 8);
    const auto left = splitLeft(body, 10);
    const auto right = splitRight(body, 10);
    const uint16_t cycle = static_cast<uint16_t>((nowMs / 125u) % 800u);

    ui.clear(bg565);
    drawHeader(ui, "Text", "Static drawText() on the left, updateText() on the right", accent2(ui));
    drawPanel(ui, left.x, left.y, left.w, left.h, panel565, line(ui), 12);
    drawPanel(ui, right.x, right.y, right.w, right.h, alt565, line(ui), 12);

    auto title = ui.drawText()
                     .font(WixMadeForDisplay, 12)
                     .weight(Semibold)
                     .color(text565);
    auto note = ui.drawText()
                    .font(WixMadeForDisplay, 10)
                    .weight(Medium)
                    .color(muted565);
    auto sample = ui.drawText()
                      .font(WixMadeForDisplay, 13)
                      .weight(Medium)
                      .color(text565);
    auto dynamic = ui.updateText()
                       .font(WixMadeForDisplay, 13)
                       .weight(Medium)
                       .color(text565);

    title.derive()
        .text("drawText()")
        .pos(left.x + left.w / 2, left.y + 10)
        .color(text565)
        .bgColor(panel565)
        .align(Center);

    note.derive()
        .text("Fixed positions and explicit anchors")
        .pos(left.x + left.w / 2, left.y + 22)
        .color(muted565)
        .bgColor(panel565)
        .align(Center);

    sample.derive()
        .text("Left anchor")
        .pos(left.x + 10, left.y + 52)
        .color(text565)
        .bgColor(panel565);

    sample.derive()
        .text("Center anchor")
        .pos(left.x + left.w / 2, left.y + 84)
        .color(text565)
        .bgColor(panel565)
        .align(Center);

    sample.derive()
        .text("Object center")
        .in(left)
        .pos(center, left.h - 50)
        .color(text565)
        .bgColor(panel565)
        .align(Center);

    ui.drawText()
        .font(KronaOne, 18)
        .weight(Regular)
        .text("PSDF")
        .pos(left.x + left.w / 2, left.y + left.h - 26)
        .color(accent(ui))
        .bgColor(panel565)
        .align(Center);

    title.derive()
        .text("updateText()")
        .pos(right.x + right.w / 2, right.y + 10)
        .color(text565)
        .bgColor(alt565)
        .align(Center);

    note.derive()
        .text("Same slots, changing values")
        .pos(right.x + right.w / 2, right.y + 22)
        .color(muted565)
        .bgColor(alt565)
        .align(Center);

    dynamic.derive()
        .text(String("Tick: ") + cycle)
        .pos(right.x + 10, right.y + 52)
        .color(text565)
        .bgColor(alt565);
    dynamic.derive()
        .text(String("Seconds: ") + String(nowMs / 1000u))
        .pos(right.x + right.w / 2, right.y + 86)
        .color(text565)
        .bgColor(alt565)
        .align(Center);
    dynamic.derive()
        .text((cycle & 0x40u) ? "Center pulse" : "Center calm")
        .in(right)
        .pos(center, right.h - 50)
        .color(accent2(ui))
        .bgColor(alt565)
        .align(Center);

    ui.updateText()
        .font(KronaOne, 18)
        .weight(Regular)
        .text(String("0x") + String(cycle, HEX))
        .pos(right.x + right.w / 2, right.y + right.h - 26)
        .color(accent(ui))
        .bgColor(alt565)
        .align(Center);

    drawFooter(ui, "PREV back");
}
