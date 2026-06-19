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
#include <QPolygonF>
#include <QRectF>

namespace cwp::ui {

RadarView::RadarView(QWidget* parent)
    : QWidget{parent}
{
    setMinimumSize(800, 600);
    setStyleSheet(QStringLiteral("background-color: #2a2a2a;"));
    setMouseTracking(true);
}

void RadarView::updateTracks(std::vector<std::shared_ptr<domain::Track>> tracks)
{
    m_tracks = std::move(tracks);
    updateTrails();
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

    drawBackground(painter);
    drawSector(painter);
    drawRoutes(painter);

    // Draw trails for all aircraft
    for (const auto& track : m_tracks) {
        const auto trailIt = m_trails.find(track->id());
        if (trailIt != m_trails.end()) {
            drawTrail(painter, trailIt->second, track->isInsideSector());
        }
    }

    // Draw aircraft and labels with collision avoidance
    std::vector<QRectF> occupiedRects;
    for (std::size_t i = 0; i < m_tracks.size(); ++i) {
        const auto& track = m_tracks[i];
        const QPointF pos = project(track->position());
        const bool hovered = (m_hoveredIndex && *m_hoveredIndex == i);
        
        drawAircraft(painter, *track, pos, hovered);
        drawLabel(painter, *track, pos, occupiedRects, occupiedRects);
    }

    // Draw hover tooltip on top
    if (m_hoveredIndex) {
        const auto& track = m_tracks[*m_hoveredIndex];
        drawHoverTooltip(painter, *track, project(track->position()));
    }
}

QPointF RadarView::project(const domain::Position& position) const
{
    const double dx = (position.longitude() - k_centerLon) * k_scalePixPerDeg;
    const double dy = (k_centerLat - position.latitude())  * k_scalePixPerDeg;
    return {static_cast<double>(width())  / 2.0 + dx,
            static_cast<double>(height()) / 2.0 + dy};
}

void RadarView::drawBackground(QPainter& painter) const
{
    painter.fillRect(rect(), QColor{42, 42, 42});
}

void RadarView::drawSector(QPainter& painter) const
{
    // Draw much larger irregular sector polygon (control airspace boundary)
    QPolygonF sector;
    sector << project(domain::Position{47.0, -5.0, 0})
           << project(domain::Position{54.0, -4.0, 0})
           << project(domain::Position{56.0,  4.0, 0})
           << project(domain::Position{54.5,  6.0, 0})
           << project(domain::Position{48.0,  5.0, 0})
           << project(domain::Position{46.0, -3.0, 0});
    
    painter.setBrush(QColor{0, 0, 0});
    painter.setPen(QPen{QColor{60, 60, 60}, 0.8});
    painter.drawPolygon(sector);
}

void RadarView::drawRoutes(QPainter& painter) const
{
    // Draw thin grey lines representing air routes across larger sector
    painter.setPen(QPen{QColor{70, 70, 70}, 0.5, Qt::SolidLine});
    
    // Route 1: Northwest-Southeast
    painter.drawLine(
        project(domain::Position{56.0, -4.0, 0}),
        project(domain::Position{47.0,  5.0, 0})
    );
    
    // Route 2: Northeast-Southwest
    painter.drawLine(
        project(domain::Position{56.0,  4.0, 0}),
        project(domain::Position{47.0, -4.0, 0})
    );
    
    // Route 3: Horizontal
    painter.drawLine(
        project(domain::Position{51.5, -5.0, 0}),
        project(domain::Position{51.5,  6.0, 0})
    );
    
    // Route 4: Vertical
    painter.drawLine(
        project(domain::Position{46.0,  0.0, 0}),
        project(domain::Position{56.0,  0.0, 0})
    );
}

void RadarView::drawTrail(QPainter& painter, const TrailHistory& trail,
                          bool isInsideSector) const
{
    if (trail.positions.empty()) {
        return;
    }
    
    // Choose color based on sector status: white inside, grey outside
    const QColor trailColor = isInsideSector 
        ? QColor{200, 200, 200}  // White for inside sector
        : QColor{100, 100, 100}; // Darker grey for outside sector
    
    // Draw distinct dots at each trail position
    painter.setPen(Qt::NoPen);
    painter.setBrush(trailColor);
    
    for (const auto& position : trail.positions) {
        // Project geographic position to screen coordinates
        const QPointF screenPos = project(position);
        painter.drawEllipse(screenPos, k_trailDotRadius, k_trailDotRadius);
    }
}

void RadarView::drawAircraft(QPainter& painter, const domain::Track& track,
                             const QPointF& pos, bool hovered) const
{
    // Choose color based on sector status: green inside, grey outside
    const QColor vectorColor = track.isInsideSector() 
        ? QColor{0, 255, 100}   // Green for inside sector
        : QColor{120, 120, 120}; // Grey for outside sector
    
    // Draw white dot for aircraft
    const int radius = hovered ? k_dotRadius + 2 : k_dotRadius;
    painter.setBrush(QColor{255, 255, 255});
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(pos, radius, radius);
    
    // Draw velocity vector with appropriate color
    const double heading = track.velocity().heading() * M_PI / 180.0;
    const double speed = track.velocity().speed();
    const double vectorLen = k_vectorLength * (speed / 500.0); // Scale by speed
    
    const QPointF vectorEnd{
        pos.x() + vectorLen * std::sin(heading),
        pos.y() - vectorLen * std::cos(heading)
    };
    
    painter.setPen(QPen{vectorColor, hovered ? 1.5 : 1.0});
    painter.drawLine(pos, vectorEnd);
}

void RadarView::drawLabel(QPainter& painter, const domain::Track& track,
                          const QPointF& pos, const std::vector<QRectF>& occupiedRects,
                          std::vector<QRectF>& newOccupiedRects) const
{
    // Choose color based on sector status: green inside, grey outside
    const QColor labelColor = track.isInsideSector() 
        ? QColor{0, 255, 100}   // Green for inside sector
        : QColor{120, 120, 120}; // Grey for outside sector
    
    const QString callsign = track.callsign().has_value()
        ? QString::fromStdString(*track.callsign())
        : QStringLiteral("????");
    
    const int flightLevel = track.position().altitude() / 100;
    const QString flStr = QString::number(flightLevel);
    
    // Use thinner, sharper font (no bold, smaller size)
    painter.setFont(QFont{QStringLiteral("Monospace"), 8, QFont::Normal});
    const QFontMetrics fm{painter.font()};
    
    // Calculate label dimensions
    const int callsignWidth = fm.horizontalAdvance(callsign);
    const int flWidth = fm.horizontalAdvance(flStr);
    const int maxWidth = std::max(callsignWidth, flWidth);
    const int lineHeight = fm.height();
    const int totalHeight = lineHeight * 2;
    
    // Initial label position (further from aircraft for leader line)
    QRectF labelRect{pos.x() + 25, pos.y() - lineHeight, 
                     static_cast<double>(maxWidth + 4), static_cast<double>(totalHeight)};
    
    // Calculate heading for perpendicular positioning
    const double heading = track.velocity().heading() * M_PI / 180.0;
    
    // Find non-overlapping position, prioritizing perpendicular to velocity vector
    QPointF adjustedPos = findNonOverlappingPosition(pos, labelRect, occupiedRects, heading);
    labelRect.moveTo(adjustedPos);
    
    // Draw thin leader line from aircraft center to label with appropriate color
    QPointF labelCenter{adjustedPos.x() + labelRect.width() / 2, 
                        adjustedPos.y() + totalHeight / 2};
    painter.setPen(QPen{labelColor, 0.8});
    painter.drawLine(pos, labelCenter);
    
    // Draw labels with thin pen
    painter.setPen(QPen{labelColor, 0.5});
    painter.drawText(QPointF{adjustedPos.x(), adjustedPos.y() + lineHeight - 2}, callsign);
    painter.drawText(QPointF{adjustedPos.x(), adjustedPos.y() + lineHeight * 2 - 2}, flStr);
    
    // Add this label's rect to occupied list
    newOccupiedRects.push_back(labelRect);
}

QPointF RadarView::findNonOverlappingPosition(
    const QPointF& aircraftPos,
    const QRectF& labelRect,
    const std::vector<QRectF>& occupiedRects,
    double headingRadians) const
{
    // Calculate perpendicular directions (90° left and right from heading)
    // Perpendicular right: heading + 90° = heading + π/2
    // Perpendicular left: heading - 90° = heading - π/2
    const double perpRight = headingRadians + M_PI / 2.0;
    const double perpLeft = headingRadians - M_PI / 2.0;
    const double distance = 30.0;
    
    // Priority: perpendicular positions first, then other positions
    // Perpendicular right and left are calculated based on actual heading
    const std::vector<QPointF> candidateOffsets = {
        // Perpendicular positions (priority)
        {distance * std::sin(perpRight), -distance * std::cos(perpRight) - labelRect.height() / 2},  // perpendicular right
        {distance * std::sin(perpLeft) - labelRect.width(), -distance * std::cos(perpLeft) - labelRect.height() / 2},   // perpendicular left
        
        // Fallback positions
        {25, -labelRect.height() / 2},          // right
        {-labelRect.width() - 25, -labelRect.height() / 2}, // left
        {30, -labelRect.height() - 10},         // top-right
        {30, 10},                                // bottom-right
        {-labelRect.width() - 30, -labelRect.height() - 10}, // top-left
        {-labelRect.width() - 30, 10},          // bottom-left
        {-labelRect.width() / 2, -labelRect.height() - 25}, // top
        {-labelRect.width() / 2, 25}            // bottom
    };
    
    for (const auto& offset : candidateOffsets) {
        QPointF candidate = aircraftPos + offset;
        QRectF testRect = labelRect;
        testRect.moveTo(candidate);
        
        // Check if this position overlaps with any occupied rect
        bool overlaps = false;
        for (const auto& occupied : occupiedRects) {
            if (testRect.intersects(occupied)) {
                overlaps = true;
                break;
            }
        }
        
        // Also check if within screen bounds
        if (!overlaps && testRect.left() >= 0 && testRect.right() < width() &&
            testRect.top() >= 0 && testRect.bottom() < height()) {
            return candidate;
        }
    }
    
    // If all positions are occupied, return default (right position)
    return aircraftPos + candidateOffsets[0];
}

void RadarView::drawHoverTooltip(QPainter& painter, const domain::Track& track,
                                 QPointF screenPos) const
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

    const QFont font{QStringLiteral("Monospace"), 9, QFont::Normal};
    painter.setFont(font);
    const QFontMetrics fm{font};
    const QRectF textBounds = fm.boundingRect(
        QRect{0, 0, 300, 200},
        Qt::AlignLeft | Qt::TextWordWrap,
        text).adjusted(-8, -6, 8, 6);

    constexpr double k_offset = 20.0;
    double tx = screenPos.x() + k_offset;
    double ty = screenPos.y() - textBounds.height() / 2.0;

    if (tx + textBounds.width() > width() - 4) {
        tx = screenPos.x() - k_offset - textBounds.width();
    }
    ty = std::clamp(ty, 4.0, static_cast<double>(height()) - textBounds.height() - 4);

    const QRectF box{tx, ty, textBounds.width(), textBounds.height()};

    painter.setBrush(QColor{0, 0, 0, 220});
    painter.setPen(QPen{QColor{0, 255, 100}, 1.0});
    painter.drawRoundedRect(box, 4, 4);

    painter.setPen(QPen{QColor{0, 255, 100}, 0.5});
    painter.drawText(box.adjusted(8, 6, -8, -6),
                     Qt::AlignLeft | Qt::TextWordWrap, text);
}

void RadarView::updateTrails()
{
    for (const auto& track : m_tracks) {
        auto& trail = m_trails[track->id()];
        const auto& currentPos = track->position();
        
        // Add current geographic position to trail
        // Check distance in geographic coordinates to avoid adding duplicate points
        if (trail.positions.empty()) {
            trail.positions.push_back(currentPos);
        } else {
            const auto& lastPos = trail.positions.back();
            const double latDiff = currentPos.latitude() - lastPos.latitude();
            const double lonDiff = currentPos.longitude() - lastPos.longitude();
            const double distance = std::sqrt(latDiff * latDiff + lonDiff * lonDiff);
            
            // Only add if moved significantly (0.045 degrees ~= 5km for distinct dots)
            if (distance > 0.045) {
                trail.positions.push_back(currentPos);
                
                // Limit trail length
                if (trail.positions.size() > k_maxTrailPoints) {
                    trail.positions.pop_front();
                }
            }
        }
    }
}

} // namespace cwp::ui

