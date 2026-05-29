SCREEN(errwarn, ScreenErrWarn)
{
    const uint16_t bg565 = bg(ui);
    ui.clear(bg565);
    drawHeader(ui, "Warning", "NEXT invokes showError().type(Warning)", warn(ui));
    drawPanel(ui, 28, 84, 184, 96, panel(ui), warn(ui), 14);
    ui.drawIcon().pos(48, 112).size(34).icon(warning).color(warn(ui)).bgColor(panel(ui));
    ui.drawText().font(WixMadeForDisplay, 14).weight(Semibold).text("Warning overlay").pos(94, 116).color(fg(ui)).bgColor(panel(ui));
    drawFooter(ui, "PREV hold back");
}
