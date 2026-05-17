#include <PipGUI/Core/Config/Select.hpp>
#include <PipGUI/Systems/Network/Wifi.hpp>

namespace pipgui::net
{
        namespace
        {
                inline void configureOnce() noexcept
                {
#if PIPGUI_WIFI
                        static bool configured = false;
                        if (configured)
                                return;
                        pipcore::net::WifiConfig cfg;
                        cfg.ssid = PIPGUI_WIFI_SSID;
                        cfg.password = PIPGUI_WIFI_PASSWORD;
                        pipcore::net::wifiConfigure(cfg);
                        configured = true;
#endif
                }
        }

        void wifiRequest(bool enabled) noexcept
        {
#if PIPGUI_WIFI
                if (enabled)
                        configureOnce();
                pipcore::net::wifiRequest(enabled);
#else
                (void)enabled;
#endif
        }

        void wifiService() noexcept
        {
#if PIPGUI_WIFI
                configureOnce();
                pipcore::net::wifiService();
#endif
        }

        WifiState wifiState() noexcept
        {
#if PIPGUI_WIFI
                return pipcore::net::wifiState();
#else
                return WifiState::Off;
#endif
        }

        bool wifiConnected() noexcept
        {
#if PIPGUI_WIFI
                return pipcore::net::wifiConnected();
#else
                return false;
#endif
        }

        uint32_t wifiLocalIpV4() noexcept
        {
#if PIPGUI_WIFI
                return pipcore::net::wifiLocalIpV4();
#else
                return 0;
#endif
        }
}
