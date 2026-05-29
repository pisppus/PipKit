SCREEN(shota, ScreenShotA)
{
    const uint16_t bg565 = bg(ui);
    const auto body = bodyInset(ui, 12, 10, 8);
    ui.clear(bg565);
    drawHeader(ui, "Shots A", "drawScreenshot() 2x2 grid + screenshotCount()", accent(ui));
    drawPanel(ui, body.x, body.y, body.w, body.h - 34, panel(ui), line(ui), 12);
    ui.drawScreenshot().pos(body.x + 8, body.y + 8).size(body.w - 16, body.h - 50).grid(2, 2).padding(4);
    ui.drawText().font(WixMadeForDisplay, 12).weight(Semibold).text(String("count=") + ui.screenshotCount() + " mode=" + screenshotModeLabel(PIPGUI_SCREENSHOT_MODE)).pos(center, body.y + body.h - 22).color(fg(ui)).bgColor(bg565).align(Center);
    drawFooter(ui, "NEXT capture, PREV hold back");
}
