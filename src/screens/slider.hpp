SCREEN(slider, ScreenSlider)
{
    static int16_t a = 20;
    static int16_t b = 58;
    static int16_t c = 88;
    const uint16_t bg565 = bg(ui);
    ui.clear(bg565);
    drawHeader(ui, "Sliders", "drawSlider(): sizes, values and colors", accent2(ui));

    ui.drawSlider().pos(24, 68).size(188, 22).bind(a).activeColor(accent(ui)).inactiveColor(line(ui)).thumbColor(fg(ui));
    ui.drawSlider().pos(24, 118).size(188, 14).bind(b).activeColor(warn(ui)).inactiveColor(panelAlt(ui)).thumbColor(fg(ui));
    ui.drawSlider().pos(24, 164).size(188, 30).bind(c).activeColor(danger(ui)).inactiveColor(line(ui)).thumbColor(accent2(ui));
    ui.drawSlider().pos(24, 214).size(188, 22).bind(c).activeColor(panelAlt(ui)).inactiveColor(line(ui)).thumbColor(muted(ui)).enabled(false);

    drawFooter(ui, "PREV back");
}
