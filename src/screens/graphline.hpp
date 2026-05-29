SCREEN(graphline, ScreenGraphLine)
{
    const uint16_t bg565 = bg(ui);
    static int16_t samples[4][64];
    const float t = static_cast<float>(millis() % 7000u) * 0.006f;
    for (uint8_t i = 0; i < 4; ++i)
    {
        for (uint8_t j = 0; j < 64; ++j)
        {
            const float p = t * (0.8f + i * 0.35f) + static_cast<float>(j) * 0.12f + i;
            samples[i][j] = static_cast<int16_t>(sinf(p) * (90 - i * 12));
        }
    }

    ui.clear(bg565);
    drawHeader(ui, "Graph line", "draw/updateGraphLine(): line index, range, color, thickness", success(ui));

    ui.updateGraphGrid().pos(18, 64).size(204, 136).radius(14).direction(LeftToRight).bgColor(panel(ui)).speed(1.0f);
    for (uint8_t i = 0; i < 4; ++i)
    {
        const uint16_t colors[4] = {accent(ui), accent2(ui), warn(ui), danger(ui)};
        ui.updateGraphSamples().line(i).samples(samples[i], 64).color(colors[i]).range(-110, 110).thickness(1 + (i & 1u));
    }
}
