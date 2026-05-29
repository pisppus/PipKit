SCREEN(drumh, ScreenDrumH)
{
    static const char *const opts[] = {"Off", "5m", "10m", "30m", "1h", "2h"};
    const uint16_t bg565 = bg(ui);
    ui.clear(bg565);
    drawHeader(ui, "Drum H", "drawDrumRoll() horizontal: options, selected, colors", accent(ui));

    ui.drawDrumRoll().pos(0, 82).size(ui.screenWidth(), 34).options(18, opts).selected(runtimeState().drumH).fgColor(fg(ui)).bgColor(bg565);
    ui.drawDrumRoll().pos(18, 150).size(204, 28).options(14, "Low", "Medium", "High").selected(runtimeState().drumH % 3u).fgColor(warn(ui)).bgColor(panel(ui));
    drawFooter(ui, "NEXT/PREV changes, PREV hold back");
}
