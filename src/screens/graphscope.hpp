SCREEN(graphscope, ScreenGraphScope)
{
    static int16_t samplesA[128];
    static int16_t samplesB[128];
    const uint16_t bg565 = bg(ui);
    const float base = static_cast<float>(millis() % 8000u) * 0.003f;

    for (uint8_t i = 0; i < 128; ++i)
    {
        const float x = base + static_cast<float>(i) * 0.08f;
        samplesA[i] = static_cast<int16_t>(sinf(x) * 80.0f);
        samplesB[i] = static_cast<int16_t>((sinf(x * 2.7f) * 0.55f + sinf(x * 0.7f) * 0.45f) * 95.0f);
    }

    ui.clear(bg565);
    drawHeader(ui, "Scope", "drawGraphSamples() full buffer oscilloscope path", warn(ui));
    ui.updateGraphGrid().pos(18, 64).size(204, 138).radius(12).direction(Oscilloscope).bgColor(panel(ui)).speed(1.0f).scope(320, 600).visible(128);
    ui.updateGraphSamples().line(0).samples(samplesA, 128).color(accent(ui)).range(-110, 110).thickness(2);
    ui.updateGraphSamples().line(1).samples(samplesB, 128).color(accent2(ui)).range(-110, 110).thickness(1);
}
