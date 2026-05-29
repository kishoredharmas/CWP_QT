#include "ui/RadarView.hpp"

#include <algorithm>
#include <ranges>

#include <QColor>
#include <QFont>
#include <QPainter>
#include <QPaintEvent>
#include <QPen>

namespace cwp::ui {

RadarView::RadarView(QWidget* parent)
    : QWidget{parent}
{
    setMinimumSize(800, 600);
    setStyleSheet(QStringLiteral("background-color: #080d08;"));
}

void RadarView::updateTracks(std::vector<std::shared_ptr<domain::Track>> tracks)
{
    m_tracks = std::move(tracks);
    update();
}

void RadarView::removeTrack(domain::TrackId id)
{
    // C++23 std::ranges::remove + erase idiom — expressive and range-safe.
    const auto removed = std::ranges::remove_if(
        m_tracks,
        [id](const std::shared_ptr<domain::Track>& t) { return t->id() == id; });
    m_tracks.erase(removed.begin(), removed.end());
    update();
}

void RadarView::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter{this};
    painter.setRenderHint(QPainter::Antialiasing);

    drawRangeRings(painter);

    for (const auto& track : m_tracks) {
        drawTrack(painter, *track);
    }
}

QPointF RadarView::project(const domain::Position& position) const
{
    const double dx = (position.longitude() - k_centerLon) * k_scalePixPerDeg;
    const double dy = (k_centerLat - position.latitude())  * k_scalePixPerDeg;
    return {static_cast<double>(width())  / 2.0 + dx,
            static_cast<double>(height()) / 2.0 + dy};
}

void RadarView::drawRangeRings(QPainter& painter) const
{
    painter.setPen(QPen{QColor{0, 55, 0}, 1, Qt::DotLine});
    const QPointF centre{static_cast<double>(width())  / 2.0,
                         static_cast<double>(height()) / 2.0};
    for (int ring = 1; ring <= k_rangeRingCount; ++ring) {
        const double radius = ring * k_scalePixPerDeg * 2.0;
        painter.drawEllipse(centre, radius, radius);
    }
}

void RadarView::drawTrack(QPainter& painter, const domain::Track& track) const
{
    const QPointF pos = project(track.position());

    // Track symbol — filled cross
    const auto hs = static_cast<double>(k_symbolHalfSize);
    painter.setPen(QPen{Qt::green, 2});
    painter.drawLine(QPointF{pos.x() - hs, pos.y()},
                     QPointF{pos.x() + hs, pos.y()});
    painter.drawLine(QPointF{pos.x(), pos.y() - hs},
                     QPointF{pos.x(), pos.y() + hs});

    // Data label
    const QString callsign = track.callsign().has_value()
        ? QString::fromStdString(*track.callsign())
        : QStringLiteral("????");

    const QString label = QStringLiteral("%1\nFL%2 %3kt")
        .arg(callsign)
        .arg(track.position().altitude() / 100)
        .arg(static_cast<int>(track.velocity().speed()));

    painter.setPen(Qt::green);
    painter.setFont(QFont{QStringLiteral("Monospace"), 8});
    painter.drawText(QPointF{pos.x() + hs + 2.0, pos.y() - hs}, label);
}

} // namespace cwp::ui
