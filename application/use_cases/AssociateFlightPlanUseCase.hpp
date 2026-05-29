#pragma once

#include <expected>
#include <memory>

#include "domain/entities/FlightPlan.hpp"
#include "domain/entities/Track.hpp"
#include "domain/errors/DomainError.hpp"
#include "domain/interfaces/ITrackRepository.hpp"

namespace cwp::application {

/**
 * @brief Use case: correlate a filed flight plan with a specific radar track.
 *
 * An ATCo selects a track and enters a callsign; this use case finds the
 * track in the repository, updates its callsign from the flight plan, and
 * persists the change.
 *
 * Returns std::expected (C++23) so the caller is **forced** to handle
 * the error case — no silent failures in safety-critical ATC operations.
 */
class AssociateFlightPlanUseCase {
public:
    /**
     * @brief Constructs the use case.
     * @param repository Track repository used for look-up and persistence.
     */
    explicit AssociateFlightPlanUseCase(
        std::shared_ptr<domain::ITrackRepository> repository);

    /**
     * @brief Associates the flight plan callsign with the specified track.
     *
     * @param trackId    Strong TrackId — type system prevents passing a raw int.
     * @param flightPlan Flight plan whose callsign will be applied.
     * @return std::expected<void, AssociationError>:
     *   - success (has_value()) if the track was found and updated;
     *   - std::unexpected(AssociationError::TrackNotFound) otherwise.
     */
    [[nodiscard]] std::expected<void, domain::AssociationError>
    execute(domain::TrackId trackId, const domain::FlightPlan& flightPlan);

private:
    std::shared_ptr<domain::ITrackRepository> m_repository;
};

} // namespace cwp::application
