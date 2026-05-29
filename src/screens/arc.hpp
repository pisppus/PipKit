SCREEN(arc, ScreenArc)
{
    const uint16_t bg565 = bg(ui);
    const auto body = bodyInset(ui, 12, 10, 8);

    ui.clear(bg565);
    drawHeader(ui, "Arcs", "Radius, start/end angle, thickness and color", warn(ui));
    drawPanel(ui, body.x, body.y, body.w, body.h, panel(ui), line(ui), 14);

    auto a = ui.drawArc().radius(20).thickness(2).color(accent(ui));
    a.derive().pos(body.x + 38, body.y + 46).start(-90).end(0);
    a.derive().pos(body.x + 92, body.y + 46).start(0).end(180).color(accent2(ui)).thickness(4);
    a.derive().pos(body.x + 154, body.y + 46).radius(24).start(180).end(330).color(warn(ui)).thickness(6);
    a.derive().pos(body.x + 204, body.y + 46).radius(16).start(45).end(315).color(danger(ui)).thickness(3);

    a.derive().pos(body.x + 54, body.y + 126).radius(34).start(-120).end(120).color(accent(ui)).thickness(8);
    a.derive().pos(body.x + 128, body.y + 126).radius(38).start(0).end(360).color(accent2(ui)).thickness(5);
    a.derive().pos(body.x + 202, body.y + 126).radius(30).start(220).end(500).color(warn(ui)).thickness(10);

    ui.drawText().font(WixMadeForDisplay, 11).weight(Medium).text("Wrap >360, full circle, thin/thick strokes").pos(center, body.y + body.h - 24).color(muted(ui)).bgColor(panel(ui)).align(Center);
    drawFooter(ui, "PREV back");
}
