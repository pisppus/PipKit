SCREEN(dotsmany, ScreenDotsMany)
{
    const DemoState &st = runtimeState();
    const uint16_t bg565 = bg(ui);
    ui.clear(bg565);
    drawHeader(ui, "Dots >7", "updateScrollDots() windowing/taper stress", warn(ui));

    ui.updateScrollDots().pos(24, 86).count(12).activeIndex(st.dotsMany).radius(3).spacing(13).activeColor(accent2(ui)).inactiveColor(line(ui));
    ui.updateScrollDots().pos(18, 136).count(15).activeIndex(st.dotsMany).radius(2).spacing(10).activeColor(danger(ui)).inactiveColor(panelAlt(ui));
    ui.drawText().font(WixMadeForDisplay, 13).weight(Semibold).text(String("active=") + st.dotsMany).pos(center, 184).color(fg(ui)).bgColor(bg565).align(Center);
    drawFooter(ui, "PREV hold back");
}
