#include "ui/MainWindow.hpp"
#include "ui/RadarView.hpp"

#include <QVBoxLayout>
#include <QWidget>

#include <utility>
#include <vector>

namespace cwp::ui {

MainWindow::MainWindow(
    std::shared_ptr<application::DisplayTrackUseCase>      displayUseCase,
    std::shared_ptr<application::MonitorStaleTracksUseCase> staleMonitorUseCase,
    QWidget* parent)
    : QMainWindow{parent}
    , m_displayUseCase{std::move(displayUseCase)}
    , m_staleMonitorUseCase{std::move(staleMonitorUseCase)}
{
    setupUi();
    connect(&m_refreshTimer, &QTimer::timeout, this, &MainWindow::onRefreshTimer);
    m_refreshTimer.start(k_refreshIntervalMs);
}

void MainWindow::setupUi()
{
    setWindowTitle(QStringLiteral("CWP — Controller Working Position"));
    resize(1280, 800);

    auto* central = new QWidget{this};
    auto* layout  = new QVBoxLayout{central};

    m_radarView = new RadarView{central};
    layout->addWidget(m_radarView);
    layout->setContentsMargins(0, 0, 0, 0);

    setCentralWidget(central);
}

void MainWindow::onRefreshTimer()
{
    // Safety-first: evict stale tracks before refreshing the display so a
    // controller never sees an out-of-date position symbol.
    m_staleMonitorUseCase->execute();
    m_displayUseCase->execute();
}

void MainWindow::presentTracks(
    std::span<const std::shared_ptr<domain::Track>> tracks)
{
    // RadarView stores a copy — the span is only valid during this call.
    m_radarView->updateTracks(
        std::vector<std::shared_ptr<domain::Track>>(tracks.begin(), tracks.end()));
}

void MainWindow::presentTrackRemoved(domain::TrackId id)
{
    m_radarView->removeTrack(id);
}

} // namespace cwp::ui
