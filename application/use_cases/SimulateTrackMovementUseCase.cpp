#include "application/use_cases/SimulateTrackMovementUseCase.hpp"

#include <cmath>
#include <numbers>
#include <utility>

namespace cwp::application {

SimulateTrackMovementUseCase::SimulateTrackMovementUseCase(
    std::shared_ptr<domain::ITrackRepository> repository,
    std::shared_ptr<domain::ISectorBoundaryService> sectorService)
    : m_repository{std::move(repository)}
    , m_sectorService{std::move(sectorService)}
{}

void SimulateTrackMovementUseCase::execute(double deltaSeconds)
{
    using namespace domain;
    const auto now = std::chrono::system_clock::now();

    for (const auto& track : m_repository->findAll()) {
        const double headingRad = track->velocity().heading() * std::numbers::pi / 180.0;

        const double dLat = std::cos(headingRad) * track->velocity().speed()
                            * k_ktToDegreesPerSec * deltaSeconds;
        const double dLon = std::sin(headingRad) * track->velocity().speed()
                            * k_ktToDegreesPerSec * deltaSeconds
                            / std::cos(track->position().latitude() * std::numbers::pi / 180.0);

        const double newLat = track->position().latitude() + dLat;
        const double newLon = track->position().longitude() + dLon;
        const int alt = track->position().altitude();

        // Validate new position is within domain bounds
        const bool positionValid = 
            newLat >= Position::k_minLatitude && newLat <= Position::k_maxLatitude &&
            newLon >= Position::k_minLongitude && newLon <= Position::k_maxLongitude;

        if (!positionValid) {
            // Track has left displayable area — will be handled by stale eviction
            continue;
        }

        const Position newPosition{newLat, newLon, alt};
        track->update(newPosition, track->velocity(), now);
        
        // Update sector boundary status
        track->setInsideSector(m_sectorService->isInsideSector(newPosition));
        
        m_repository->save(track);
    }
}

} // namespace cwp::application
