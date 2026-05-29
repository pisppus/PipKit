SCREEN(blurdup, ScreenBlurDUp)
{
    const uint16_t bg565 = bg(ui);
    const auto body = bodyInset(ui, 12, 10, 8);
    const float t = static_cast<float>(millis() % 6000u) * 0.0035f;

    ui.clear(bg565);
    drawHeader(ui, "BlurD update", "updateBlur() directional over moving underlay", warn(ui));

    for (uint8_t i = 0; i < 6; ++i)
    {
        const int16_t x0 = body.x + static_cast<int16_t>((sinf(t + i) + 1.0f) * 95.0f);
        ui.drawLine().from(x0, body.y + 24).to(body.x + body.w - x0 / 3, body.y + 188).thickness(5).color((i & 1u) ? accent(ui) : danger(ui));
    }

    ui.updateBlur().pos(body.x + 10, body.y + 24).size(92, 46).radius(10).direction(TopDown).material(128, -1);
    ui.updateBlur().pos(body.x + 128, body.y + 24).size(92, 46).radius(10).direction(BottomUp).material(128, -1);
    ui.updateBlur().pos(body.x + 10, body.y + 104).size(92, 46).radius(10).direction(LeftRight).material(128, -1);
    ui.updateBlur().pos(body.x + 128, body.y + 104).size(92, 46).radius(10).direction(RightLeft).material(128, -1);
    drawFooter(ui, "Directional masks: top/bottom/left/right");
}
