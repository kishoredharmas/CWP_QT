#pragma once

#include <memory>

#include "domain/interfaces/ITrackPresenter.hpp"
#include "domain/interfaces/ITrackRepository.hpp"

namespace cwp::application {

/**
 * @brief Use case: fetch all active tracks and deliver them to the presenter.
 *
 * On each call to execute(), the use case queries the track repository for
 * the current snapshot and forwards it through the presenter interface so the
 * UI can refresh. Both dependencies are injected via constructor for
 * testability (DIP / SRP).
 */
class DisplayTrackUseCase {
public:
    /**
     * @brief Constructs the use case with its required collaborators.
     * @param repository  Source of radar track data. Must not be null.
     * @param presenter   UI output boundary. May be null; execute() is a no-op if so.
     */
    DisplayTrackUseCase(std::shared_ptr<domain::ITrackRepository> repository,
                        std::shared_ptr<domain::ITrackPresenter>  presenter);

    /**
     * @brief Sets (or replaces) the presenter after construction.
     *
     * Useful at the composition root when the window is both the owner of
     * this use case and the implementor of ITrackPresenter.
     *
     * @param presenter New presenter. May be null to detach.
     */
    void setPresenter(std::shared_ptr<domain::ITrackPresenter> presenter);

    /// Reads all active tracks and pushes them to the presenter.
    void execute();

private:
    std::shared_ptr<domain::ITrackRepository> m_repository;
    std::shared_ptr<domain::ITrackPresenter>  m_presenter;
};

} // namespace cwp::application
