#include "domain/entities/FlightPlan.hpp"

#include <utility>

namespace cwp::domain {

FlightPlan::FlightPlan(std::string callsign,
                       std::string aircraftType,
                       std::string squawk,
                       int assignedLevel,
                       std::vector<std::string> route)
    : m_callsign{std::move(callsign)}
    , m_aircraftType{std::move(aircraftType)}
    , m_squawk{std::move(squawk)}
    , m_assignedLevel{assignedLevel}
    , m_route{std::move(route)}
{}

const std::string& FlightPlan::callsign() const noexcept
{
    return m_callsign;
}

const std::string& FlightPlan::aircraftType() const noexcept
{
    return m_aircraftType;
}

const std::string& FlightPlan::squawk() const noexcept
{
    return m_squawk;
}

int FlightPlan::assignedLevel() const noexcept
{
    return m_assignedLevel;
}

const std::vector<std::string>& FlightPlan::route() const noexcept
{
    return m_route;
}

void FlightPlan::amendLevel(int level)
{
    m_assignedLevel = level;
}

} // namespace cwp::domain
