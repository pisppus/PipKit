SCREEN(tilea, ScreenTileA)
{
    (void)ui.tileNav();
    ui.updateTile()
        .items(
            tileItem(checkmark, "Glow", "2 cols", ScreenGlow),
            tileItem(arrow, "Dots", "spacing", ScreenDots),
            tileItem(warning, "Blur", "radius", ScreenBlur),
            tileItem(error, "Error", "overlay", ScreenErrCrash))
        .inactive(GUI::rgb888(10, 14, 18))
        .active(GUI::rgb888(22, 82, 150))
        .radius(14)
        .spacing(10)
        .columns(2)
        .tileSize(100, 64)
        .mode(TextSubtitle);
}
