SCREEN(styles, ScreenStyles)
{
    const uint16_t bg565 = bg(ui);
    const uint16_t panel565 = panel(ui);
    const uint16_t text565 = fg(ui);
    const uint16_t muted565 = muted(ui);
    const auto body = bodyInset(ui, 12, 10, 8);
    const int16_t cardH = 44;

    struct StyleRow
    {
        const char *label;
        TextStyle style;
        const char *sample;
    };

    const StyleRow rows[4] = {
        {"H1", H1, "Heading one"},
        {"H2", H2, "Heading two"},
        {"Body", Body, "Body copy for default reading"},
        {"Caption", Caption, "Caption / metadata line"},
    };

    ui.clear(bg565);
    drawHeader(ui, "Styles", "Built-in text styles and their visual rhythm", accent(ui));

    for (uint8_t i = 0; i < 4; ++i)
    {
        const int16_t y = body.y + i * (cardH + 8);
        drawPanel(ui, body.x, y, body.w, cardH, panel565, line(ui), 12);

        ui.drawText()
            .font(WixMadeForDisplay, 10)
            .weight(Semibold)
            .text(rows[i].label)
            .pos(body.x + 12, y + 6)
            .color(muted565)
            .bgColor(panel565);

        ui.setTextStyle(rows[i].style);
        ui.drawText()
            .text(rows[i].sample)
            .pos(body.x + 12, y + 18)
            .color(text565)
            .bgColor(panel565);
    }

    drawFooter(ui, "PREV back");
}
