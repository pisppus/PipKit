SCREEN(graphdraw, ScreenGraphDraw)
{
    const uint16_t bg565 = bg(ui);
    static const int16_t samplesA[] = {-70, -34, -8, 28, 62, 84, 70, 24, -18, -52, -78, -44, 2, 48, 76, 88, 42, -12, -56, -84};
    static const int16_t samplesB[] = {44, 50, 38, 12, -24, -52, -64, -28, 8, 46, 60, 34, -4, -40, -58, -44, -8, 26, 52, 48};
    static const int16_t samplesC[] = {-96, -74, -48, -18, 10, 32, 58, 88, 70, 42, 20, -6, -34, -66, -86, -54, -20, 18, 52, 92};
    ui.clear(bg565);
    drawHeader(ui, "Graph draw", "drawGraphGrid(): directions, size, speed, colors, line options", accent(ui));

    ui.drawGraphGrid().pos(16, 58).size(96, 64).radius(10).direction(LeftToRight).bgColor(panel(ui)).speed(0.6f);
    ui.drawGraphSamples().line(0).samples(samplesA, sizeof(samplesA) / sizeof(samplesA[0])).color(accent(ui)).range(-100, 100).thickness(1);
    ui.drawGraphSamples().line(1).samples(samplesB, sizeof(samplesB) / sizeof(samplesB[0])).color(warn(ui)).range(-100, 100).thickness(2);

    ui.drawGraphGrid().pos(128, 58).size(96, 64).radius(4).direction(RightToLeft).bgColor(panel(ui)).speed(1.4f).scale(true);
    ui.drawGraphSamples().line(0).samples(samplesC, sizeof(samplesC) / sizeof(samplesC[0])).color(accent2(ui)).range(-100, 100).thickness(2);

    ui.drawGraphGrid().pos(24, 150).size(190, 62).radius(14).direction(Oscilloscope).bgColor(panel(ui)).speed(1.0f).scope(120, 700).visible(80);
    ui.drawGraphSamples().line(0).samples(samplesA, sizeof(samplesA) / sizeof(samplesA[0])).color(danger(ui)).range(-100, 100).thickness(3);
    drawFooter(ui, "PREV back");
}
