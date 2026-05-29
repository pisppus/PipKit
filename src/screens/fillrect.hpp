SCREEN(fillrect, ScreenFillRect)
{
    const uint16_t bg565 = bg(ui);
    const uint16_t panel565 = panel(ui);
    const auto body = bodyInset(ui, 12, 10, 8);
    const auto left = splitLeft(body, 10);
    const auto right = splitRight(body, 10);

    ui.clear(bg565);
    drawHeader(ui, "Filled rects", "Left: independent radii, right: single radius", accent(ui));
    drawPanel(ui, left.x, left.y, left.w, left.h, panel565, line(ui), 12);
    drawPanel(ui, right.x, right.y, right.w, right.h, panel565, line(ui), 12);

    ui.drawText()
        .font(WixMadeForDisplay, 12)
        .weight(Semibold)
        .text("Per-corner radii")
        .pos(left.x + left.w / 2, left.y + 10)
        .color(fg(ui))
        .bgColor(panel565)
        .align(Center);

    ui.drawText()
        .font(WixMadeForDisplay, 12)
        .weight(Semibold)
        .text("Single radius")
        .pos(right.x + right.w / 2, right.y + 10)
        .color(fg(ui))
        .bgColor(panel565)
        .align(Center);

    ui.drawRect()
        .pos(left.x + 14, left.y + 34)
        .size(80, 34)
        .radius(3, 14, 10, 18)
        .fill(accent(ui))
        .border(1, fg(ui));

    ui.drawRect()
        .pos(left.x + 24, left.y + 84)
        .size(62, 42)
        .radius(16, 6, 16, 6)
        .fill(accent2(ui))
        .border(1, fg(ui));

    ui.drawRect()
        .pos(left.x + 10, left.y + 142)
        .size(96, 32)
        .radius(18, 18, 4, 4)
        .fill(warn(ui))
        .border(1, fg(ui));

    ui.drawRect()
        .pos(right.x + 16, right.y + 34)
        .size(76, 30)
        .radius(4)
        .fill(accent(ui))
        .border(1, fg(ui));

    ui.drawRect()
        .pos(right.x + 10, right.y + 80)
        .size(92, 38)
        .radius(10)
        .fill(accent2(ui))
        .border(1, fg(ui));

    ui.drawRect()
        .pos(right.x + 20, right.y + 136)
        .size(70, 46)
        .radius(18)
        .fill(warn(ui))
        .border(1, fg(ui));

    drawFooter(ui, "PREV back");
}
