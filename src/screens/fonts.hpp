SCREEN(fonts, ScreenFonts)
{
    const uint16_t bg565 = bg(ui);
    const uint16_t panel565 = panel(ui);
    const uint16_t text565 = fg(ui);
    const uint16_t muted565 = muted(ui);
    const auto body = bodyInset(ui, 12, 10, 8);
    const auto left = splitLeft(body, 10);
    const auto right = splitRight(body, 10);
    const uint16_t sizes[4] = {12, 14, 16, 18};

    ui.clear(bg565);
    drawHeader(ui, "Fonts", "WixMadeForDisplay and KronaOne across four sizes", accent(ui));
    drawPanel(ui, left.x, left.y, left.w, left.h, panel565, line(ui), 12);
    drawPanel(ui, right.x, right.y, right.w, right.h, panel565, line(ui), 12);

    ui.drawText()
        .font(WixMadeForDisplay, 14)
        .weight(Semibold)
        .text("WixMadeForDisplay")
        .pos(left.x + left.w / 2, left.y + 12)
        .color(text565)
        .bgColor(panel565)
        .align(Center);

    ui.drawText()
        .font(WixMadeForDisplay, 14)
        .weight(Semibold)
        .text("KronaOne")
        .pos(right.x + right.w / 2, right.y + 12)
        .color(text565)
        .bgColor(panel565)
        .align(Center);

    auto label = ui.drawText()
                     .font(WixMadeForDisplay, 10)
                     .weight(Medium)
                     .color(muted565)
                     .bgColor(panel565);
    auto sample = ui.drawText()
                      .color(text565)
                      .bgColor(panel565);

    for (uint8_t i = 0; i < 4; ++i)
    {
        const int16_t y = left.y + 34 + i * 48;
        label.derive()
            .text(String(sizes[i]) + " px")
            .pos(left.x + 12, y);
        sample.derive()
            .font(WixMadeForDisplay, sizes[i])
            .weight(Medium)
            .text("Sample Aa 123")
            .pos(left.x + 12, y + 12);

        label.derive()
            .text(String(sizes[i]) + " px")
            .pos(right.x + 12, y);
        sample.derive()
            .font(KronaOne, sizes[i])
            .weight(Regular)
            .text("Sample Aa 123")
            .pos(right.x + 12, y + 12);
    }

    drawFooter(ui, "PREV back");
}
