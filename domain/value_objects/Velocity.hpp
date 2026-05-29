#pragma once

#include <expected>

#include "domain/errors/DomainError.hpp"

namespace cwp::domain {

/**
 * @brief Immutable value object representing an aircraft velocity vector.
 *
 * Safety-critical: validated construction prevents physically impossible
 * velocity vectors (e.g. negative speed, heading >= 360°) from reaching
 * the display or any downstream safety logic.
 *
 * Speed is in knots, heading in true degrees [0, 360), vertical rate
 * in feet per minute (positive = climbing).
 */
class Velocity {
public:
    // ── Validation bounds ────────────────────────────────────────────────
    static constexpr double k_minSpeed  {   0.0 };  ///< Aircraft cannot have negative speed.
    static constexpr double k_maxSpeed  {3000.0 };  ///< Above any known aircraft (hypersonic margin).
    static constexpr double k_minHeading{   0.0 };  ///< True north.
    static constexpr double k_maxHeading{ 360.0 };  ///< Exclusive upper bound.

    /**
     * @brief Validated factory — **mandatory** for all external / network data.
     *
     * @param speed        Ground speed in knots [0, 3000).
     * @param heading      True heading in degrees [0, 360).
     * @param verticalRate Vertical rate in feet per minute (unconstrained).
     */
    [[nodiscard]] static constexpr
    std::expected<Velocity, VelocityError>
    create(double speed, double heading, int verticalRate) noexcept
    {
        if (speed < k_minSpeed || speed > k_maxSpeed) {
            return std::unexpected{VelocityError::NegativeSpeed};
        }
        if (heading < k_minHeading || heading >= k_maxHeading) {
            return std::unexpected{VelocityError::HeadingOutOfRange};
        }
        return Velocity{speed, heading, verticalRate};
    }

    /**
     * @brief Direct constructor for pre-validated, trusted internal data.
     *
     * Prefer create() for any data originating from external sources.
     */
    constexpr Velocity(double speed, double heading, int verticalRate) noexcept
        : m_speed{speed}
        , m_heading{heading}
        , m_verticalRate{verticalRate}
    {
        [[assume(speed   >= k_minSpeed   && speed   <= k_maxSpeed)]];
        [[assume(heading >= k_minHeading && heading <  k_maxHeading)]];
    }

    /// @return Ground speed in knots.
    [[nodiscard]] constexpr double speed()        const noexcept { return m_speed;        }

    /// @return True heading in degrees [0, 360).
    [[nodiscard]] constexpr double heading()      const noexcept { return m_heading;      }

    /// @return Vertical rate in feet per minute.
    [[nodiscard]] constexpr int    verticalRate() const noexcept { return m_verticalRate; }

    /// C++23: defaulted spaceship gives all six comparison operators.
    [[nodiscard]] auto operator<=>(const Velocity&) const noexcept = default;
    [[nodiscard]] bool operator== (const Velocity&) const noexcept = default;

private:
    double m_speed       {0.0};
    double m_heading     {0.0};
    int    m_verticalRate{0};
};

} // namespace cwp::domain
