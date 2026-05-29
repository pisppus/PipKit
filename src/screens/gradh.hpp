SCREEN(gradh, ScreenGradH)
{
    const uint16_t bg565 = bg(ui);
    const auto body = bodyInset(ui, 12, 10, 8);
    ui.clear(bg565);
    drawHeader(ui, "Gradient H", "Horizontal left/right colors and aspect ratios", accent2(ui));
    ui.gradientHorizontal().pos(body.x, body.y).size(body.w, 52).LColor(danger(ui)).RColor(accent(ui));
    ui.gradientHorizontal().pos(body.x + 18, body.y + 72).size(body.w - 36, 44).LColor(warn(ui)).RColor(accent2(ui));
    ui.gradientHorizontal().pos(body.x + 44, body.y + 136).size(body.w - 88, 54).LColor(panel(ui)).RColor(panelAlt(ui));
    drawFooter(ui, "PREV back");
}
