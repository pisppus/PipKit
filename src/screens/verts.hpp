SCREEN(verts, ScreenVerts)
{
    const uint16_t bg565 = bg(ui);
    const auto body = bodyInset(ui, 10, 10, 8);
    const auto left = splitLeft(body, 8);
    const auto right = splitRight(body, 8);

    ui.clear(bg565);
    drawHeader(ui, "Tri vertices", "Arbitrary vertex forms without rounding", warn(ui));
    drawPanel(ui, left.x, left.y, left.w, left.h, panel(ui), line(ui), 12);
    drawPanel(ui, right.x, right.y, right.w, right.h, panel(ui), line(ui), 12);

    auto f = ui.drawTriangle().radius(0).fill(accent(ui)).border(1, fg(ui));
    f.derive().vertices(left.x + 16, left.y + 38, left.x + 94, left.y + 52, left.x + 32, left.y + 86);
    f.derive().vertices(left.x + 24, left.y + 116, left.x + 82, left.y + 96, left.x + 68, left.y + 164).fill(accent2(ui));
    f.derive().vertices(left.x + 8, left.y + 178, left.x + 102, left.y + 170, left.x + 58, left.y + 202).fill(warn(ui));

    auto s = ui.drawTriangle().radius(0).border(2, accent(ui));
    s.derive().vertices(right.x + 16, right.y + 38, right.x + 94, right.y + 52, right.x + 32, right.y + 86);
    s.derive().vertices(right.x + 24, right.y + 116, right.x + 82, right.y + 96, right.x + 68, right.y + 164).border(2, accent2(ui));
    s.derive().vertices(right.x + 8, right.y + 178, right.x + 102, right.y + 170, right.x + 58, right.y + 202).border(2, warn(ui));

    drawFooter(ui, "Vertex API stress: flat and skewed edges");
}
