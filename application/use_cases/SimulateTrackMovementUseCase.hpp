/**
 * @file SimulateTrackMovementUseCase.hpp
 * @brief Use case for advancing aircraft positions during simulation.
 *
 * This use case is responsible for:
 * 1. Reading all tracks from the repository
 * 2. Calculating new positions based on velocity vectors
 * 3. Updating sector boundary status (inside/outside controlled airspace)
 * 4. Saving updated tracks back to the repository
 *
 * The simulation uses great-circle approximations at the equator, which is
 * sufficient for visualization but not for safety-critical calculations.
 *
 * Thread Safety: Should be called from a single thread (Qt main thread).
 * The repository itself is thread-safe.
 */

#pragma once

#include <memory>

#include "domain/interfaces/ITrackRepository.hpp"
#include "domain/services/ISectorBoundaryService.hpp"

namespace cwp::application {

/**
 * @brief Use case: update track positions and sector status during simulation.
 *
 * Advances aircraft along their velocity vectors and updates their sector
 * boundary status. Separated from main() for testability and clarity.
 */
class SimulateTrackMovementUseCase {
public:
    SimulateTrackMovementUseCase(
        std::shared_ptr<domain::ITrackRepository> repository,
        std::shared_ptr<domain::ISectorBoundaryService> sectorService);

    /**
     * @brief Advances all tracks by the given time step.
     * @param deltaSeconds Time elapsed since last update.
     */
    void execute(double deltaSeconds);

private:
    std::shared_ptr<domain::ITrackRepository> m_repository;
    std::shared_ptr<domain::ISectorBoundaryService> m_sectorService;
    
    static constexpr double k_ktToDegreesPerSec = 1.852 / (111.12 * 3600.0);
};

} // namespace cwp::application
