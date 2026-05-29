SCREEN(squfill, ScreenSquFill)
{
    const uint16_t bg565 = bg(ui);
    const auto body = bodyInset(ui, 10, 10, 8);
    const auto left = splitLeft(body, 8);
    const auto right = splitRight(body, 8);

    ui.clear(bg565);
    drawHeader(ui, "Squircle fill", "Left per-corner radii, right single radius", success(ui));
    drawPanel(ui, left.x, left.y, left.w, left.h, panel(ui), line(ui), 12);
    drawPanel(ui, right.x, right.y, right.w, right.h, panel(ui), line(ui), 12);

    auto l = ui.drawSquircleRect().fill(accent(ui)).border(1, fg(ui));
    l.derive().pos(left.x + 12, left.y + 34).size(88, 34).radius(4, 18, 6, 14);
    l.derive().pos(left.x + 24, left.y + 88).size(64, 44).radius(18, 4, 18, 4).fill(accent2(ui));
    l.derive().pos(left.x + 10, left.y + 150).size(94, 34).radius(22, 22, 4, 4).fill(warn(ui));

    auto r = ui.drawSquircleRect().fill(accent(ui)).border(1, fg(ui));
    r.derive().pos(right.x + 14, right.y + 34).size(82, 32).radius(4);
    r.derive().pos(right.x + 10, right.y + 84).size(92, 42).radius(14).fill(accent2(ui));
    r.derive().pos(right.x + 24, right.y + 144).size(64, 50).radius(24).fill(warn(ui));

    drawFooter(ui, "PREV back");
}
