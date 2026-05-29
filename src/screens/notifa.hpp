SCREEN(notifa, ScreenNotifA)
{
    const uint16_t bg565 = bg(ui);
    ui.clear(bg565);
    drawHeader(ui, "Notif A", "NEXT shows timed Normal/Warning notification", accent(ui));
    drawPanel(ui, 20, 72, 200, 118, panel(ui), line(ui), 14);
    ui.drawText().font(WixMadeForDisplay, 14).weight(Semibold).text("Timer + type").pos(center, 102).color(fg(ui)).bgColor(panel(ui)).align(Center);
    ui.drawText().font(WixMadeForDisplay, 11).weight(Medium).text("delay(3), icon(), button(\"OK\")").pos(center, 128).color(muted(ui)).bgColor(panel(ui)).align(Center);
    ui.drawButton().label("Show").pos(center, 154).size(112, 30).baseColor(accent(ui)).radius(10);
    drawFooter(ui, "PREV hold back");
}
