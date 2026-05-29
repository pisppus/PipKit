SCREEN(light, ScreenLight)
{
    const DemoState &st = runtimeState();
    const uint16_t bg565 = bg(ui);
    const auto body = bodyInset(ui, 12, 10, 8);

    ui.clear(bg565);
    drawHeader(ui, "Backlight", "NEXT changes value, PREV selects row, PREV hold backs", accent(ui));
    drawPanel(ui, body.x, body.y, body.w, body.h, panel(ui), line(ui), 14);

    const uint8_t rows[2] = {st.brightness, st.maxBrightness};
    const char *labels[2] = {"Brightness", "Max brightness"};
    for (uint8_t i = 0; i < 2; ++i)
    {
        const int16_t y = body.y + 34 + i * 62;
        const bool active = (st.lightRow == i);
        ui.drawText().font(WixMadeForDisplay, 13).weight(Semibold).text(labels[i]).pos(body.x + 14, y).color(active ? accent(ui) : fg(ui)).bgColor(panel(ui));
        ui.drawProgress().pos(body.x + 14, y + 24).size(body.w - 28, 14).value(rows[i]).baseColor(panelAlt(ui)).fillColor(active ? accent(ui) : accent2(ui)).radius(7).percent(Right).percentColor(fg(ui)).percentFont(10);
    }

    ui.drawText().font(WixMadeForDisplay, 11).weight(Medium).text(String("Actual: ") + ui.brightness() + "% / cap " + ui.maxBrightness() + "%").pos(center, body.y + body.h - 36).color(muted(ui)).bgColor(panel(ui)).align(Center);
    drawFooter(ui, "Backlight pin is compile-time optional");
}
