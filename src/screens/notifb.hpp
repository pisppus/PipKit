SCREEN(notifb, ScreenNotifB)
{
    const uint16_t bg565 = bg(ui);
    ui.clear(bg565);
    drawHeader(ui, "Notif B", "NEXT shows button/icon/Error variants", danger(ui));
    drawPanel(ui, 20, 72, 200, 118, panel(ui), line(ui), 14);
    ui.drawText().font(WixMadeForDisplay, 14).weight(Semibold).text("Action variant").pos(center, 102).color(fg(ui)).bgColor(panel(ui)).align(Center);
    ui.drawText().font(WixMadeForDisplay, 11).weight(Medium).text("button text alternates: Retry / Install").pos(center, 128).color(muted(ui)).bgColor(panel(ui)).align(Center);
    ui.drawButton().label("Show").icon(warning).pos(center, 154).size(112, 30).baseColor(danger(ui)).radius(10);
    drawFooter(ui, "PREV hold back");
}
