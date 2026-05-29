SCREEN(cprog, ScreenCircleProg)
{
    const uint16_t bg565 = bg(ui);
    ui.clear(bg565);
    drawHeader(ui, "Circle prog", "drawCircleProgress(): radius, value, thickness, colors", success(ui));

    auto c = ui.drawCircleProgress().pos(48, 86).radius(22).thickness(5).value(25).baseColor(panelAlt(ui)).fillColor(accent(ui)).anim(None);
    c.derive().pos(120, 86).radius(28).thickness(8).value(62).fillColor(accent2(ui)).anim(Shimmer);
    c.derive().pos(194, 86).radius(20).thickness(10).value(0).fillColor(warn(ui)).anim(Indeterminate);
    c.derive().pos(78, 174).radius(34).thickness(4).value(86).fillColor(danger(ui));
    c.derive().pos(164, 174).radius(30).thickness(12).value(44).fillColor(accent(ui)).anim(Shimmer);

    drawFooter(ui, "PREV back");
}
