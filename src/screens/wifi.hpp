SCREEN(wifi, ScreenWifi)
{
    const uint16_t bg565 = bg(ui);
    const auto body = bodyInset(ui, 12, 10, 8);
    const pipcore::net::WifiState ws = ui.wifiState();
    const char *name = "Unknown";
    if (ws == pipcore::net::WifiState::Off) name = "Off";
    else if (ws == pipcore::net::WifiState::Connecting) name = "Connecting";
    else if (ws == pipcore::net::WifiState::Connected) name = "Connected";
    else if (ws == pipcore::net::WifiState::Failed) name = "Failed";
    else if (ws == pipcore::net::WifiState::Unsupported) name = "Unsupported";

    const uint32_t ip = ui.wifiLocalIpV4();
    char ipbuf[20];
    snprintf(ipbuf, sizeof(ipbuf), "%u.%u.%u.%u",
             static_cast<unsigned>((ip >> 24) & 0xFFu),
             static_cast<unsigned>((ip >> 16) & 0xFFu),
             static_cast<unsigned>((ip >> 8) & 0xFFu),
             static_cast<unsigned>(ip & 0xFFu));

    ui.clear(bg565);
    drawHeader(ui, "Wi-Fi", "NEXT toggles requestWiFi(); state/IP shown live", accent(ui));
    drawPanel(ui, body.x, body.y, body.w, body.h, panel(ui), ui.wifiConnected() ? accent(ui) : line(ui), 14);
    ui.drawText().font(WixMadeForDisplay, 14).weight(Semibold).text(String("State: ") + name).pos(body.x + 16, body.y + 42).color(fg(ui)).bgColor(panel(ui));
    ui.drawText().font(WixMadeForDisplay, 12).weight(Medium).text(String("Connected: ") + (ui.wifiConnected() ? "yes" : "no")).pos(body.x + 16, body.y + 78).color(ui.wifiConnected() ? accent(ui) : muted(ui)).bgColor(panel(ui));
    ui.drawText().font(WixMadeForDisplay, 12).weight(Medium).text(String("IP: ") + ((ip == 0u) ? "-" : ipbuf)).pos(body.x + 16, body.y + 112).color(muted(ui)).bgColor(panel(ui));
    drawFooter(ui, "Requires configured Wi-Fi backend");
}
