SCREEN(errcrash, ScreenErrCrash)
{
    const uint16_t bg565 = bg(ui);
    ui.clear(bg565);
    drawHeader(ui, "Error", "NEXT invokes showError().type(Crash)", danger(ui));
    drawPanel(ui, 28, 84, 184, 96, panel(ui), danger(ui), 14);
    ui.drawIcon().pos(48, 112).size(34).icon(error).color(danger(ui)).bgColor(panel(ui));
    ui.drawText().font(WixMadeForDisplay, 14).weight(Semibold).text("Error overlay").pos(94, 116).color(fg(ui)).bgColor(panel(ui));
    drawFooter(ui, "PREV hold back");
}
