SCREEN(glowup, ScreenGlowUp)
{
    const uint16_t bg565 = bg(ui);
    ui.clear(bg565);
    drawHeader(ui, "Glow update", "updateGlowCircle() pulse periods and strengths", accent(ui));

    auto g = ui.updateGlowCircle().bgColor(bg565).fillColor(danger(ui)).glowColor(danger(ui)).glowSize(18).glowStrength(220).anim(Pulse).pulseMs(700);
    g.derive().pos(58, 86).radius(22);
    g.derive().pos(152, 86).radius(30).fillColor(accent2(ui)).glowColor(accent2(ui)).glowSize(24).glowStrength(240).pulseMs(1100);
    g.derive().pos(70, 172).radius(26).fillColor(accent(ui)).glowColor(warn(ui)).glowSize(16).glowStrength(170).pulseMs(1500);
    g.derive().pos(172, 178).radius(18).fillColor(warn(ui)).glowColor(danger(ui)).glowSize(28).glowStrength(210).pulseMs(2100);

    drawFooter(ui, "Different pulse periods are intentionally out of phase");
}
