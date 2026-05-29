#pragma once

#include <chrono>
#include <memory>

#include "domain/entities/Track.hpp"
#include "domain/interfaces/ITrackPresenter.hpp"
#include "domain/interfaces/ITrackRepository.hpp"

namespace cwp::application {

/**
 * @brief Use case: detect and evict stale radar tracks.
 *
 * Safety-critical: a track that has not received a surveillance update within
 * k_staleTrackTimeout must not continue to be displayed to a controller, as its
 * position is no longer verified. This use case scans the repository on every
 * radar cycle and removes any track whose age exceeds the configured threshold,
 * notifying the presenter so the symbol is immediately removed from the display.
 *
 * Should be executed on the same timer cycle as DisplayTrackUseCase.
 */
class MonitorStaleTracksUseCase {
public:
    /**
     * @brief Constructs the use case.
     * @param repository  Repository to scan and mutate.
     * @param presenter   Notified when a stale track is evicted.
     * @param threshold   Maximum acceptable track age (default: domain constant).
     */
    MonitorStaleTracksUseCase(
        std::shared_ptr<domain::ITrackRepository> repository,
        std::shared_ptr<domain::ITrackPresenter>  presenter,
        std::chrono::seconds threshold = domain::k_staleTrackTimeout);

    /**
     * @brief Scans all tracks; removes and reports any whose age ≥ threshold.
     *
     * @param now  Injected current time (defaults to now, overridable in tests).
     * @return Number of stale tracks evicted in this cycle.
     */
    [[nodiscard]] std::size_t
    execute(std::chrono::system_clock::time_point now =
                std::chrono::system_clock::now());

    /// @brief Late-binds the presenter; used at the composition root to
    ///        break the circular dependency with the UI layer.
    void setPresenter(std::shared_ptr<domain::ITrackPresenter> presenter);

private:
    std::shared_ptr<domain::ITrackRepository> m_repository;
    std::shared_ptr<domain::ITrackPresenter>  m_presenter;
    std::chrono::seconds                      m_threshold;
};

} // namespace cwp::application
