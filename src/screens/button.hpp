SCREEN(button, ScreenButton)
{
    const uint16_t bg565 = bg(ui);
    ui.clear(bg565);
    drawHeader(ui, "Buttons", "drawButton(): text/icon/size/radius/color/progress", accent(ui));

    ui.drawButton().label("Text").pos(18, 58).size(92, 30).baseColor(accent(ui)).fillColor(fg(ui)).radius(8);
    ui.drawButton().label("").icon(checkmark).pos(130, 58).size(54, 30).baseColor(accent2(ui)).fillColor(fg(ui)).radius(15);
    ui.drawButton().label("Icon").icon(arrow).pos(18, 104).size(128, 36).baseColor(warn(ui)).fillColor(bg565).radius(6);
    ui.drawButton().label("Disabled").pos(18, 154).size(112, 32).baseColor(panelAlt(ui)).fillColor(muted(ui)).radius(12).mode(false, false);
    ui.drawButton().label("Loading").pos(144, 154).size(80, 32).baseColor(danger(ui)).fillColor(fg(ui)).radius(10).mode(true, true);
    ui.drawButton().label("Progress").pos(18, 206).size(184, 28).baseColor(accent2(ui)).fillColor(fg(ui)).radius(14).value(62);

    drawFooter(ui, "PREV back");
}
