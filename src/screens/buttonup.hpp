SCREEN(buttonup, ScreenButtonUp)
{
    const DemoState &st = runtimeState();
    const uint16_t bg565 = bg(ui);
    ui.clear(bg565);
    drawHeader(ui, "Button update", "NEXT toggles/down+progress, PREV changes row, PREV hold backs", accent2(ui));

    ui.updateButton().label(st.buttonDown ? "Pressed" : "Press me").pos(18, 62).size(128, 34).baseColor(accent(ui)).fillColor(fg(ui)).radius(10).icon(checkmark).down(st.buttonDown);
    ui.updateButton().label("Progress").pos(18, 116).size(184, 30).baseColor(accent2(ui)).fillColor(fg(ui)).radius(15).value(st.buttonValue);
    ui.updateButton().label("Loading").pos(18, 168).size(94, 32).baseColor(warn(ui)).fillColor(bg565).radius(8).mode(true, st.controlRow == 1u);
    ui.updateButton().label("Disabled").pos(128, 168).size(94, 32).baseColor(panelAlt(ui)).fillColor(muted(ui)).radius(8).mode(st.controlRow != 2u, false);

    drawFooter(ui, "Controlled button uses cache invalidation path");
}
