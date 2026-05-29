#include "domain/entities/Track.hpp"

#include <utility>

namespace cwp::domain {

Track::Track(TrackId id,
             Position position,
             Velocity velocity,
             std::chrono::system_clock::time_point timestamp)
    : m_id{id}
    , m_position{position}
    , m_velocity{velocity}
    , m_timestamp{timestamp}
{}

TrackId Track::id() const noexcept
{
    return m_id;
}

const Position& Track::position() const noexcept
{
    return m_position;
}

const Velocity& Track::velocity() const noexcept
{
    return m_velocity;
}

std::chrono::system_clock::time_point Track::timestamp() const noexcept
{
    return m_timestamp;
}

const std::optional<std::string>& Track::callsign() const noexcept
{
    return m_callsign;
}

void Track::associateCallsign(std::string callsign)
{
    m_callsign = std::move(callsign);
}

void Track::update(Position position,
                   Velocity velocity,
                   std::chrono::system_clock::time_point timestamp)
{
    m_position  = position;
    m_velocity  = velocity;
    m_timestamp = timestamp;
}

std::chrono::seconds
Track::age(std::chrono::system_clock::time_point now) const noexcept
{
    const auto delta = now - m_timestamp;
    return std::chrono::duration_cast<std::chrono::seconds>(delta);
}

bool Track::isStale(std::chrono::seconds              threshold,
                    std::chrono::system_clock::time_point now) const noexcept
{
    return age(now) >= threshold;
}

} // namespace cwp::domain
