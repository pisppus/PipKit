SCREEN(popupb, ScreenPopupB)
{
    const uint16_t bg565 = bg(ui);
    ui.clear(bg565);
    drawHeader(ui, "Popup B", "NEXT opens 6-item wider popup with selected row", warn(ui));
    ui.drawText().font(WixMadeForDisplay, 12).weight(Medium).text("Width/count/selection stress, anchored to fluent button").pos(center, 86).color(muted(ui)).bgColor(bg565).align(Center);
    ui.drawButton().label("Wide anchor").pos(22, 154).size(196, 34).baseColor(warn(ui)).fillColor(bg565).radius(8);
    drawFooter(ui, "PREV hold back");
}
