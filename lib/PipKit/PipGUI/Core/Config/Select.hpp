#pragma once

#include <PipCore/Config/Features.hpp>

#if PIPCORE_TARGET_DESKTOP && __has_include(<config_sim.hpp>)
#include <config_sim.hpp>
#elif __has_include(<config.hpp>)
#include <config.hpp>
#endif

#include <PipGUI/Core/Config/Defaults.hpp>
