#include "application/use_cases/DisplayTrackUseCase.hpp"

#include <utility>

namespace cwp::application {

DisplayTrackUseCase::DisplayTrackUseCase(
    std::shared_ptr<domain::ITrackRepository> repository,
    std::shared_ptr<domain::ITrackPresenter>  presenter)
    : m_repository{std::move(repository)}
    , m_presenter{std::move(presenter)}
{}

void DisplayTrackUseCase::setPresenter(std::shared_ptr<domain::ITrackPresenter> presenter)
{
    m_presenter = std::move(presenter);
}

void DisplayTrackUseCase::execute()
{
    if (!m_presenter) {
        return;
    }
    const auto tracks = m_repository->findAll();
    // Pass a std::span: zero-copy view, avoids an extra vector allocation
    // on the 4-second radar refresh hot path.
    m_presenter->presentTracks(std::span{tracks});
}

} // namespace cwp::application
