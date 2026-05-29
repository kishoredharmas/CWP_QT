#include <QApplication>
#include <QTimer>

#include <chrono>
#include <cmath>
#include <memory>
#include <numbers>

#include "application/use_cases/DisplayTrackUseCase.hpp"
#include "application/use_cases/MonitorStaleTracksUseCase.hpp"
#include "domain/entities/Track.hpp"
#include "domain/interfaces/ITrackRepository.hpp"
#include "domain/value_objects/Position.hpp"
#include "domain/value_objects/Velocity.hpp"
#include "infrastructure/playback/FilePlaybackService.hpp"
#include "infrastructure/repositories/FileTrackRepository.hpp"
#include "ui/MainWindow.hpp"

namespace {

/// Knots → degrees of latitude per second (1 kt ≈ 1.852 km/h).
/// At the equator 1° lat ≈ 111.12 km; good enough for a demo approximation.
constexpr double k_ktToDegreesPerSec = 1.852 / (111.12 * 3600.0);

/// Advance every track in the repository by `dtSeconds` along its heading,
/// and reset its timestamp to `now` — simulating a new surveillance return.
void advanceTracks(cwp::domain::ITrackRepository& repo, double dtSeconds)
{
    using namespace cwp::domain;
    const auto now = std::chrono::system_clock::now();

    for (const auto& track : repo.findAll()) {
        const double headingRad =
            track->velocity().heading() * std::numbers::pi / 180.0;

        const double dLat = std::cos(headingRad) * track->velocity().speed()
                            * k_ktToDegreesPerSec * dtSeconds;
        const double dLon = std::sin(headingRad) * track->velocity().speed()
                            * k_ktToDegreesPerSec * dtSeconds
                            / std::cos(track->position().latitude()
                                       * std::numbers::pi / 180.0);

        const double newLat = track->position().latitude()  + dLat;
        const double newLon = track->position().longitude() + dLon;
        const int    alt    = track->position().altitude();

        // Clamp to valid domain bounds before updating.
        if (newLat < Position::k_minLatitude  || newLat > Position::k_maxLatitude  ||
            newLon < Position::k_minLongitude || newLon > Position::k_maxLongitude) {
            continue; // Track has left displayable area — leave it to stale-eviction.
        }

        track->update(Position{newLat, newLon, alt}, track->velocity(), now);
        repo.save(track);
    }
}

/// Seed the repository with representative demo tracks so the radar
/// displays activity on first launch (until live surveillance data is wired).
void seedDemoTracks(cwp::domain::ITrackRepository& repo)
{
    using namespace cwp::domain;
    using namespace std::chrono;

    const auto now = system_clock::now();

    struct Seed {
        std::uint32_t id;
        double lat, lon;
        double speedKt, headingDeg;
        int altFt;
        const char* callsign;
    };

    constexpr std::array<Seed, 6> seeds{{
        {1001,  51.8,  -0.5,  430,  175, 37000, "BAW123"},
        {1002,  51.2,   0.8,  410,  270, 35000, "EZY456"},
        {1003,  51.6,  -1.2,  460,   90, 39000, "VIR789"},
        {1004,  50.9,   0.2,  390,  320, 33000, "TOM101"},
        {1005,  51.4,   1.0,  440,  210, 36000, "RYR202"},
        {1006,  51.7,  -0.1,  420,   45, 38000, "DLH303"},
    }};

    for (const auto& s : seeds) {
        auto track = std::make_shared<Track>(
            TrackId{s.id},
            Position{s.lat, s.lon, s.altFt},
            Velocity{s.speedKt, s.headingDeg, 0},
            now);
        track->associateCallsign(s.callsign);
        repo.save(std::move(track));
    }
}

} // namespace

/**
 * @brief Application entry point — composition root.
 *
 * Wires the dependency graph in accordance with Clean Architecture:
 *   - Infrastructure provides concrete implementations of domain interfaces.
 *   - Application use cases receive dependencies via constructor injection.
 *   - The UI (MainWindow) implements ITrackPresenter and is registered with
 *     both use cases after construction to break the circular dependency.
 *
 * Safety wiring:
 *   - MonitorStaleTracksUseCase runs on every radar cycle BEFORE the display
 *     use case, ensuring stale tracks are evicted before they can be shown.
 */
int main(int argc, char* argv[])
{
    QApplication app{argc, argv};

    // ── Infrastructure ───────────────────────────────────────────────────────
    auto trackRepository =
        std::make_shared<cwp::infrastructure::FileTrackRepository>("cwp_tracks.json");
    auto playbackService =
        std::make_shared<cwp::infrastructure::FilePlaybackService>();

    // Seed demo tracks so the radar has visible targets on first launch.
    seedDemoTracks(*trackRepository);

    // ── Application ─────────────────────────────────────────────────────────
    // Presenters are wired after MainWindow is constructed (resolves circular dep).
    auto displayUseCase =
        std::make_shared<cwp::application::DisplayTrackUseCase>(
            trackRepository, /*presenter=*/nullptr);

    auto staleMonitorUseCase =
        std::make_shared<cwp::application::MonitorStaleTracksUseCase>(
            trackRepository, /*presenter=*/nullptr);

    // ── Presentation ────────────────────────────────────────────────────────
    cwp::ui::MainWindow mainWindow{displayUseCase, staleMonitorUseCase, playbackService};

    // Resolve circular dependency: MainWindow IS the presenter.
    // The no-op deleter prevents a double-free; mainWindow is stack-owned.
    auto presenterHandle = std::shared_ptr<cwp::domain::ITrackPresenter>(
        &mainWindow, [](cwp::domain::ITrackPresenter*) {});

    displayUseCase->setPresenter(presenterHandle);

    // Wire the stale monitor presenter via setPresenter (no reconstruction needed).
    staleMonitorUseCase->setPresenter(presenterHandle);

    mainWindow.show();

    // ── Simulation feed ──────────────────────────────────────────────────────
    // Advances each demo track along its heading every 4 seconds and resets
    // its timestamp, mimicking a real surveillance return. Without this the
    // MonitorStaleTracksUseCase (correctly) evicts tracks after 60 s.
    constexpr int    k_simIntervalMs = 4000;
    constexpr double k_simIntervalS  = k_simIntervalMs / 1000.0;
    QTimer simTimer;
    QObject::connect(&simTimer, &QTimer::timeout, [&] {
        advanceTracks(*trackRepository, k_simIntervalS);
    });
    simTimer.start(k_simIntervalMs);

    return app.exec();
}

