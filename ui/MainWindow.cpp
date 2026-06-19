#include "ui/MainWindow.hpp"
#include "ui/FlightListView.hpp"
#include "ui/FlightStripView.hpp"
#include "ui/PlaybackView.hpp"
#include "ui/RadarView.hpp"

#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>

#include <utility>
#include <vector>

namespace cwp::ui {

MainWindow::MainWindow(
    std::shared_ptr<application::DisplayTrackUseCase>       displayUseCase,
    std::shared_ptr<application::MonitorStaleTracksUseCase> staleMonitorUseCase,
    std::shared_ptr<domain::IPlaybackService>               playbackService,
    QWidget* parent)
    : QMainWindow{parent}
    , m_displayUseCase{std::move(displayUseCase)}
    , m_staleMonitorUseCase{std::move(staleMonitorUseCase)}
    , m_playbackService{std::move(playbackService)}
{
    setupUi();
    connect(&m_refreshTimer, &QTimer::timeout, this, &MainWindow::onRefreshTimer);
    m_refreshTimer.start(k_refreshIntervalMs);
    
    // Trigger immediate display
    onRefreshTimer();
}

void MainWindow::setupUi()
{
    setWindowTitle(QStringLiteral("CWP — Controller Working Position"));
    resize(1280, 800);

    auto* tabs = new QTabWidget{this};
    tabs->setTabPosition(QTabWidget::South);
    tabs->setStyleSheet(
        "QTabWidget::pane { border:1px solid #333; }"
        "QTabBar::tab { background:#1a1a1a; color:#888; padding:6px 18px; "
        "  border:1px solid #333; border-bottom:none; }"
        "QTabBar::tab:selected { background:#000; color:#0f0; border-color:#0a0; }"
        "QTabBar::tab:hover { color:#ccc; }");

    m_radarView   = new RadarView{tabs};
    m_stripView   = new FlightStripView{tabs};
    m_listView    = new FlightListView{tabs};
    m_playbackView = new PlaybackView{m_playbackService, tabs};

    tabs->addTab(m_radarView,    QStringLiteral("Radar"));
    tabs->addTab(m_stripView,    QStringLiteral("Flight Strips"));
    tabs->addTab(m_listView,     QStringLiteral("Flight List"));
    tabs->addTab(m_playbackView, QStringLiteral("Recording / Playback"));

    setCentralWidget(tabs);
}

void MainWindow::onRefreshTimer()
{
    // Safety-first: evict stale tracks before refreshing the display.
    m_staleMonitorUseCase->execute();
    m_displayUseCase->execute();
}

void MainWindow::presentTracks(
    std::span<const std::shared_ptr<domain::Track>> tracks)
{
    std::vector<std::shared_ptr<domain::Track>> vec(tracks.begin(), tracks.end());
    m_radarView->updateTracks(vec);
    m_stripView->updateTracks(tracks);
    m_listView->updateTracks(tracks);
}

void MainWindow::presentTrackRemoved(domain::TrackId id)
{
    m_radarView->removeTrack(id);
    m_stripView->removeTrack(id);
    m_listView->removeTrack(id);
}

} // namespace cwp::ui

