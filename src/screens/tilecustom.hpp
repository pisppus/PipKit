SCREEN(tilecustom, ScreenTileCustom)
{
    (void)ui.tileNav();
    ui.updateTile()
        .grid(3, 3)
        .tile(checkmark, "Wide", "span 2x1", ScreenTileA).at(0, 0).span(2, 1)
        .tile(warning, "Tall", "1x2", ScreenTileB).at(2, 0).span(1, 2)
        .tile(arrow, "Small", "", ScreenGraphDraw).at(0, 1)
        .tile(error, "Mid", "2x1", ScreenErrCrash).at(0, 2).span(2, 1)
        .tile(checkmark, "One", "", ScreenMenu).at(2, 2)
        .inactive(GUI::rgb888(12, 18, 22))
        .active(GUI::rgb888(32, 120, 92))
        .radius(13)
        .spacing(8)
        .mode(TextSubtitle);
}
