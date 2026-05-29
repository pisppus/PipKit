SCREEN(iconGrid, ScreenIcons)
{
    const uint16_t bg565 = bg(ui);
    const uint16_t panel565 = panel(ui);
    const uint16_t text565 = fg(ui);
    const uint16_t muted565 = muted(ui);
    const auto body = bodyInset(ui, 12, 10, 8);

    ui.clear(bg565);
    drawHeader(ui, "Icons", "All icon glyphs, plus layered battery composition", accent(ui));
    drawPanel(ui, body.x, body.y, body.w, body.h, panel565, line(ui), 12);

    ui.drawIcon()
        .pos(body.x + 22, body.y + 28)
        .size(26)
        .icon(arrow)
        .color(accent2(ui))
        .bgColor(panel565);

    ui.drawText()
        .font(WixMadeForDisplay, 10)
        .weight(Medium)
        .text("arrow")
        .pos(body.x + 20, body.y + 60)
        .color(muted565)
        .bgColor(panel565);

    ui.drawIcon()
        .pos(body.x + 88, body.y + 28)
        .size(24)
        .icon(checkmark)
        .color(success(ui))
        .bgColor(panel565);

    ui.drawText()
        .font(WixMadeForDisplay, 10)
        .weight(Medium)
        .text("checkmark")
        .pos(body.x + 74, body.y + 60)
        .color(muted565)
        .bgColor(panel565);

    ui.drawIcon()
        .pos(body.x + 154, body.y + 28)
        .size(28)
        .icon(warning)
        .color(warn(ui))
        .bgColor(panel565);

    ui.drawText()
        .font(WixMadeForDisplay, 10)
        .weight(Medium)
        .text("warning")
        .pos(body.x + 148, body.y + 60)
        .color(muted565)
        .bgColor(panel565);

    ui.drawIcon()
        .pos(body.x + 52, body.y + 96)
        .size(30)
        .icon(error)
        .color(danger(ui))
        .bgColor(panel565);

    ui.drawText()
        .font(WixMadeForDisplay, 10)
        .weight(Medium)
        .text("error")
        .pos(body.x + 46, body.y + 132)
        .color(muted565)
        .bgColor(panel565);

    ui.drawText()
        .font(WixMadeForDisplay, 12)
        .weight(Semibold)
        .text("Layered battery example")
        .pos(body.x + body.w / 2, body.y + 152)
        .color(text565)
        .bgColor(panel565)
        .align(Center);

    ui.drawIcon()
        .pos(body.x + 104, body.y + 94)
        .size(34)
        .icon(battery_l0)
        .color(text565)
        .bgColor(panel565);

    ui.drawIcon()
        .pos(body.x + 104, body.y + 94)
        .size(34)
        .icon(battery_l1)
        .color(success(ui))
        .bgColor(panel565);

    ui.drawIcon()
        .pos(body.x + 104, body.y + 94)
        .size(34)
        .icon(battery_l2)
        .color(warn(ui))
        .bgColor(panel565);

    ui.drawIcon()
        .pos(body.x + 28, body.y + 186)
        .size(42)
        .icon(arrow)
        .color(accent2(ui))
        .bgColor(panel565);

    ui.drawIcon()
        .pos(body.x + 92, body.y + 188)
        .size(38)
        .icon(checkmark)
        .color(success(ui))
        .bgColor(panel565);

    ui.drawIcon()
        .pos(body.x + 150, body.y + 184)
        .size(44)
        .icon(error)
        .color(danger(ui))
        .bgColor(panel565);

    drawFooter(ui, "PREV back");
}
