SCREEN(glow, ScreenGlow)
{
    const uint16_t bg565 = bg(ui);
    ui.clear(bg565);
    drawHeader(ui, "Glow", "drawGlowCircle() radius, fill/glow colors, size, strength", danger(ui));

    auto g = ui.drawGlowCircle().bgColor(bg565).fillColor(danger(ui)).glowColor(danger(ui)).glowSize(18).glowStrength(220);
    g.derive().pos(58, 86).radius(22);
    g.derive().pos(152, 86).radius(30).fillColor(accent2(ui)).glowColor(accent2(ui)).glowSize(24).glowStrength(240);
    g.derive().pos(70, 172).radius(26).fillColor(accent(ui)).glowColor(warn(ui)).glowSize(16).glowStrength(170);
    g.derive().pos(172, 178).radius(18).fillColor(warn(ui)).glowColor(danger(ui)).glowSize(28).glowStrength(210);

    drawFooter(ui, "PREV back");
}
