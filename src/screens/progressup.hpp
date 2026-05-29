SCREEN(progressup, ScreenProgressUp)
{
    const DemoState &st = runtimeState();
    const uint8_t autoValue = static_cast<uint8_t>((millis() / 35u) % 101u);
    const uint16_t bg565 = bg(ui);
    ui.clear(bg565);
    drawHeader(ui, "Progress update", "NEXT/PREV changes user value, PREV hold backs", accent2(ui));

    ui.updateProgress().pos(22, 62).size(196, 14).value(st.progressUser).baseColor(panelAlt(ui)).fillColor(accent(ui)).radius(7).label("User", Left).labelColor(fg(ui)).percent(Right).percentColor(muted(ui));
    ui.updateProgress().pos(22, 106).size(196, 14).value(autoValue).baseColor(panelAlt(ui)).fillColor(warn(ui)).radius(7).label("Auto shimmer", Left).labelColor(fg(ui)).percent(Right).percentColor(muted(ui)).anim(Shimmer);
    ui.updateProgress().pos(22, 150).size(196, 22).value(0).baseColor(panelAlt(ui)).fillColor(danger(ui)).radius(11).label("Indeterminate", Left).labelColor(fg(ui)).anim(Indeterminate);
    drawFooter(ui, "Value cache + decorated update path");
}
