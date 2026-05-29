SCREEN(sliderup, ScreenSliderUp)
{
    DemoState &st = runtimeState();
    int16_t autoValue = static_cast<int16_t>((sinf(static_cast<float>(millis() % 4000u) * 0.00157f) + 1.0f) * 50.0f);
    const uint16_t bg565 = bg(ui);
    ui.clear(bg565);
    drawHeader(ui, "Slider update", "NEXT/PREV adjust first slider, PREV hold backs", warn(ui));

    ui.updateSlider().pos(24, 72).size(188, 24).bind(st.sliderUser).activeColor(accent(ui)).inactiveColor(line(ui)).thumbColor(fg(ui));
    ui.updateSlider().pos(24, 132).size(188, 18).bind(autoValue).activeColor(accent2(ui)).inactiveColor(panelAlt(ui)).thumbColor(fg(ui));
    ui.updateSlider().pos(24, 188).size(188, 26).bind(autoValue).activeColor(warn(ui)).inactiveColor(line(ui)).thumbColor(danger(ui));
    ui.drawText().font(WixMadeForDisplay, 12).weight(Semibold).text(String("user=") + st.sliderUser).pos(center, 232).color(fg(ui)).bgColor(bg565).align(Center);
}
