SCREEN(weight, ScreenWeight)
{
    const uint16_t bg565 = bg(ui);
    const uint16_t panel565 = panel(ui);
    const uint16_t text565 = fg(ui);
    const uint16_t muted565 = muted(ui);
    const auto body = bodyInset(ui, 12, 10, 8);
    const auto left = splitLeft(body, 10);
    const auto right = splitRight(body, 10);

    struct WeightRow
    {
        const char *name;
        uint16_t weight;
    };

    const WeightRow rows[4] = {
        {"Regular", Regular},
        {"Medium", Medium},
        {"Semibold", Semibold},
        {"Black", Black},
    };

    ui.clear(bg565);
    drawHeader(ui, "Weight", "Same size, different weights for both fonts", accent2(ui));
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

    for (uint8_t i = 0; i < 4; ++i)
    {
        const int16_t y = left.y + 40 + i * 44;
        ui.drawText()
            .font(WixMadeForDisplay, 10)
            .weight(Medium)
            .text(rows[i].name)
            .pos(left.x + 10, y)
            .color(muted565)
            .bgColor(panel565);

        ui.drawText()
            .font(WixMadeForDisplay, 18)
            .weight(rows[i].weight)
            .text("Sample Aa 123")
            .pos(left.x + 10, y + 12)
            .color(text565)
            .bgColor(panel565);

        ui.drawText()
            .font(WixMadeForDisplay, 10)
            .weight(Medium)
            .text(rows[i].name)
            .pos(right.x + 10, y)
            .color(muted565)
            .bgColor(panel565);

        ui.drawText()
            .font(KronaOne, 18)
            .weight(rows[i].weight)
            .text("Sample Aa 123")
            .pos(right.x + 10, y + 12)
            .color(text565)
            .bgColor(panel565);
    }

    drawFooter(ui, "PREV back");
}
