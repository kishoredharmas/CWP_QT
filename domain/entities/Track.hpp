#pragma once

#include <chrono>
#include <compare>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>

#include "domain/value_objects/Position.hpp"
#include "domain/value_objects/Velocity.hpp"

namespace cwp::domain {

/**
 * @brief Strong identifier type for a radar track.
 *
 * Wraps a raw uint32_t so it cannot be accidentally passed where an altitude,
 * squawk code, or other integer is expected — a common source of bugs in
 * safety-critical ATC systems. Supports the full set of comparison operators
 * via C++23 defaulted spaceship.
 */
struct TrackId {
    std::uint32_t value{0};

    constexpr explicit TrackId(std::uint32_t v) noexcept : value{v} {}

    /// C++23: defaulted spaceship gives all six comparison operators.
    [[nodiscard]] auto operator<=>(const TrackId&) const noexcept = default;
    [[nodiscard]] bool operator== (const TrackId&) const noexcept = default;
};

/// Configurable freshness timeout: tracks not updated within this window
/// are considered stale and must not be displayed to controllers.
static constexpr std::chrono::seconds k_staleTrackTimeout{60};

/**
 * @brief Domain entity representing a radar track (aircraft return).
 *
 * Encapsulates the current kinematic state of a tracked aircraft as reported
 * by the surveillance radar. A track may optionally be correlated with an ATC
 * flight plan, in which case it carries the associated callsign.
 */
class Track {
public:
    /**
     * @brief Constructs a Track entity.
     * @param id        Unique track identifier assigned by the radar system.
     * @param position  Current 3-D position (lat/lon/altitude).
     * @param velocity  Current velocity vector (speed/heading/vertical-rate).
     * @param timestamp UTC time of the latest radar return.
     */
    Track(TrackId id,
          Position position,
          Velocity velocity,
          std::chrono::system_clock::time_point timestamp);

    /// @return Unique track identifier.
    [[nodiscard]] TrackId id() const noexcept;

    /// @return Current geographic position.
    [[nodiscard]] const Position& position() const noexcept;

    /// @return Current velocity vector.
    [[nodiscard]] const Velocity& velocity() const noexcept;

    /// @return UTC timestamp of the last radar return.
    [[nodiscard]] std::chrono::system_clock::time_point timestamp() const noexcept;

    /// @return Associated ATC callsign, or std::nullopt if uncorrelated.
    [[nodiscard]] const std::optional<std::string>& callsign() const noexcept;

    /**
     * @brief Associates a flight-plan callsign with this track.
     * @param callsign ICAO callsign string (e.g. "DLH123").
     */
    void associateCallsign(std::string callsign);

    /**
     * @brief Updates the track with data from a new radar return.
     * @param position  Updated 3-D position.
     * @param velocity  Updated velocity vector.
     * @param timestamp UTC time of this new return.
     */
    void update(Position position,
                Velocity velocity,
                std::chrono::system_clock::time_point timestamp);

    /**
     * @brief Returns the elapsed time since the last radar return.
     *
     * Thread-safe: reads the immutable timestamp captured at the last update.
     *
     * @param now  Current UTC time, typically std::chrono::system_clock::now().
     * @return Non-negative duration since the last update.
     */
    [[nodiscard]] std::chrono::seconds
    age(std::chrono::system_clock::time_point now) const noexcept;

    /**
     * @brief Safety check: true if this track has not been updated recently.
     *
     * A stale track must not be presented to controllers as it may represent
     * an aircraft whose position is dangerously out of date.
     *
     * @param threshold  Maximum acceptable age (default: k_staleTrackTimeout).
     * @param now        Current UTC time.
     */
    [[nodiscard]] bool isStale(
        std::chrono::seconds              threshold = k_staleTrackTimeout,
        std::chrono::system_clock::time_point now  = std::chrono::system_clock::now()
    ) const noexcept;

private:
    TrackId                               m_id;
    Position                              m_position;
    Velocity                              m_velocity;
    std::chrono::system_clock::time_point m_timestamp;
    std::optional<std::string>            m_callsign;
};

} // namespace cwp::domain

// ── std::hash specialisation ────────────────────────────────────────────────
template<>
struct std::hash<cwp::domain::TrackId> {
    [[nodiscard]] std::size_t
    operator()(cwp::domain::TrackId id) const noexcept
    {
        return std::hash<std::uint32_t>{}(id.value);
    }
};
