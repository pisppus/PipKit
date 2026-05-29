SCREEN(progress, ScreenProgress)
{
    const uint16_t bg565 = bg(ui);
    ui.clear(bg565);
    drawHeader(ui, "Progress", "drawProgress(): labels, percent, radius, animation", accent(ui));

    auto p = ui.drawProgress().pos(22, 60).size(196, 12).value(0).baseColor(panelAlt(ui)).fillColor(accent(ui)).radius(6).label("Indeterminate", Left).labelColor(fg(ui)).percent(Right).percentColor(muted(ui)).anim(Indeterminate);
    p.derive().pos(22, 96).value(38).fillColor(accent2(ui)).label("Shimmer").anim(Shimmer);
    p.derive().pos(22, 132).value(72).fillColor(warn(ui)).label("Determinate").anim(None);
    p.derive().pos(22, 174).size(196, 22).value(55).fillColor(danger(ui)).radius(11).label("", Left).percent(Center).percentColor(fg(ui)).percentFont(12);

    drawFooter(ui, "PREV back");
}
