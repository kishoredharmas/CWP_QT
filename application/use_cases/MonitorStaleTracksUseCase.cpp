#include "application/use_cases/MonitorStaleTracksUseCase.hpp"

#include <utility>
#include <vector>

namespace cwp::application {

MonitorStaleTracksUseCase::MonitorStaleTracksUseCase(
    std::shared_ptr<domain::ITrackRepository> repository,
    std::shared_ptr<domain::ITrackPresenter>  presenter,
    std::chrono::seconds threshold)
    : m_repository{std::move(repository)}
    , m_presenter{std::move(presenter)}
    , m_threshold{threshold}
{}

std::size_t
MonitorStaleTracksUseCase::execute(std::chrono::system_clock::time_point now)
{
    const auto tracks = m_repository->findAll();

    // Collect stale IDs first to avoid mutating the repository while iterating.
    std::vector<domain::TrackId> staleIds;
    staleIds.reserve(tracks.size()); // worst-case: all tracks stale

    for (const auto& track : tracks) {
        if (track->isStale(m_threshold, now)) {
            [[likely]] // most iterations will NOT add to staleIds in normal ops
            staleIds.push_back(track->id());
        }
    }

    for (const auto& id : staleIds) {
        m_repository->remove(id);
        if (m_presenter) {
            m_presenter->presentTrackRemoved(id);
        }
    }

    return staleIds.size();
}

void MonitorStaleTracksUseCase::setPresenter(
    std::shared_ptr<domain::ITrackPresenter> presenter)
{
    m_presenter = std::move(presenter);
}

} // namespace cwp::application
