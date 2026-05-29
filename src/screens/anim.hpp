SCREEN(anim, ScreenAnimIcons)
{
    const uint16_t bg565 = bg(ui);
    const uint16_t panel565 = panel(ui);
    const uint16_t alt565 = panelAlt(ui);
    const uint16_t text565 = fg(ui);
    const uint16_t muted565 = muted(ui);
    const auto body = bodyInset(ui, 12, 10, 8);
    const auto left = splitLeft(body, 10);
    const auto right = splitRight(body, 10);

    ui.clear(bg565);
    drawHeader(ui, "Anim icon", "drawAnimIcon() versus updateAnimIcon()", accent2(ui));
    drawPanel(ui, left.x, left.y, left.w, left.h, panel565, line(ui), 12);
    drawPanel(ui, right.x, right.y, right.w, right.h, alt565, line(ui), 12);

    ui.drawText()
        .font(WixMadeForDisplay, 12)
        .weight(Semibold)
        .text("drawAnimIcon()")
        .pos(left.x + left.w / 2, left.y + 10)
        .color(text565)
        .bgColor(panel565)
        .align(Center);

    ui.drawText()
        .font(WixMadeForDisplay, 12)
        .weight(Semibold)
        .text("updateAnimIcon()")
        .pos(right.x + right.w / 2, right.y + 10)
        .color(text565)
        .bgColor(alt565)
        .align(Center);

    const uint16_t sizes[3] = {28, 42, 60};
    const uint16_t colors[3] = {accent(ui), accent2(ui), warn(ui)};

    for (uint8_t i = 0; i < 3; ++i)
    {
        const int16_t y = left.y + 42 + i * 56;

        ui.drawAnimIcon()
            .pos(left.x + 16, y)
            .size(sizes[i])
            .icon(setting_anim)
            .color(colors[i]);

        ui.drawText()
            .font(WixMadeForDisplay, 10)
            .weight(Medium)
            .text(String(sizes[i]) + " px")
            .pos(left.x + 60, y + 8)
            .color(muted565)
            .bgColor(panel565);

        ui.updateAnimIcon()
            .pos(right.x + 16, y)
            .size(sizes[i])
            .icon(setting_anim)
            .color(colors[i])
            .bgColor(alt565);

        ui.drawText()
            .font(WixMadeForDisplay, 10)
            .weight(Medium)
            .text(String(sizes[i]) + " px")
            .pos(right.x + 60, y + 8)
            .color(muted565)
            .bgColor(alt565);
    }

    drawFooter(ui, "PREV back");
}
