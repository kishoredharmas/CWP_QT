#include <QApplication>

#include <chrono>
#include <memory>

#include "application/use_cases/DisplayTrackUseCase.hpp"
#include "application/use_cases/MonitorStaleTracksUseCase.hpp"
#include "domain/entities/Track.hpp"
#include "domain/interfaces/ITrackRepository.hpp"
#include "domain/value_objects/Position.hpp"
#include "domain/value_objects/Velocity.hpp"
#include "infrastructure/repositories/FileTrackRepository.hpp"
#include "ui/MainWindow.hpp"

namespace {

/// Seed the repository with representative demo tracks so the radar
/// displays activity on first launch (until live surveillance data is wired).
void seedDemoTracks(cwp::domain::ITrackRepository& repo)
{
    using namespace cwp::domain;
    using namespace std::chrono;

    const auto now = system_clock::now();

    // Aircraft in the London TMA area (lat ~51.5, lon ~0.0)
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
    cwp::ui::MainWindow mainWindow{displayUseCase, staleMonitorUseCase};

    // Resolve circular dependency: MainWindow IS the presenter.
    // The no-op deleter prevents a double-free; mainWindow is stack-owned.
    auto presenterHandle = std::shared_ptr<cwp::domain::ITrackPresenter>(
        &mainWindow, [](cwp::domain::ITrackPresenter*) {});

    displayUseCase->setPresenter(presenterHandle);

    // Wire the stale monitor presenter via setPresenter (no reconstruction needed).
    staleMonitorUseCase->setPresenter(presenterHandle);

    mainWindow.show();
    return app.exec();
}

