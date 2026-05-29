SCREEN(gradv, ScreenGradV)
{
    const uint16_t bg565 = bg(ui);
    const auto body = bodyInset(ui, 12, 10, 8);
    ui.clear(bg565);
    drawHeader(ui, "Gradient V", "Vertical top/bottom colors, sizes and low contrast", accent(ui));
    ui.gradientVertical().pos(body.x, body.y).size(body.w, 52).TColor(danger(ui)).BColor(accent2(ui));
    ui.gradientVertical().pos(body.x + 18, body.y + 72).size(body.w - 36, 44).TColor(accent(ui)).BColor(warn(ui));
    ui.gradientVertical().pos(body.x + 44, body.y + 136).size(body.w - 88, 54).TColor(panel(ui)).BColor(panelAlt(ui));
    drawFooter(ui, "PREV back");
}
