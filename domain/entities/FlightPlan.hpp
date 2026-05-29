#pragma once

#include <string>
#include <vector>

namespace cwp::domain {

/**
 * @brief Domain entity representing a filed ATC flight plan.
 *
 * Carries all data needed to identify and manage a flight: callsign,
 * aircraft type, SSR squawk code, assigned flight level, and route.
 * The assigned level may be amended at any time by an ATCo.
 */
class FlightPlan {
public:
    /**
     * @brief Constructs a FlightPlan entity.
     * @param callsign      ICAO callsign (e.g. "DLH123").
     * @param aircraftType  ICAO aircraft type designator (e.g. "A320").
     * @param squawk        SSR squawk code, 4 octal digits (e.g. "1234").
     * @param assignedLevel Assigned flight level in hundreds of feet (e.g. 350 = FL350).
     * @param route         Ordered list of route waypoints/fixes.
     */
    FlightPlan(std::string callsign,
               std::string aircraftType,
               std::string squawk,
               int assignedLevel,
               std::vector<std::string> route);

    /// @return ICAO callsign.
    [[nodiscard]] const std::string&              callsign()      const noexcept;

    /// @return ICAO aircraft type designator.
    [[nodiscard]] const std::string&              aircraftType()  const noexcept;

    /// @return SSR squawk code (4 octal digits).
    [[nodiscard]] const std::string&              squawk()        const noexcept;

    /// @return Assigned flight level (e.g. 350 = FL350).
    [[nodiscard]] int                             assignedLevel() const noexcept;

    /// @return Ordered list of route waypoints.
    [[nodiscard]] const std::vector<std::string>& route()         const noexcept;

    /**
     * @brief Amends the assigned flight level.
     * @param level New flight level in hundreds of feet (e.g. 390 = FL390).
     */
    void amendLevel(int level);

private:
    std::string              m_callsign;
    std::string              m_aircraftType;
    std::string              m_squawk;
    int                      m_assignedLevel{0};
    std::vector<std::string> m_route;
};

} // namespace cwp::domain
