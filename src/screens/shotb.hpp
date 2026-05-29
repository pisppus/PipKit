SCREEN(shotb, ScreenShotB)
{
    const uint16_t bg565 = bg(ui);
    ui.clear(bg565);
    drawHeader(ui, "Shots B", "Offset gallery window, stable layout, screenshotCount()", accent2(ui));
    drawPanel(ui, 30, 58, 180, 142, panel(ui), line(ui), 12);
    ui.drawScreenshot().pos(38, 66).size(164, 126).grid(2, 2).padding(8);
    ui.drawText().font(WixMadeForDisplay, 11).weight(Medium).text(String("screenshotCount() = ") + ui.screenshotCount()).pos(center, 220).color(fg(ui)).bgColor(bg565).align(Center);
    drawFooter(ui, "NEXT capture, PREV hold back");
}
