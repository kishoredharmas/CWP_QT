/**
 * @file ISectorBoundaryService.hpp
 * @brief Domain service interface for sector boundary operations.
 *
 * Defines the contract for determining whether aircraft are within
 * controlled airspace boundaries. Different implementations can support:
 * - Polygon boundaries (ray-casting algorithm)
 * - Circular boundaries (distance calculation)
 * - Complex 3D volumes (altitude-dependent)
 *
 * Usage Example:
 * @code
 *   auto service = std::make_shared<PolygonSectorBoundaryService>(vertices);
 *   bool inside = service->isInsideSector(track->position());
 *   if (!inside) {
 *       // Aircraft has left controlled airspace
 *   }
 * @endcode
 *
 * Thread Safety: Implementations must be thread-safe for concurrent reads.
 */

#pragma once

#include "domain/value_objects/Position.hpp"

namespace cwp::domain {

/**
 * @brief Domain service for sector boundary calculations.
 *
 * Separates the geometry logic from UI and simulation concerns,
 * making it testable and reusable across different sectors.
 */
class ISectorBoundaryService {
public:
    virtual ~ISectorBoundaryService() = default;

    /**
     * @brief Tests whether a position is inside the controlled airspace sector.
     * @param position Geographic position to test.
     * @return True if inside the sector boundary, false otherwise.
     */
    [[nodiscard]] virtual bool isInsideSector(const Position& position) const noexcept = 0;

    /**
     * @brief Generates a random position on the sector boundary.
     * @return A position on the boundary perimeter.
     */
    [[nodiscard]] virtual Position randomBoundaryPosition() const = 0;

    /**
     * @brief Calculates the approximate center of the sector.
     * @return Center position of the controlled airspace.
     */
    [[nodiscard]] virtual Position sectorCenter() const noexcept = 0;
};

} // namespace cwp::domain
