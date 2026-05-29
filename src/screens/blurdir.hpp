SCREEN(blurdir, ScreenBlurDir)
{
    const uint16_t bg565 = bg(ui);
    const auto body = bodyInset(ui, 12, 10, 8);
    ui.clear(bg565);
    drawHeader(ui, "Blur dir", "Static drawBlur() with directional gradient masks", warn(ui));

    drawPanel(ui, body.x, body.y, body.w, body.h, ui.rgb(8, 12, 16), line(ui), 14);
    for (uint8_t i = 0; i < 5; ++i)
        ui.drawLine().from(body.x + 8, body.y + 24 + i * 30).to(body.x + body.w - 8, body.y + 62 + i * 20).thickness(4).color((i & 1u) ? accent(ui) : warn(ui));
    ui.drawCircle().pos(body.x + 46, body.y + 128).radius(20).fill(accent2(ui));
    ui.drawSquircleRect().pos(body.x + 142, body.y + 112).size(54, 54).radius(16).fill(danger(ui));

    ui.drawBlur().pos(body.x + 10, body.y + 24).size(92, 46).radius(10).direction(TopDown).material(128, -1);
    ui.drawBlur().pos(body.x + 128, body.y + 24).size(92, 46).radius(10).direction(BottomUp).material(128, -1);
    ui.drawBlur().pos(body.x + 10, body.y + 102).size(92, 46).radius(10).direction(LeftRight).material(128, -1);
    ui.drawBlur().pos(body.x + 128, body.y + 102).size(92, 46).radius(10).direction(RightLeft).material(128, -1);

    auto t = ui.drawText().font(WixMadeForDisplay, 10).weight(Medium).color(fg(ui)).bgColor(bg565).align(Center);
    t.derive().text("TopDown").pos(body.x + 56, body.y + 76);
    t.derive().text("BottomUp").pos(body.x + 174, body.y + 76);
    t.derive().text("LeftRight").pos(body.x + 56, body.y + 154);
    t.derive().text("RightLeft").pos(body.x + 174, body.y + 154);
    drawFooter(ui, "PREV back");
}
