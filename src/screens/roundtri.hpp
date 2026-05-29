SCREEN(roundtri, ScreenRoundTri)
{
    const uint16_t bg565 = bg(ui);
    const auto body = bodyInset(ui, 10, 10, 8);
    const auto left = splitLeft(body, 8);
    const auto right = splitRight(body, 8);

    ui.clear(bg565);
    drawHeader(ui, "Round tri", "Isosceles preset with corner radius", accent(ui));
    drawPanel(ui, left.x, left.y, left.w, left.h, panel(ui), line(ui), 12);
    drawPanel(ui, right.x, right.y, right.w, right.h, panel(ui), line(ui), 12);

    auto f = ui.drawTriangle().radius(6).fill(accent(ui)).border(1, fg(ui));
    f.derive().pos(left.x + 18, left.y + 34).size(34, 32).direction(Up);
    f.derive().pos(left.x + 62, left.y + 40).size(42, 32).direction(Right).radius(9).fill(accent2(ui));
    f.derive().pos(left.x + 18, left.y + 100).size(52, 42).direction(Down).radius(12).fill(warn(ui));
    f.derive().pos(left.x + 62, left.y + 140).size(38, 48).direction(Left).radius(10).fill(danger(ui));

    auto s = ui.drawTriangle().radius(6).border(2, accent(ui));
    s.derive().pos(right.x + 18, right.y + 34).size(34, 32).direction(Up);
    s.derive().pos(right.x + 62, right.y + 40).size(42, 32).direction(Right).radius(9).border(2, accent2(ui));
    s.derive().pos(right.x + 18, right.y + 100).size(52, 42).direction(Down).radius(12).border(2, warn(ui));
    s.derive().pos(right.x + 62, right.y + 140).size(38, 48).direction(Left).radius(10).border(2, danger(ui));

    drawFooter(ui, "SDF rounded triangle AA");
}
