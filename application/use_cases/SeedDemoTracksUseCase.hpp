/**
 * @file SeedDemoTracksUseCase.hpp
 * @brief Use case for generating initial demo tracks at sector boundaries.
 *
 * This use case creates aircraft starting at random positions along the
 * sector boundary, with headings directed toward the sector center (ensuring
 * they cross through the airspace to the opposite boundary).
 *
 * Each track is assigned:
 * - Random callsign from a predefined list
 * - Random speed (380-460 knots, typical for jets)
 * - Random altitude (FL330-FL390, typical cruise altitudes)
 * - Heading toward sector center
 *
 * Usage: Call once at application startup to populate the display.
 */

#pragma once

#include <memory>

#include "domain/interfaces/ITrackRepository.hpp"
#include "domain/services/ISectorBoundaryService.hpp"

namespace cwp::application {

/**
 * @brief Use case: seed the repository with demo tracks for initial display.
 *
 * Generates aircraft starting at random boundary positions with headings
 * toward the opposite side of the sector. Separated for testability.
 */
class SeedDemoTracksUseCase {
public:
    SeedDemoTracksUseCase(
        std::shared_ptr<domain::ITrackRepository> repository,
        std::shared_ptr<domain::ISectorBoundaryService> sectorService);

    /**
     * @brief Seeds the repository with demo tracks.
     * @param count Number of tracks to create.
     */
    void execute(std::size_t count = 12);

private:
    std::shared_ptr<domain::ITrackRepository> m_repository;
    std::shared_ptr<domain::ISectorBoundaryService> m_sectorService;
    
    static double calculateHeading(double lat1, double lon1, double lat2, double lon2);
};

} // namespace cwp::application
