SCREEN(blur, ScreenBlur)
{
    const uint16_t bg565 = bg(ui);
    const auto body = bodyInset(ui, 12, 10, 8);
    ui.clear(bg565);
    drawHeader(ui, "Blur", "Static drawBlur() without direction, radius/material sweep", accent2(ui));

    drawPanel(ui, body.x, body.y, body.w, body.h, ui.rgb(8, 12, 16), line(ui), 14);
    ui.drawCircle().pos(body.x + 40, body.y + 54).radius(26).fill(warn(ui));
    ui.drawSquircleRect().pos(body.x + 118, body.y + 70).size(86, 54).radius(22).fill(accent(ui));
    ui.drawRect().pos(body.x + 24, body.y + 96).size(172, 4).radius(2).fill(accent2(ui));
    ui.drawRect().pos(body.x + 44, body.y + 112).size(132, 3).radius(1).fill(danger(ui));
    ui.drawIcon().pos(body.x + 28, body.y + 138).size(38).icon(checkmark).color(fg(ui)).bgColor(bg565);
    ui.drawText().font(WixMadeForDisplay, 20).weight(Bold).text("TEXT").pos(body.x + 156, body.y + 154).color(fg(ui)).bgColor(bg565).align(Center);

    ui.drawBlur().pos(body.x + 8, body.y + 24).size(86, 42).radius(6).material(72, -1);
    ui.drawBlur().pos(body.x + 118, body.y + 46).size(92, 58).radius(14).material(120, ui.rgb(20, 32, 44));
    ui.drawBlur().pos(body.x + 20, body.y + 126).size(82, 54).radius(22).material(180, ui.rgb(255, 255, 255));
    ui.drawBlur().pos(body.x + 128, body.y + 136).size(76, 42).radius(4).material(48, -1);

    drawFooter(ui, "Opaque underlay + material compositing");
}
