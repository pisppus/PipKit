SCREEN(squline, ScreenSquLine)
{
    const uint16_t bg565 = bg(ui);
    const auto body = bodyInset(ui, 10, 10, 8);
    const auto left = splitLeft(body, 8);
    const auto right = splitRight(body, 8);

    ui.clear(bg565);
    drawHeader(ui, "Squircle line", "Border-only squircle rect variants", accent2(ui));
    drawPanel(ui, left.x, left.y, left.w, left.h, panel(ui), line(ui), 12);
    drawPanel(ui, right.x, right.y, right.w, right.h, panel(ui), line(ui), 12);

    auto l = ui.drawSquircleRect().border(2, accent(ui));
    l.derive().pos(left.x + 12, left.y + 34).size(88, 34).radius(4, 18, 6, 14);
    l.derive().pos(left.x + 24, left.y + 88).size(64, 44).radius(18, 4, 18, 4).border(3, accent2(ui));
    l.derive().pos(left.x + 10, left.y + 150).size(94, 34).radius(22, 22, 4, 4).border(2, warn(ui));

    auto r = ui.drawSquircleRect().border(2, accent(ui));
    r.derive().pos(right.x + 14, right.y + 34).size(82, 32).radius(4);
    r.derive().pos(right.x + 10, right.y + 84).size(92, 42).radius(14).border(3, accent2(ui));
    r.derive().pos(right.x + 24, right.y + 144).size(64, 50).radius(24).border(2, warn(ui));

    drawFooter(ui, "PREV back");
}
