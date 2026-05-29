SCREEN(tri, ScreenTri)
{
    const uint16_t bg565 = bg(ui);
    const auto body = bodyInset(ui, 10, 10, 8);
    const auto left = splitLeft(body, 8);
    const auto right = splitRight(body, 8);

    ui.clear(bg565);
    drawHeader(ui, "Triangles", "Isosceles preset: size + direction, no radius", success(ui));
    drawPanel(ui, left.x, left.y, left.w, left.h, panel(ui), line(ui), 12);
    drawPanel(ui, right.x, right.y, right.w, right.h, panel(ui), line(ui), 12);

    auto f = ui.drawTriangle().radius(0).fill(accent(ui)).border(1, fg(ui));
    f.derive().pos(left.x + 18, left.y + 34).size(30, 28).direction(Up);
    f.derive().pos(left.x + 60, left.y + 40).size(36, 30).direction(Right).fill(accent2(ui));
    f.derive().pos(left.x + 18, left.y + 94).size(46, 36).direction(Down).fill(warn(ui));
    f.derive().pos(left.x + 62, left.y + 128).size(34, 46).direction(Left).fill(danger(ui));

    auto s = ui.drawTriangle().radius(0).border(2, accent(ui));
    s.derive().pos(right.x + 18, right.y + 34).size(30, 28).direction(Up);
    s.derive().pos(right.x + 60, right.y + 40).size(36, 30).direction(Right).border(2, accent2(ui));
    s.derive().pos(right.x + 18, right.y + 94).size(46, 36).direction(Down).border(2, warn(ui));
    s.derive().pos(right.x + 62, right.y + 128).size(34, 46).direction(Left).border(2, danger(ui));

    drawFooter(ui, "Filled left / stroked right");
}
