SCREEN(circle, ScreenCircle)
{
    const uint16_t bg565 = bg(ui);
    const uint16_t panel565 = panel(ui);
    const auto body = bodyInset(ui, 12, 10, 8);
    const auto left = splitLeft(body, 10);
    const auto right = splitRight(body, 10);
    const uint16_t text565 = fg(ui);

    ui.clear(bg565);
    drawHeader(ui, "Circles", "Filled circles on the left, stroked circles on the right", warn(ui));
    drawPanel(ui, left.x, left.y, left.w, left.h, panel565, line(ui), 12);
    drawPanel(ui, right.x, right.y, right.w, right.h, panel565, line(ui), 12);

    auto title = ui.drawText()
                     .font(WixMadeForDisplay, 12)
                     .weight(Semibold)
                     .color(text565)
                     .bgColor(panel565)
                     .align(Center);

    title.derive()
        .text("Filled")
        .pos(left.x + left.w / 2, left.y + 10);

    title.derive()
        .text("Border only")
        .pos(right.x + right.w / 2, right.y + 10);

    ui.drawCircle()
        .pos(left.x + 34, left.y + 54)
        .radius(12)
        .fill(accent(ui))
        .border(1, text565);

    ui.drawCircle()
        .pos(left.x + 76, left.y + 74)
        .radius(18)
        .fill(accent2(ui))
        .border(1, text565);

    ui.drawCircle()
        .pos(left.x + 46, left.y + 122)
        .radius(22)
        .fill(warn(ui))
        .border(1, text565);

    ui.drawCircle()
        .pos(left.x + 84, left.y + 154)
        .radius(16)
        .fill(danger(ui))
        .border(1, text565);

    ui.drawCircle()
        .pos(right.x + 36, right.y + 54)
        .radius(12)
        .border(1, accent(ui));

    ui.drawCircle()
        .pos(right.x + 78, right.y + 74)
        .radius(18)
        .border(1, accent2(ui));

    ui.drawCircle()
        .pos(right.x + 46, right.y + 122)
        .radius(22)
        .border(1, warn(ui));

    ui.drawCircle()
        .pos(right.x + 84, right.y + 154)
        .radius(16)
        .border(1, danger(ui));

    drawFooter(ui, "PREV back");
}
