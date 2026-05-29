SCREEN(popupa, ScreenPopupA)
{
    const uint16_t bg565 = bg(ui);
    ui.clear(bg565);
    drawHeader(ui, "Popup A", "NEXT opens 4-item menu anchored to button", accent2(ui));
    ui.drawText().font(WixMadeForDisplay, 12).weight(Medium).text("Popup is created from nav handler to test anchor extraction").pos(center, 86).color(muted(ui)).bgColor(bg565).align(Center);
    ui.drawButton().label("Anchor").pos(center, 154).size(142, 34).baseColor(accent2(ui)).radius(11);
    drawFooter(ui, "PREV hold back");
}
