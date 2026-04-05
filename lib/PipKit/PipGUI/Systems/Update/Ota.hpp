#pragma once

#include <PipGUI/Core/Types.hpp>
#include <PipCore/Update/Ota.hpp>

namespace pipgui
{
    using OtaChannel = pipcore::ota::Channel;
    using OtaCheckMode = pipcore::ota::CheckMode;
    using OtaState = pipcore::ota::State;
    using OtaError = pipcore::ota::Error;
    using OtaManifest = pipcore::ota::Manifest;
    using OtaStatus = pipcore::ota::Status;
    using OtaStatusCallback = pipcore::ota::StatusCallback;

    inline constexpr OtaCheckMode NewerOnly = OtaCheckMode::NewerOnly;
    inline constexpr OtaCheckMode AllowDowngrade = OtaCheckMode::AllowDowngrade;
}

