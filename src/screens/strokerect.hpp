SCREEN(strokerect, ScreenStrokeRect)
{
    const uint16_t bg565 = bg(ui);
    const uint16_t panel565 = panel(ui);
    const auto body = bodyInset(ui, 12, 10, 8);
    const auto left = splitLeft(body, 10);
    const auto right = splitRight(body, 10);

    ui.clear(bg565);
    drawHeader(ui, "Stroke rects", "Border-only coverage for both radius modes", accent2(ui));
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
        .size(82, 34)
        .radius(4, 12, 18, 6)
        .border(1, accent(ui));

    ui.drawRect()
        .pos(left.x + 26, left.y + 82)
        .size(58, 42)
        .radius(18, 4, 18, 4)
        .border(1, accent2(ui));

    ui.drawRect()
        .pos(left.x + 8, left.y + 142)
        .size(98, 30)
        .radius(14, 14, 2, 2)
        .border(1, warn(ui));

    ui.drawRect()
        .pos(right.x + 16, right.y + 34)
        .size(76, 30)
        .radius(4)
        .border(1, accent(ui));

    ui.drawRect()
        .pos(right.x + 10, right.y + 80)
        .size(92, 38)
        .radius(10)
        .border(1, accent2(ui));

    ui.drawRect()
        .pos(right.x + 20, right.y + 136)
        .size(70, 46)
        .radius(18)
        .border(1, warn(ui));

    drawFooter(ui, "PREV back");
}
