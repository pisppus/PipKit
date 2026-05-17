#pragma once

#include <cstdint>

#include <PipGUI/Core/Config/Defaults.hpp>

namespace pipgui::config
{
    struct FirmwareVersion
    {
        uint16_t major = 0;
        uint16_t minor = 0;
        uint16_t patch = 0;
    };

    [[nodiscard]] constexpr const char *firmwareVersionText() noexcept
    {
        return PIPGUI_FIRMWARE_VERSION;
    }

    [[nodiscard]] constexpr FirmwareVersion parseFirmwareVersion(const char *text) noexcept
    {
        FirmwareVersion out{};
        uint16_t *part = &out.major;
        uint8_t index = 0;

        if (!text)
            return out;

        for (const char *p = text; *p; ++p)
        {
            const char ch = *p;
            if (ch == '.')
            {
                if (index >= 2)
                    break;
                ++index;
                part = (index == 1) ? &out.minor : &out.patch;
                continue;
            }
            if (ch < '0' || ch > '9')
                break;

            const uint32_t next = static_cast<uint32_t>(*part) * 10u + static_cast<uint32_t>(ch - '0');
            *part = (next > 65535u) ? 65535u : static_cast<uint16_t>(next);
        }

        return out;
    }

    [[nodiscard]] constexpr FirmwareVersion firmwareVersion() noexcept
    {
        return parseFirmwareVersion(firmwareVersionText());
    }

    [[nodiscard]] constexpr uint64_t firmwareBuildNumber() noexcept
    {
        const FirmwareVersion v = firmwareVersion();
        return (static_cast<uint64_t>(v.major) * 1000000ull) +
               (static_cast<uint64_t>(v.minor) * 1000ull) +
               static_cast<uint64_t>(v.patch);
    }
}
