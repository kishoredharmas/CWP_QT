#pragma once

#include <cstdint>

namespace cwp::domain {

// ── Position validation errors ───────────────────────────────────────────────

/// Error codes returned by Position::create() when input is out of range.
enum class PositionError : std::uint8_t {
    LatitudeOutOfRange,   ///< Latitude is not in the valid range [-90, 90].
    LongitudeOutOfRange,  ///< Longitude is not in the valid range [-180, 180].
};

// ── Velocity validation errors ───────────────────────────────────────────────

/// Error codes returned by Velocity::create() when input is out of range.
enum class VelocityError : std::uint8_t {
    NegativeSpeed,        ///< Ground speed must be >= 0.
    HeadingOutOfRange,    ///< Heading must be in [0, 360).
};

// ── Association errors ───────────────────────────────────────────────────────

/// Error codes returned by AssociateFlightPlanUseCase::execute().
enum class AssociationError : std::uint8_t {
    TrackNotFound,        ///< No track with the requested TrackId exists.
};

// ── Playback / recording errors ──────────────────────────────────────────────

/// Error codes returned by IPlaybackService start operations.
enum class PlaybackError : std::uint8_t {
    FileNotAccessible,    ///< File cannot be opened for reading or writing.
    AlreadyActive,        ///< A recording or playback session is already running.
};

} // namespace cwp::domain
