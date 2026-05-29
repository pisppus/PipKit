SCREEN(cprogup, ScreenCircleProgUp)
{
    const DemoState &st = runtimeState();
    const uint8_t autoValue = static_cast<uint8_t>((millis() / 30u) % 101u);
    const uint16_t bg565 = bg(ui);
    ui.clear(bg565);
    drawHeader(ui, "CProg update", "updateCircleProgress() animated values", warn(ui));

    auto c = ui.updateCircleProgress().pos(58, 92).radius(26).thickness(7).value(st.progressUser).baseColor(panelAlt(ui)).fillColor(accent(ui)).anim(None);
    c.derive().pos(150, 92).radius(32).thickness(5).value(autoValue).fillColor(accent2(ui)).anim(Shimmer);
    c.derive().pos(76, 182).radius(30).thickness(11).value(0).fillColor(warn(ui)).anim(Indeterminate);
    c.derive().pos(166, 182).radius(24).thickness(12).value(100u - autoValue).fillColor(danger(ui)).anim(Shimmer);
    drawFooter(ui, "NEXT/PREV adjusts user ring");
}
