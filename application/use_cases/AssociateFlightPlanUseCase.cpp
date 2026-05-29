#include "application/use_cases/AssociateFlightPlanUseCase.hpp"

#include <utility>

namespace cwp::application {

AssociateFlightPlanUseCase::AssociateFlightPlanUseCase(
    std::shared_ptr<domain::ITrackRepository> repository)
    : m_repository{std::move(repository)}
{}

std::expected<void, domain::AssociationError>
AssociateFlightPlanUseCase::execute(domain::TrackId            trackId,
                                     const domain::FlightPlan& flightPlan)
{
    auto result = m_repository->findById(trackId);
    if (!result.has_value()) {
        return std::unexpected{domain::AssociationError::TrackNotFound};
    }

    result.value()->associateCallsign(flightPlan.callsign());
    m_repository->save(result.value());
    return {}; // success
}

} // namespace cwp::application
