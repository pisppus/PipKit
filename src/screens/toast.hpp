SCREEN(toast, ScreenToast)
{
    const uint16_t bg565 = bg(ui);
    ui.clear(bg565);
    drawHeader(ui, "Toast", "NEXT cycles text/icon/position variants", warn(ui));
    drawPanel(ui, 22, 70, 196, 112, panel(ui), line(ui), 14);
    ui.drawIcon().pos(46, 100).size(34).icon(checkmark).color(accent(ui)).bgColor(panel(ui));
    ui.drawText().font(WixMadeForDisplay, 13).weight(Semibold).text("Press NEXT").pos(104, 102).color(fg(ui)).bgColor(panel(ui));
    ui.drawText().font(WixMadeForDisplay, 11).weight(Medium).text("Top/down, warning/check/network text").pos(104, 122).color(muted(ui)).bgColor(panel(ui));
    drawFooter(ui, "PREV hold back");
}
