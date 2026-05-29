SCREEN(toggleup, ScreenToggleUp)
{
    DemoState &st = runtimeState();
    const uint16_t bg565 = bg(ui);
    const bool autoValue = ((millis() / 700u) & 1u) != 0u;
    bool changed = false;

    ui.clear(bg565);
    drawHeader(ui, "Toggle update", "NEXT toggles first, PREV toggles auto enable, PREV hold backs", accent(ui));

    ui.updateToggleSwitch().pos(34, 70).size(78, 36).value(st.toggleUser).changed(changed).activeColor(accent(ui)).inactiveColor(line(ui)).knobColor(fg(ui));
    ui.updateToggleSwitch().pos(134, 70).size(78, 36).value(st.toggleAuto).activeColor(warn(ui)).inactiveColor(panelAlt(ui)).knobColor(fg(ui));
    bool animValue = st.toggleAuto ? autoValue : false;
    ui.updateToggleSwitch().pos(34, 136).size(178, 36).value(animValue).activeColor(accent2(ui)).inactiveColor(line(ui)).enabled(st.toggleAuto);

    ui.drawText().font(WixMadeForDisplay, 12).weight(Medium).text(changed ? "changed" : "stable").pos(center, 196).color(changed ? accent(ui) : muted(ui)).bgColor(bg565).align(Center);
    drawFooter(ui, "PREV hold back");
}
