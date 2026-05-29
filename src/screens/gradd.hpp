SCREEN(gradd, ScreenGradD)
{
    const uint16_t bg565 = bg(ui);
    const auto body = bodyInset(ui, 12, 10, 8);
    ui.clear(bg565);
    drawHeader(ui, "Gradient D", "Diagonal TL/BR color interpolation", warn(ui));
    ui.gradientDiagonal().pos(body.x, body.y).size(body.w, 66).TLColor(fg(ui)).BRColor(danger(ui));
    ui.gradientDiagonal().pos(body.x + 18, body.y + 88).size(body.w - 36, 54).TLColor(accent(ui)).BRColor(accent2(ui));
    ui.gradientDiagonal().pos(body.x + 44, body.y + 160).size(body.w - 88, 38).TLColor(panelAlt(ui)).BRColor(bg565);
    drawFooter(ui, "PREV back");
}
