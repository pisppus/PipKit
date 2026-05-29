SCREEN(textbox, ScreenTextbox)
{
    const uint16_t bg565 = bg(ui);
    const uint16_t text565 = fg(ui);
    const uint16_t muted565 = muted(ui);
    const auto body = bodyInset(ui, 12, 10, 8);
    const int16_t gap = 10;
    const int16_t boxW = (body.w - gap) / 2;
    const int16_t boxH = (body.h - gap) / 2;
    const char *copy = "drawTextBox wraps words, clips overflow and respects lineGap.";

    ui.clear(bg565);
    drawHeader(ui, "Textbox", "Alignment, wrapping and custom line gaps", warn(ui));

    const UiRect boxes[4] = {
        {body.x, body.y, boxW, boxH},
        {int16_t(body.x + boxW + gap), body.y, boxW, boxH},
        {body.x, int16_t(body.y + boxH + gap), boxW, boxH},
        {int16_t(body.x + boxW + gap), int16_t(body.y + boxH + gap), boxW, boxH},
    };

    const uint16_t fills[4] = {panel(ui), panelAlt(ui), panel(ui), panelAlt(ui)};
    const TextAlign aligns[4] = {Left, Center, Right, Left};
    const int16_t gaps[4] = {-1, -1, -1, 6};
    const char *titles[4] = {"Left / auto", "Center / auto", "Right / auto", "Left / lineGap=6"};

    for (uint8_t i = 0; i < 4; ++i)
    {
        drawPanel(ui, boxes[i].x, boxes[i].y, boxes[i].w, boxes[i].h, fills[i], line(ui), 12);

        ui.drawText()
            .font(WixMadeForDisplay, 10)
            .weight(Semibold)
            .text(titles[i])
            .pos(boxes[i].x + 10, boxes[i].y + 8)
            .color(muted565)
            .bgColor(fills[i]);

        ui.drawTextBox()
            .font(WixMadeForDisplay, 12)
            .weight(Medium)
            .pos(boxes[i].x + 10, boxes[i].y + 24)
            .size(boxes[i].w - 20, boxes[i].h - 34)
            .text(copy)
            .color(text565)
            .bgColor(fills[i])
            .align(aligns[i])
            .lineGap(gaps[i]);
    }

    drawFooter(ui, "PREV back");
}
