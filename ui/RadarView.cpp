#include "ui/RadarView.hpp"

#include <algorithm>
#include <cmath>
#include <ranges>

#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QPen>
#include <QRectF>

namespace cwp::ui {

// ── Colour palette — 8 distinct ATC-style hues ───────────────────────────────
const std::array<QColor, 8> RadarView::k_trackColours{{
    QColor{0,   255, 128},   // emerald green
    QColor{0,   200, 255},   // cyan
    QColor{255, 220,   0},   // amber
    QColor{255, 100, 100},   // soft red
    QColor{180, 130, 255},   // lavender
    QColor{ 80, 220,  80},   // lime
    QColor{255, 170,  50},   // orange
    QColor{100, 200, 255},   // sky blue
}};

RadarView::RadarView(QWidget* parent)
    : QWidget{parent}
{
    setMinimumSize(800, 600);
    setStyleSheet(QStringLiteral("background-color: #080d08;"));
    setMouseTracking(true); // receive mouseMoveEvent even without button held
}

void RadarView::updateTracks(std::vector<std::shared_ptr<domain::Track>> tracks)
{
    m_tracks = std::move(tracks);
    // Reset hover if the hovered track was removed.
    if (m_hoveredIndex && *m_hoveredIndex >= m_tracks.size()) {
        m_hoveredIndex.reset();
    }
    update();
}

void RadarView::removeTrack(domain::TrackId id)
{
    const auto removed = std::ranges::remove_if(
        m_tracks,
        [id](const std::shared_ptr<domain::Track>& t) { return t->id() == id; });
    m_tracks.erase(removed.begin(), removed.end());
    m_hoveredIndex.reset();
    update();
}

void RadarView::mouseMoveEvent(QMouseEvent* event)
{
    const QPointF cursor = event->position();
    std::optional<std::size_t> nearest;
    double minDist = k_hoverRadiusPx;

    for (std::size_t i = 0; i < m_tracks.size(); ++i) {
        const QPointF sp = project(m_tracks[i]->position());
        const double dx  = cursor.x() - sp.x();
        const double dy  = cursor.y() - sp.y();
        const double d   = std::sqrt(dx * dx + dy * dy);
        if (d < minDist) {
            minDist = d;
            nearest = i;
        }
    }

    if (nearest != m_hoveredIndex) {
        m_hoveredIndex = nearest;
        update();
    }
}

void RadarView::leaveEvent(QEvent* /*event*/)
{
    if (m_hoveredIndex) {
        m_hoveredIndex.reset();
        update();
    }
}

void RadarView::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter{this};
    painter.setRenderHint(QPainter::Antialiasing);

    drawRangeRings(painter);

    // First pass: draw all non-hovered tracks.
    for (std::size_t i = 0; i < m_tracks.size(); ++i) {
        if (m_hoveredIndex && *m_hoveredIndex == i) {
            continue;
        }
        drawTrack(painter, *m_tracks[i], colourForIndex(i), /*hovered=*/false);
    }

    // Second pass: draw the hovered track and its tooltip on top of everything.
    if (m_hoveredIndex) {
        const std::size_t i = *m_hoveredIndex;
        const QColor colour = colourForIndex(i);
        drawTrack(painter, *m_tracks[i], colour, /*hovered=*/true);
        drawHoverTooltip(painter, *m_tracks[i],
                         project(m_tracks[i]->position()), colour);
    }
}

QPointF RadarView::project(const domain::Position& position) const
{
    const double dx = (position.longitude() - k_centerLon) * k_scalePixPerDeg;
    const double dy = (k_centerLat - position.latitude())  * k_scalePixPerDeg;
    return {static_cast<double>(width())  / 2.0 + dx,
            static_cast<double>(height()) / 2.0 + dy};
}

QColor RadarView::colourForIndex(std::size_t index) const
{
    return k_trackColours[index % k_trackColours.size()];
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

void RadarView::drawTrack(QPainter& painter, const domain::Track& track,
                          QColor colour, bool hovered) const
{
    const QPointF pos = project(track.position());
    const auto    hs  = static_cast<double>(hovered ? k_symbolHalfSize + 3
                                                     : k_symbolHalfSize);

    // Cross symbol — brighter and larger when hovered.
    painter.setPen(QPen{colour, hovered ? 2.5 : 2.0});
    painter.drawLine(QPointF{pos.x() - hs, pos.y()},
                     QPointF{pos.x() + hs, pos.y()});
    painter.drawLine(QPointF{pos.x(), pos.y() - hs},
                     QPointF{pos.x(), pos.y() + hs});

    // Compact label (always visible, small).
    const QString callsign = track.callsign().has_value()
        ? QString::fromStdString(*track.callsign())
        : QStringLiteral("????");

    const QString label =
        QStringLiteral("%1\nFL%2 %3kt")
            .arg(callsign)
            .arg(track.position().altitude() / 100)
            .arg(static_cast<int>(track.velocity().speed()));

    painter.setPen(colour);
    painter.setFont(QFont{QStringLiteral("Monospace"), 8});
    painter.drawText(QPointF{pos.x() + hs + 4.0, pos.y() - 2.0}, label);
}

void RadarView::drawHoverTooltip(QPainter& painter, const domain::Track& track,
                                 QPointF screenPos, QColor colour) const
{
    const QString callsign = track.callsign().has_value()
        ? QString::fromStdString(*track.callsign())
        : QStringLiteral("????");

    // Full details shown on hover.
    const QString text =
        QStringLiteral("▶ %1\n"
                        "FL%2   %3 kt\n"
                        "HDG %4°\n"
                        "LAT %5   LON %6")
            .arg(callsign)
            .arg(track.position().altitude() / 100)
            .arg(static_cast<int>(track.velocity().speed()))
            .arg(static_cast<int>(track.velocity().heading()))
            .arg(track.position().latitude(),  0, 'f', 3)
            .arg(track.position().longitude(), 0, 'f', 3);

    const QFont font{QStringLiteral("Monospace"), 10, QFont::Bold};
    painter.setFont(font);
    const QFontMetrics fm{font};
    const QRectF textBounds = fm.boundingRect(
        QRect{0, 0, 300, 200},
        Qt::AlignLeft | Qt::TextWordWrap,
        text).adjusted(-8, -6, 8, 6);

    // Position the tooltip to the right of the symbol; flip left if near edge.
    constexpr double k_offset = 16.0;
    double tx = screenPos.x() + k_offset;
    double ty = screenPos.y() - textBounds.height() / 2.0;

    if (tx + textBounds.width() > width() - 4) {
        tx = screenPos.x() - k_offset - textBounds.width();
    }
    ty = std::clamp(ty, 4.0, static_cast<double>(height()) - textBounds.height() - 4);

    const QRectF box{tx, ty, textBounds.width(), textBounds.height()};

    // Semi-transparent background so the tooltip never covers other labels.
    painter.setBrush(QColor{0, 0, 0, 200});
    painter.setPen(QPen{colour, 1});
    painter.drawRoundedRect(box, 4, 4);

    painter.setPen(colour);
    painter.drawText(box.adjusted(8, 6, -8, -6),
                     Qt::AlignLeft | Qt::TextWordWrap, text);
}

} // namespace cwp::ui

