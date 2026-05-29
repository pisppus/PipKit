SCREEN(ota, ScreenOta)
{
    const uint16_t bg565 = bg(ui);
    const auto body = bodyInset(ui, 12, 10, 8);
    const OtaStatus &st = ui.otaStatus();
    const char *state = "Unknown";
    if (st.state == OtaState::Idle) state = "Idle";
    else if (st.state == OtaState::WifiStarting) state = "WiFi";
    else if (st.state == OtaState::FetchingManifest) state = "Manifest";
    else if (st.state == OtaState::UpdateAvailable) state = "Available";
    else if (st.state == OtaState::Downloading) state = "Downloading";
    else if (st.state == OtaState::Installing) state = "Installing";
    else if (st.state == OtaState::Success) state = "Success";
    else if (st.state == OtaState::Error) state = "Error";
    else if (st.state == OtaState::UpToDate) state = "UpToDate";

    const uint8_t pct = (st.total > 0u) ? static_cast<uint8_t>((st.downloaded * 100u) / st.total) : 0u;

    ui.clear(bg565);
    drawHeader(ui, "OTA", "NEXT cycles check/downgrade/stable-list/cancel", warn(ui));
    drawPanel(ui, body.x, body.y, body.w, body.h, panel(ui), (st.state == OtaState::Error) ? danger(ui) : line(ui), 14);
    ui.drawText().font(WixMadeForDisplay, 13).weight(Semibold).text(String("State: ") + state).pos(body.x + 14, body.y + 28).color(fg(ui)).bgColor(panel(ui));
    ui.drawText().font(WixMadeForDisplay, 11).weight(Medium).text(String("Version: ") + st.manifest.version).pos(body.x + 14, body.y + 58).color(muted(ui)).bgColor(panel(ui));
    ui.drawText().font(WixMadeForDisplay, 11).weight(Medium).text(String("Stable list: ") + (ui.otaStableListReady() ? "ready " : "not ready ") + ui.otaStableListCount()).pos(body.x + 14, body.y + 84).color(muted(ui)).bgColor(panel(ui));
    ui.updateProgress().pos(body.x + 14, body.y + 116).size(body.w - 28, 14).value(pct).baseColor(panelAlt(ui)).fillColor(accent2(ui)).radius(7).percent(Right).percentColor(fg(ui));
    ui.drawText().font(WixMadeForDisplay, 11).weight(Medium).text(String("Error code: ") + static_cast<unsigned>(st.error)).pos(body.x + 14, body.y + 150).color((st.state == OtaState::Error) ? danger(ui) : muted(ui)).bgColor(panel(ui));
    drawFooter(ui, "Requires PIPGUI_OTA + project URL/key");
}
