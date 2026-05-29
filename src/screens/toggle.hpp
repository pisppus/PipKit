SCREEN(toggle, ScreenToggle)
{
    static bool on = true;
    static bool off = false;
    static bool alt = true;
    const uint16_t bg565 = bg(ui);
    ui.clear(bg565);
    drawHeader(ui, "Toggles", "drawToggleSwitch(): size, value, colors, enabled", success(ui));

    ui.drawToggleSwitch().pos(28, 66).size(64, 30).value(on).activeColor(accent(ui)).inactiveColor(line(ui)).knobColor(fg(ui));
    ui.drawToggleSwitch().pos(128, 66).size(86, 38).value(off).activeColor(warn(ui)).inactiveColor(panelAlt(ui)).knobColor(fg(ui));
    ui.drawToggleSwitch().pos(28, 126).size(48, 24).value(alt).activeColor(accent2(ui)).inactiveColor(line(ui)).knobColor(fg(ui));
    ui.drawToggleSwitch().pos(128, 126).size(86, 38).value(on).activeColor(danger(ui)).inactiveColor(panelAlt(ui)).enabled(false);

    drawFooter(ui, "PREV back");
}
