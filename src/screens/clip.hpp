SCREEN(clip, ScreenClip)
{
    const uint16_t bg565 = bg(ui);
    const uint16_t panel565 = panel(ui);
    const uint16_t warn565 = warn(ui);
    const uint16_t danger565 = danger(ui);
    const uint16_t text565 = fg(ui);
    const uint16_t muted565 = muted(ui);
    const auto body = bodyInset(ui, 12, 10, 8);
    const auto left = splitLeft(body, 10);
    const auto right = splitRight(body, 10);

    ui.clear(bg565);
    drawHeader(ui, "setClip()", "Left: text and shape overflow clipped, right: raw Warning/Error colors", warn565);

    drawPanel(ui, left.x, left.y, left.w, left.h, panel565, warn565, 12);
    drawPanel(ui, right.x, right.y, right.w, right.h, panel565, danger565, 12);

    ui.drawText()
        .font(WixMadeForDisplay, 14)
        .weight(Semibold)
        .text("Clip test")
        .pos(left.x + left.w / 2, left.y + 12)
        .color(warn565)
        .bgColor(panel565)
        .align(Center);

    ui.drawText()
        .font(WixMadeForDisplay, 14)
        .weight(Semibold)
        .text("Color swatches")
        .pos(right.x + right.w / 2, right.y + 12)
        .color(danger565)
        .bgColor(panel565)
        .align(Center);

    const UiRect clipWarn{int16_t(left.x + 10), int16_t(left.y + 30), int16_t(left.w - 20), int16_t(left.h - 44)};
    const UiRect clipErr{int16_t(right.x + 10), int16_t(right.y + 30), int16_t(right.w - 20), int16_t(right.h - 44)};

    ui.drawRect()
        .pos(clipWarn.x, clipWarn.y)
        .size(clipWarn.w, clipWarn.h)
        .radius(10)
        .border(1, line(ui));

    ui.setClip().pos(clipWarn.x, clipWarn.y).size(clipWarn.w, clipWarn.h);

    ui.drawRect()
        .pos(clipWarn.x - 22, clipWarn.y + 16)
        .size(clipWarn.w + 48, 18)
        .radius(9)
        .fill(accent(ui));

    ui.drawRect()
        .pos(clipWarn.x + 14, clipWarn.y + clipWarn.h - 30)
        .size(clipWarn.w + 10, 20)
        .radius(10)
        .fill(warn565);

    ui.drawCircle()
        .pos(clipWarn.x + clipWarn.w - 4, clipWarn.y + clipWarn.h / 2)
        .radius(28)
        .fill(danger565);

    ui.drawText()
        .font(KronaOne, 18)
        .weight(Regular)
        .text("Text starts before clip")
        .pos(clipWarn.x - 24, clipWarn.y + 62)
        .color(text565)
        .bgColor(panel565);

    ui.drawText()
        .font(WixMadeForDisplay, 11)
        .weight(Medium)
        .text("Anything beyond the frame must be cut off")
        .pos(clipWarn.x - 10, clipWarn.y + clipWarn.h - 18)
        .color(muted565)
        .bgColor(panel565);

    ui.clearClip();

    const int16_t swatchW = clipErr.w - 20;

    ui.drawRect()
        .pos(clipErr.x + 10, clipErr.y + 12)
        .size(swatchW, 32)
        .radius(10)
        .fill(warn565);

    ui.drawText()
        .font(WixMadeForDisplay, 12)
        .weight(Semibold)
        .text("Warning")
        .pos(clipErr.x + 18, clipErr.y + 22)
        .color(panel565)
        .bgColor(warn565);

    ui.drawRect()
        .pos(clipErr.x + 10, clipErr.y + 60)
        .size(swatchW, 32)
        .radius(10)
        .fill(danger565);

    ui.drawText()
        .font(WixMadeForDisplay, 12)
        .weight(Semibold)
        .text("Error")
        .pos(clipErr.x + 18, clipErr.y + 70)
        .color(text565)
        .bgColor(danger565);

    ui.drawCircle()
        .pos(clipErr.x + 34, clipErr.y + 124)
        .radius(14)
        .fill(warn565);

    ui.drawCircle()
        .pos(clipErr.x + 74, clipErr.y + 124)
        .radius(14)
        .fill(danger565);

    ui.drawText()
        .font(WixMadeForDisplay, 11)
        .weight(Medium)
        .text("Raw fills only: no clip, no blend tricks")
        .pos(clipErr.x + 10, clipErr.y + clipErr.h - 18)
        .color(muted565)
        .bgColor(panel565);

    drawFooter(ui, "PREV back");
}
