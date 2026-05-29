SCREEN(dots, ScreenDots)
{
    const uint16_t bg565 = bg(ui);
    ui.clear(bg565);
    drawHeader(ui, "Scroll dots", "drawScrollDots(): <=7, >7, radius and spacing", accent2(ui));

    ui.drawScrollDots().pos(38, 72).count(5).activeIndex(2).radius(3).spacing(14).activeColor(accent(ui)).inactiveColor(line(ui));
    ui.drawScrollDots().pos(34, 112).count(7).activeIndex(5).radius(4).spacing(17).activeColor(warn(ui)).inactiveColor(panelAlt(ui));
    ui.drawScrollDots().pos(26, 158).count(12).activeIndex(9).radius(3).spacing(12).activeColor(accent2(ui)).inactiveColor(line(ui));
    ui.drawScrollDots().pos(24, 202).count(15).activeIndex(4).radius(2).spacing(9).activeColor(danger(ui)).inactiveColor(panelAlt(ui));

    drawFooter(ui, "PREV back");
}
