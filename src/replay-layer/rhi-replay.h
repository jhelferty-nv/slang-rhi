#pragma once

/// @file rhi-replay.h
/// Private header exposing RHI replay registration.
/// The slang-replay tool includes this directly (slang-rhi is a submodule).

namespace rhi::replay {

/// Register RHI replay handlers with the Slang replay system.
/// Call this before executing replay to enable handling of RHI call signatures.
void registerHandlers();

} // namespace rhi::replay
