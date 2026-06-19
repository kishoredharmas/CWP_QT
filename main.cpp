/**
 * @file main.cpp
 * @brief Application entry point and composition root.
 *
 * This file is responsible for:
 * 1. Creating all infrastructure dependencies (repositories, services)
 * 2. Wiring application use cases with dependencies (Dependency Injection)
 * 3. Creating and showing the UI
 * 4. Breaking circular dependencies (MainWindow implements ITrackPresenter)
 * 5. Starting simulation timer for demo mode
 *
 * Architecture Note:
 * - This is the ONLY place where concrete types are instantiated
 * - All other code depends on interfaces (Dependency Inversion Principle)
 * - No business logic belongs here (orchestration only)
 *
 * Safety Note:
 * - MonitorStaleTracksUseCase runs BEFORE DisplayTrackUseCase to ensure
 *   stale tracks are evicted before display refresh
 */

#include <QApplication>
#include <QTimer>

#include <chrono>
#include <memory>

#include "application/use_cases/DisplayTrackUseCase.hpp"
#include "application/use_cases/MonitorStaleTracksUseCase.hpp"
#include "application/use_cases/SeedDemoTracksUseCase.hpp"
#include "application/use_cases/SimulateTrackMovementUseCase.hpp"
#include "infrastructure/playback/FilePlaybackService.hpp"
#include "infrastructure/repositories/FileTrackRepository.hpp"
#include "infrastructure/services/PolygonSectorBoundaryService.hpp"
#include "ui/MainWindow.hpp"

namespace {

/// UK sector boundary definition (matches RadarView display)
constexpr std::array<std::pair<double, double>, 6> k_sectorVertices{{
    {47.0, -5.0}, {54.0, -4.0}, {56.0, 4.0},
    {54.5, 6.0}, {48.0, 5.0}, {46.0, -3.0}
}};

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
    auto sectorService =
        std::make_shared<cwp::infrastructure::PolygonSectorBoundaryService>(k_sectorVertices);

    // ── Application ─────────────────────────────────────────────────────────
    auto seedUseCase =
        std::make_shared<cwp::application::SeedDemoTracksUseCase>(
            trackRepository, sectorService);
    
    auto simulateMovementUseCase =
        std::make_shared<cwp::application::SimulateTrackMovementUseCase>(
            trackRepository, sectorService);
    
    auto displayUseCase =
        std::make_shared<cwp::application::DisplayTrackUseCase>(
            trackRepository, /*presenter=*/nullptr);

    auto staleMonitorUseCase =
        std::make_shared<cwp::application::MonitorStaleTracksUseCase>(
            trackRepository, /*presenter=*/nullptr);

    // Seed demo tracks so the radar has visible targets on first launch
    seedUseCase->execute();

    // ── Presentation ────────────────────────────────────────────────────────
    cwp::ui::MainWindow mainWindow{displayUseCase, staleMonitorUseCase, playbackService};

    // Resolve circular dependency: MainWindow IS the presenter.
    // The no-op deleter prevents a double-free; mainWindow is stack-owned.
    auto presenterHandle = std::shared_ptr<cwp::domain::ITrackPresenter>(
        &mainWindow, [](cwp::domain::ITrackPresenter*) {});

    displayUseCase->setPresenter(presenterHandle);
    staleMonitorUseCase->setPresenter(presenterHandle);

    mainWindow.show();

    // ── Simulation feed ──────────────────────────────────────────────────────
    // Advances each demo track along its heading every 1 second and resets
    // its timestamp, mimicking a real surveillance return.
    constexpr int    k_simIntervalMs = 1000;  // Update every second
    constexpr double k_simIntervalS  = k_simIntervalMs / 1000.0;
    constexpr double k_speedMultiplier = 5.0;  // Make movement more visible
    
    QTimer simTimer;
    QObject::connect(&simTimer, &QTimer::timeout, [&] {
        simulateMovementUseCase->execute(k_simIntervalS * k_speedMultiplier);
    });
    simTimer.start(k_simIntervalMs);

    return app.exec();
}

