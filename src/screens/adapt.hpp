SCREEN(adapt, ScreenAdapt)
{
    const uint16_t bg565 = bg(ui);
    const uint16_t panel565 = panel(ui);
    const uint16_t accent565 = accent2(ui);
    const uint16_t text565 = fg(ui);
    const uint16_t muted565 = muted(ui);
    const auto body = bodyInset(ui, 12, 10, 8);

    ui.clear(bg565);
    drawHeader(ui, "Adaptive preview", "Enter: ui.setAdaptivePreview(150, 108, 5200)  Exit: clearAdaptivePreview()", accent565);
    drawPanel(ui, body.x, body.y, body.w, body.h, panel565, accent565, 14);

    ui.drawText()
        .font(WixMadeForDisplay, 12)
        .weight(Medium)
        .text(String("Logical size: ") + ui.screenWidth() + "x" + ui.screenHeight())
        .pos(center, body.y + 16)
        .color(text565)
        .bgColor(panel565)
        .align(Center);

    ui.drawText()
        .font(WixMadeForDisplay, 10)
        .weight(Medium)
        .text("This page is intentionally simple: only built-in scaling should change")
        .pos(center, body.y + 30)
        .color(muted565)
        .bgColor(panel565)
        .align(Center);

    const UiRect stage{int16_t(body.x + 12), int16_t(body.y + 48), int16_t(body.w - 24), int16_t(body.h - 62)};
    drawPanel(ui, stage.x, stage.y, stage.w, stage.h, panelAlt(ui), line(ui), 12);

    ui.drawRect()
        .pos(stage.x + 10, stage.y + 10)
        .size(stage.w - 20, stage.h - 20)
        .radius(12)
        .border(1, line(ui));

    ui.drawRect()
        .pos(stage.x + 22, stage.y + 22)
        .size(stage.w - 44, 28)
        .radius(10)
        .fill(accent(ui));

    ui.drawRect()
        .pos(stage.x + 22, stage.y + 62)
        .size(stage.w - 44, 18)
        .radius(9)
        .fill(accent2(ui));

    ui.drawRect()
        .pos(stage.x + 22, stage.y + 92)
        .size(stage.w - 44, 18)
        .radius(9)
        .fill(warn(ui));

    ui.drawRect()
        .pos(stage.x + 22, stage.y + 122)
        .size(stage.w - 44, 36)
        .radius(12)
        .border(1, line(ui));

    ui.drawCircle()
        .pos(stage.x + 44, stage.y + stage.h - 32)
        .radius(12)
        .fill(accent(ui));

    ui.drawCircle()
        .pos(stage.x + stage.w - 44, stage.y + stage.h - 32)
        .radius(12)
        .border(1, danger(ui));

    ui.drawText()
        .font(KronaOne, 18)
        .weight(Regular)
        .text("Preview")
        .pos(center, stage.y + 142)
        .color(text565)
        .bgColor(panelAlt(ui))
        .align(Center);

    ui.drawText()
        .font(WixMadeForDisplay, 11)
        .weight(Medium)
        .text("Check how the entire page breathes in and out via the built-in adaptive path")
        .pos(center, stage.y + 176)
        .color(muted565)
        .bgColor(panelAlt(ui))
        .align(Center);

    drawFooter(ui, "PREV back");
}
