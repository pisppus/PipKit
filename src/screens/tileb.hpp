SCREEN(tileb, ScreenTileB)
{
    (void)ui.tileNav();
    ui.updateTile()
        .items(
            tileItem(checkmark, "A", "", ScreenTileA),
            tileItem(arrow, "B", "", ScreenTileB),
            tileItem(warning, "C", "", ScreenTileCustom),
            tileItem(error, "D", "", ScreenMenu),
            tileItem(checkmark, "E", "", ScreenGraphDraw),
            tileItem(arrow, "F", "", ScreenProgress))
        .inactive(GUI::rgb888(18, 18, 18))
        .active(GUI::rgb888(168, 92, 24))
        .radius(8)
        .spacing(6)
        .columns(3)
        .tileSize(66, 54)
        .mode(TextOnly);
}
