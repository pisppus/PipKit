SCREEN(graphup, ScreenGraphUpdate)
{
    const uint16_t bg565 = bg(ui);
    static int16_t samplesA[48];
    static int16_t samplesB[48];
    static int16_t samplesC[48];
    const float t = static_cast<float>(millis() % 10000u) * 0.00628f;
    for (uint8_t i = 0; i < 48; ++i)
    {
        const float p = t + static_cast<float>(i) * 0.18f;
        samplesA[i] = static_cast<int16_t>(sinf(p) * 90.0f);
        samplesB[i] = static_cast<int16_t>(sinf(p * 1.7f + 1.0f) * 70.0f);
        samplesC[i] = static_cast<int16_t>(sinf(p * 0.43f + 2.0f) * 100.0f);
    }

    ui.clear(bg565);
    drawHeader(ui, "Graph update", "updateGraphGrid() live traces with direction/speed variants", accent2(ui));
    ui.updateGraphGrid().pos(16, 60).size(208, 78).radius(12).direction(LeftToRight).bgColor(panel(ui)).speed(1.2f);
    ui.updateGraphSamples().line(0).samples(samplesA, 48).color(accent(ui)).range(-110, 110).thickness(2);
    ui.updateGraphSamples().line(1).samples(samplesB, 48).color(warn(ui)).range(-110, 110).thickness(1);
    ui.updateGraphGrid().pos(16, 162).size(208, 58).radius(8).direction(RightToLeft).bgColor(panel(ui)).speed(2.0f).scale(true);
    ui.updateGraphSamples().line(0).samples(samplesC, 48).color(danger(ui)).range(-110, 110).thickness(2);
}
