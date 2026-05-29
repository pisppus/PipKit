SCREEN(dots7, ScreenDots7)
{
    const DemoState &st = runtimeState();
    const uint16_t bg565 = bg(ui);
    ui.clear(bg565);
    drawHeader(ui, "Dots <=7", "NEXT/PREV changes active dot, PREV hold backs", accent(ui));

    ui.updateScrollDots().pos(38, 86).count(7).activeIndex(st.dots7).radius(4).spacing(18).activeColor(accent(ui)).inactiveColor(line(ui));
    ui.updateScrollDots().pos(52, 136).count(5).activeIndex(st.dots7 % 5u).radius(6).spacing(24).activeColor(warn(ui)).inactiveColor(panelAlt(ui));
    ui.drawText().font(WixMadeForDisplay, 13).weight(Semibold).text(String("active=") + st.dots7).pos(center, 184).color(fg(ui)).bgColor(bg565).align(Center);
    drawFooter(ui, "PREV hold back");
}
