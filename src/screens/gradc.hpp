SCREEN(gradc, ScreenGradC)
{
    const uint16_t bg565 = bg(ui);
    const auto body = bodyInset(ui, 12, 10, 8);
    ui.clear(bg565);
    drawHeader(ui, "Gradient C", "Four-corner interpolation and corner ordering", success(ui));
    ui.gradientCorners().pos(body.x, body.y).size(body.w, 92).TLColor(danger(ui)).TRColor(accent2(ui)).BLColor(accent(ui)).BRColor(warn(ui));
    ui.gradientCorners().pos(body.x + 28, body.y + 116).size(body.w - 56, 76).TLColor(panel(ui)).TRColor(fg(ui)).BLColor(bg565).BRColor(accent(ui));
    drawFooter(ui, "PREV back");
}
