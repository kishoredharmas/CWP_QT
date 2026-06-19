#include "infrastructure/services/PolygonSectorBoundaryService.hpp"

#include <cmath>
#include <numeric>

namespace cwp::infrastructure {

bool PolygonSectorBoundaryService::isInsideSector(
    const domain::Position& position) const noexcept
{
    const double lat = position.latitude();
    const double lon = position.longitude();
    
    int intersections = 0;
    const std::size_t n = m_vertices.size();
    
    for (std::size_t i = 0; i < n; ++i) {
        const auto& [lat1, lon1] = m_vertices[i];
        const auto& [lat2, lon2] = m_vertices[(i + 1) % n];
        
        // Ray casting: cast ray from point going east (+lon direction)
        const bool lonInRange = (lon1 <= lon && lon < lon2) || 
                                 (lon2 <= lon && lon < lon1);
        
        if (lonInRange) {
            const double latAtLon = lat1 + (lon - lon1) / (lon2 - lon1) * (lat2 - lat1);
            if (lat < latAtLon) {
                ++intersections;
            }
        }
    }
    
    return (intersections % 2) == 1;
}

domain::Position PolygonSectorBoundaryService::randomBoundaryPosition() const
{
    std::uniform_int_distribution<std::size_t> edgeDist(0, m_vertices.size() - 1);
    std::uniform_real_distribution<> tDist(0.0, 1.0);
    
    const std::size_t edge = edgeDist(m_generator);
    const double t = tDist(m_generator);
    
    const auto& [lat1, lon1] = m_vertices[edge];
    const auto& [lat2, lon2] = m_vertices[(edge + 1) % m_vertices.size()];
    
    const double lat = lat1 + t * (lat2 - lat1);
    const double lon = lon1 + t * (lon2 - lon1);
    
    // Use a reasonable default altitude for boundary spawning
    constexpr int defaultAltitude = 35000;
    
    return domain::Position{lat, lon, defaultAltitude};
}

void PolygonSectorBoundaryService::calculateCenter()
{
    if (m_vertices.empty()) {
        m_center = domain::Position{0.0, 0.0, 0};
        return;
    }
    
    double sumLat = 0.0;
    double sumLon = 0.0;
    
    for (const auto& [lat, lon] : m_vertices) {
        sumLat += lat;
        sumLon += lon;
    }
    
    const double centerLat = sumLat / static_cast<double>(m_vertices.size());
    const double centerLon = sumLon / static_cast<double>(m_vertices.size());
    
    m_center = domain::Position{centerLat, centerLon, 0};
}

} // namespace cwp::infrastructure
