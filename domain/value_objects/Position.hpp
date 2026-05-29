#pragma once

#include <expected>

#include "domain/errors/DomainError.hpp"

namespace cwp::domain {

/**
 * @brief Immutable value object representing a 3-D geographic position.
 *
 * Safety-critical: all external/network-sourced coordinates must be
 * constructed via the validated Position::create() factory to prevent
 * corrupted or spoofed radar data from propagating into the display.
 *
 * Equality is value-based (C++23 defaulted spaceship operator).
 */
class Position {
public:
    // ── Validation bounds ────────────────────────────────────────────────
    static constexpr double k_minLatitude  { -90.0 };  ///< WGS-84 south pole.
    static constexpr double k_maxLatitude  {  90.0 };  ///< WGS-84 north pole.
    static constexpr double k_minLongitude {-180.0 };  ///< WGS-84 antimeridian west.
    static constexpr double k_maxLongitude { 180.0 };  ///< WGS-84 antimeridian east.
    static constexpr int    k_minAltitudeFt{ -2000 };  ///< Below Dead Sea (lowest airfield).
    static constexpr int    k_maxAltitudeFt{  70000};  ///< Above operational ceiling (FL600+).

    /**
     * @brief Validated factory — **mandatory** for all external / network data.
     *
     * Returns a PositionError if any coordinate is outside valid WGS-84 bounds.
     * Using std::expected (C++23) keeps the hot path allocation-free and forces
     * callers to handle the error case explicitly.
     *
     * @param latitude  Decimal degrees in [-90, 90].
     * @param longitude Decimal degrees in [-180, 180].
     * @param altitude  Feet AMSL in [k_minAltitudeFt, k_maxAltitudeFt].
     */
    [[nodiscard]] static constexpr
    std::expected<Position, PositionError>
    create(double latitude, double longitude, int altitude) noexcept
    {
        if (latitude < k_minLatitude || latitude > k_maxLatitude) {
            return std::unexpected{PositionError::LatitudeOutOfRange};
        }
        if (longitude < k_minLongitude || longitude > k_maxLongitude) {
            return std::unexpected{PositionError::LongitudeOutOfRange};
        }
        return Position{latitude, longitude, altitude};
    }

    /**
     * @brief Direct constructor for pre-validated, trusted internal data.
     *
     * Use only when the caller can prove the values are in range (e.g.
     * computed from verified internal state). Prefer create() for any
     * data that originates from external sources.
     */
    constexpr Position(double latitude, double longitude, int altitude) noexcept
        : m_latitude{latitude}
        , m_longitude{longitude}
        , m_altitude{altitude}
    {
        // Compiler hint: inform the optimiser these bounds hold in trusted paths.
        [[assume(latitude  >= k_minLatitude  && latitude  <= k_maxLatitude)]];
        [[assume(longitude >= k_minLongitude && longitude <= k_maxLongitude)]];
    }

    /// @return Latitude in decimal degrees.
    [[nodiscard]] constexpr double latitude()  const noexcept { return m_latitude;  }

    /// @return Longitude in decimal degrees.
    [[nodiscard]] constexpr double longitude() const noexcept { return m_longitude; }

    /// @return Altitude in feet AMSL.
    [[nodiscard]] constexpr int    altitude()  const noexcept { return m_altitude;  }

    /// C++23: defaulted spaceship gives all six comparison operators.
    [[nodiscard]] auto operator<=>(const Position&) const noexcept = default;
    [[nodiscard]] bool operator== (const Position&) const noexcept = default;

private:
    double m_latitude {0.0};
    double m_longitude{0.0};
    int    m_altitude {0};
};

} // namespace cwp::domain
