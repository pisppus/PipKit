SCREEN(blurup, ScreenBlurUp)
{
    const uint16_t bg565 = bg(ui);
    const auto body = bodyInset(ui, 12, 10, 8);
    const float t = static_cast<float>(millis() % 5000u) * 0.004f;

    ui.clear(bg565);
    drawHeader(ui, "Blur update", "updateBlur() over moving icon/shape/text", accent(ui));

    for (uint8_t i = 0; i < 4; ++i)
    {
        const int16_t x = body.x + 20 + static_cast<int16_t>((sinf(t + i * 1.2f) + 1.0f) * 82.0f);
        const int16_t y = body.y + 36 + i * 38;
        ui.drawCircle().pos(x, y).radius(16 + i * 2).fill((i & 1u) ? accent2(ui) : warn(ui));
    }
    ui.drawIcon().pos(body.x + 44, body.y + 128).size(42).icon(arrow).color(fg(ui)).bgColor(bg565);
    ui.drawText().font(WixMadeForDisplay, 18).weight(Bold).text("MOVE").pos(body.x + 170, body.y + 170).color(fg(ui)).bgColor(bg565).align(Center);

    ui.updateBlur().pos(body.x + 12, body.y + 34).size(86, 46).radius(8).material(88, -1);
    ui.updateBlur().pos(body.x + 118, body.y + 64).size(92, 54).radius(18).material(150, ui.rgb(24, 36, 48));
    ui.updateBlur().pos(body.x + 34, body.y + 142).size(166, 42).radius(12).material(120, -1);
    drawFooter(ui, "Animated redraw every 33 ms");
}
