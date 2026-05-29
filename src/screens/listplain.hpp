SCREEN(listplain, ScreenListPlain)
{
    const DemoState &st = runtimeState();
    ui.updateList()
        .items(
            listItem(checkmark, "Checked", "NEXT/PREV moves this mark", ScreenListPlain),
            listItem(arrow, "Runtime", "Main runtime knobs", ScreenDebug),
            listItem(warning, "Blur", "Static blur material", ScreenBlur),
            listItem(checkmark, "Glow", "Static glow variants", ScreenGlow),
            listItem(arrow, "Graph", "Graph draw checks", ScreenGraphDraw),
            listItem(error, "Error", "Error overlay check", ScreenErrCrash))
        .inactive(bg(ui))
        .active(accent2(ui))
        .mode(Plain)
        .checked(st.plainChecked);
}
