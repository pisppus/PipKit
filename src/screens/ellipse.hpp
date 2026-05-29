SCREEN(ellipse, ScreenEllipse)
{
    const uint16_t bg565 = bg(ui);
    const auto body = bodyInset(ui, 10, 10, 8);
    const auto left = splitLeft(body, 8);
    const auto right = splitRight(body, 8);

    ui.clear(bg565);
    drawHeader(ui, "Ellipses", "Left filled, right border-only; wide, tall and near-circle", accent2(ui));
    drawPanel(ui, left.x, left.y, left.w, left.h, panel(ui), line(ui), 12);
    drawPanel(ui, right.x, right.y, right.w, right.h, panel(ui), line(ui), 12);

    auto title = ui.drawText().font(WixMadeForDisplay, 12).weight(Semibold).color(fg(ui)).bgColor(panel(ui)).align(Center);
    title.derive().text("Fill").pos(left.x + left.w / 2, left.y + 10);
    title.derive().text("Stroke").pos(right.x + right.w / 2, right.y + 10);

    auto f = ui.drawEllipse().fill(accent(ui)).border(1, fg(ui));
    f.derive().pos(left.x + 34, left.y + 44).radiusX(22).radiusY(9);
    f.derive().pos(left.x + 74, left.y + 82).radiusX(10).radiusY(24).fill(warn(ui));
    f.derive().pos(left.x + 44, left.y + 136).radiusX(25).radiusY(21).fill(accent2(ui));
    f.derive().pos(left.x + 82, left.y + 160).radiusX(5).radiusY(8).fill(danger(ui));

    auto s = ui.drawEllipse().border(2, accent(ui));
    s.derive().pos(right.x + 34, right.y + 44).radiusX(22).radiusY(9);
    s.derive().pos(right.x + 74, right.y + 82).radiusX(10).radiusY(24).border(2, warn(ui));
    s.derive().pos(right.x + 44, right.y + 136).radiusX(25).radiusY(21).border(3, accent2(ui));
    s.derive().pos(right.x + 82, right.y + 160).radiusX(5).radiusY(8).border(1, danger(ui));

    drawFooter(ui, "PREV back");
}
