#pragma once

#include <array>
#include <random>
#include <utility>

#include "domain/services/ISectorBoundaryService.hpp"

namespace cwp::infrastructure {

/**
 * @brief Sector boundary service using polygon ray-casting algorithm.
 *
 * Thread-safe implementation using a polygon defined by lat/lon vertices.
 */
class PolygonSectorBoundaryService final : public domain::ISectorBoundaryService {
public:
    /**
     * @brief Constructs a sector boundary from polygon vertices.
     * @param vertices Array of (latitude, longitude) pairs defining the boundary.
     */
    template<std::size_t N>
    explicit PolygonSectorBoundaryService(
        const std::array<std::pair<double, double>, N>& vertices)
        : m_vertices(vertices.begin(), vertices.end())
        , m_generator(std::random_device{}())
    {
        calculateCenter();
    }

    [[nodiscard]] bool isInsideSector(const domain::Position& position) const noexcept override;
    [[nodiscard]] domain::Position randomBoundaryPosition() const override;
    [[nodiscard]] domain::Position sectorCenter() const noexcept override { return m_center; }

private:
    std::vector<std::pair<double, double>> m_vertices;
    mutable std::mt19937 m_generator;  // mutable for const methods
    domain::Position m_center{0.0, 0.0, 0};

    void calculateCenter();
};

} // namespace cwp::infrastructure
