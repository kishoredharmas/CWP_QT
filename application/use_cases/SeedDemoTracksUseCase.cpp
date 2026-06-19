#include "application/use_cases/SeedDemoTracksUseCase.hpp"

#include <array>
#include <cmath>
#include <numbers>
#include <random>
#include <utility>

namespace cwp::application {

SeedDemoTracksUseCase::SeedDemoTracksUseCase(
    std::shared_ptr<domain::ITrackRepository> repository,
    std::shared_ptr<domain::ISectorBoundaryService> sectorService)
    : m_repository{std::move(repository)}
    , m_sectorService{std::move(sectorService)}
{}

void SeedDemoTracksUseCase::execute(std::size_t count)
{
    using namespace domain;
    using namespace std::chrono;

    const auto now = system_clock::now();

    constexpr std::array<const char*, 12> callsigns = {
        "BAW123", "EZY456", "VIR789", "TOM101", "RYR202", "DLH303",
        "AFR404", "KLM505", "SWR606", "UAE707", "AAL808", "UAL909"
    };

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> speedDist(380.0, 460.0);
    std::uniform_int_distribution<> altDist(33000, 39000);
    
    const Position center = m_sectorService->sectorCenter();
    
    for (std::size_t i = 0; i < std::min(count, callsigns.size()); ++i) {
        // Start at a random boundary point
        const Position startPos = m_sectorService->randomBoundaryPosition();
        
        // Calculate heading toward sector center (will cross to opposite side)
        const double heading = calculateHeading(
            startPos.latitude(), startPos.longitude(),
            center.latitude(), center.longitude());
        
        const double speed = speedDist(gen);
        const int altitude = altDist(gen);
        
        // Create track at boundary with appropriate altitude
        const Position spawnPos{startPos.latitude(), startPos.longitude(), altitude};
        
        auto track = std::make_shared<Track>(
            TrackId{static_cast<std::uint32_t>(1001 + i)},
            spawnPos,
            Velocity{speed, heading, 0},
            now);
        track->associateCallsign(callsigns[i]);
        track->setInsideSector(true); // Starting at boundary, considered inside initially
        m_repository->save(std::move(track));
    }
}

double SeedDemoTracksUseCase::calculateHeading(
    double lat1, double lon1, double lat2, double lon2)
{
    const double dLon = (lon2 - lon1) * std::numbers::pi / 180.0;
    const double lat1Rad = lat1 * std::numbers::pi / 180.0;
    const double lat2Rad = lat2 * std::numbers::pi / 180.0;
    
    const double y = std::sin(dLon) * std::cos(lat2Rad);
    const double x = std::cos(lat1Rad) * std::sin(lat2Rad) -
                     std::sin(lat1Rad) * std::cos(lat2Rad) * std::cos(dLon);
    
    double heading = std::atan2(y, x) * 180.0 / std::numbers::pi;
    return std::fmod(heading + 360.0, 360.0);
}

} // namespace cwp::application
