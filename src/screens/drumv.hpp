SCREEN(drumv, ScreenDrumV)
{
    static const char *const opts[] = {"XS", "S", "M", "L", "XL"};
    const uint16_t bg565 = bg(ui);
    ui.clear(bg565);
    drawHeader(ui, "Drum V", "drawDrumRoll() vertical sizing and color variants", accent2(ui));

    ui.drawDrumRoll().pos(36, 62).size(70, 136).options(18, opts).selected(runtimeState().drumV).fgColor(fg(ui)).bgColor(bg565).vertical();
    ui.drawDrumRoll().pos(138, 78).size(58, 104).options(14, "A", "B", "C", "D").selected(runtimeState().drumV % 4u).fgColor(warn(ui)).bgColor(panel(ui)).vertical();
    drawFooter(ui, "NEXT/PREV changes, PREV hold back");
}
