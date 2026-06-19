#pragma once

#include <memory>
#include <span>
#include <vector>

#include <QMainWindow>
#include <QTimer>

#include "application/use_cases/DisplayTrackUseCase.hpp"
#include "application/use_cases/MonitorStaleTracksUseCase.hpp"
#include "domain/interfaces/IPlaybackService.hpp"
#include "domain/interfaces/ITrackPresenter.hpp"

namespace cwp::ui {

class FlightListView;
class FlightStripView;
class PlaybackView;
class RadarView;

/**
 * @brief Top-level application window for the CWP Qt HMI.
 *
 * Owns the RadarView widget and drives the DisplayTrackUseCase on a
 * regular 4-second timer (one synthetic radar rotation). Also runs the
 * MonitorStaleTracksUseCase on the same cycle to remove outdated positions
 * from the display — a safety-critical requirement for live operations.
 *
 * Implements ITrackPresenter so that the application layer can push processed
 * track data directly into the UI without knowing about Qt.
 */
class MainWindow final : public QMainWindow,
                         public domain::ITrackPresenter
{
    Q_OBJECT

public:
    /**
     * @brief Constructs the MainWindow.
     * @param displayUseCase      Executed on each radar refresh tick.
     * @param staleMonitorUseCase Executed on the same tick; evicts stale tracks.
     * @param parent              Optional Qt parent widget.
     */
    explicit MainWindow(
        std::shared_ptr<application::DisplayTrackUseCase>       displayUseCase,
        std::shared_ptr<application::MonitorStaleTracksUseCase> staleMonitorUseCase,
        std::shared_ptr<domain::IPlaybackService>               playbackService,
        QWidget* parent = nullptr);

    ~MainWindow() override = default;

    // ── ITrackPresenter ────────────────────────────────────────────────────
    void presentTracks(
        std::span<const std::shared_ptr<domain::Track>> tracks) override;
    void presentTrackRemoved(domain::TrackId id) override;

private slots:
    void onRefreshTimer();

private:
    static constexpr int k_refreshIntervalMs{1000}; ///< Update every second.

    std::shared_ptr<application::DisplayTrackUseCase>       m_displayUseCase;
    std::shared_ptr<application::MonitorStaleTracksUseCase> m_staleMonitorUseCase;
    std::shared_ptr<domain::IPlaybackService>               m_playbackService;

    RadarView*        m_radarView{nullptr};
    FlightStripView*  m_stripView{nullptr};
    FlightListView*   m_listView{nullptr};
    PlaybackView*     m_playbackView{nullptr};

    QTimer            m_refreshTimer;

    void setupUi();
};

} // namespace cwp::ui

