SCREEN(lines, ScreenLines)
{
    const uint16_t bg565 = bg(ui);
    const uint16_t panel565 = panel(ui);
    const uint16_t text565 = fg(ui);
    const uint16_t muted565 = muted(ui);
    const auto body = bodyInset(ui, 12, 10, 8);

    ui.clear(bg565);
    drawHeader(ui, "Lines", "drawLine() with different spans, angles and thicknesses", accent(ui));
    drawPanel(ui, body.x, body.y, body.w, body.h, panel565, line(ui), 12);

    ui.drawText()
        .font(WixMadeForDisplay, 10)
        .weight(Medium)
        .text("Horizontal thickness")
        .pos(body.x + 12, body.y + 10)
        .color(muted565)
        .bgColor(panel565);

    for (uint8_t i = 0; i < 4; ++i)
    {
        const int16_t y = body.y + 28 + i * 14;

        ui.drawLine()
            .from(body.x + 14, y)
            .to(body.x + 104, y)
            .thickness(static_cast<uint8_t>(i + 1))
            .color((i & 1u) ? accent(ui) : accent2(ui));
    }

    ui.drawText()
        .font(WixMadeForDisplay, 10)
        .weight(Medium)
        .text("Diagonals")
        .pos(body.x + 132, body.y + 10)
        .color(muted565)
        .bgColor(panel565);

    ui.drawLine()
        .from(body.x + 128, body.y + 28)
        .to(body.x + 206, body.y + 74)
        .thickness(2)
        .color(warn(ui));

    ui.drawLine()
        .from(body.x + 206, body.y + 28)
        .to(body.x + 128, body.y + 74)
        .thickness(3)
        .color(danger(ui));

    ui.drawText()
        .font(WixMadeForDisplay, 10)
        .weight(Medium)
        .text("Vertical / short")
        .pos(body.x + 12, body.y + 94)
        .color(muted565)
        .bgColor(panel565);

    ui.drawLine()
        .from(body.x + 30, body.y + 112)
        .to(body.x + 30, body.y + 176)
        .thickness(1)
        .color(text565);

    ui.drawLine()
        .from(body.x + 52, body.y + 118)
        .to(body.x + 52, body.y + 176)
        .thickness(2)
        .color(accent(ui));

    ui.drawLine()
        .from(body.x + 78, body.y + 128)
        .to(body.x + 78, body.y + 176)
        .thickness(4)
        .color(accent2(ui));

    ui.drawText()
        .font(WixMadeForDisplay, 10)
        .weight(Medium)
        .text("Cross / fan")
        .pos(body.x + 120, body.y + 94)
        .color(muted565)
        .bgColor(panel565);

    for (uint8_t i = 0; i < 5; ++i)
    {
        const int16_t x1 = body.x + 124 + i * 20;

        ui.drawLine()
            .from(body.x + 168, body.y + 180)
            .to(x1, body.y + 118)
            .thickness(static_cast<uint8_t>((i % 3) + 1))
            .color((i & 1u) ? warn(ui) : success(ui));
    }

    ui.drawLine()
        .from(body.x + 18, body.y + body.h - 30)
        .to(body.x + body.w - 18, body.y + body.h - 30)
        .thickness(6)
        .color(line(ui));

    ui.drawLine()
        .from(body.x + 18, body.y + body.h - 30)
        .to(body.x + body.w - 18, body.y + body.h - 54)
        .thickness(2)
        .color(text565);

    drawFooter(ui, "PREV back");
}
