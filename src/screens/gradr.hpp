SCREEN(gradr, ScreenGradR)
{
    const uint16_t bg565 = bg(ui);
    const auto body = bodyInset(ui, 12, 10, 8);
    ui.clear(bg565);
    drawHeader(ui, "Gradient R", "Radial center, radius, inner/outer colors", accent(ui));
    ui.gradientRadial().pos(body.x, body.y).size(body.w, 82).center(body.x + body.w / 2, body.y + 40).radius(68).innerColor(fg(ui)).outerColor(accent2(ui));
    ui.gradientRadial().pos(body.x, body.y + 106).size(body.w / 2 - 4, 88).center(body.x + 18, body.y + 128).radius(74).innerColor(warn(ui)).outerColor(bg565);
    ui.gradientRadial().pos(body.x + body.w / 2 + 4, body.y + 106).size(body.w / 2 - 4, 88).center(body.x + body.w - 20, body.y + 178).radius(58).innerColor(accent(ui)).outerColor(panel(ui));
    drawFooter(ui, "PREV back");
}
