#pragma once

#include <deque>
#include <map>
#include <memory>
#include <optional>
#include <vector>

#include <QColor>
#include <QPointF>
#include <QWidget>

#include "domain/entities/Track.hpp"

namespace cwp::ui {

/**
 * @brief Realistic ATC radar display with sector boundaries and aircraft trails.
 *
 * Features:
 *  - Dark-themed background simulating a real radar scope
 *  - Irregular sector polygon (control airspace boundary)
 *  - Air route lines overlaid on the display
 *  - Aircraft rendered as white dots with green velocity vectors
 *  - Dotted white trails showing aircraft history
 *  - Green labels with callsign and flight level
 *  - Minimalist, radar-authentic styling
 */
class RadarView final : public QWidget {
    Q_OBJECT

public:
    explicit RadarView(QWidget* parent = nullptr);

    void updateTracks(std::vector<std::shared_ptr<domain::Track>> tracks);
    void removeTrack(domain::TrackId id);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    // ── Layout constants ──────────────────────────────────────────────────────
    static constexpr double k_centerLat     {51.5};   ///< Display centre latitude (°).
    static constexpr double k_centerLon     { 0.0};   ///< Display centre longitude (°).
    static constexpr double k_scalePixPerDeg{50.0};   ///< Pixels per degree of arc.
    static constexpr int    k_dotRadius     {3};      ///< Aircraft dot radius (px).
    static constexpr double k_vectorLength  {40.0};   ///< Velocity vector length (px).
    static constexpr double k_hoverRadiusPx {20.0};   ///< Pixel radius for hover hit-test.
    static constexpr int    k_maxTrailPoints{5};      ///< Max trail history per aircraft.
    static constexpr int    k_trailDotRadius{1};      ///< Trail dot radius (px).

    struct TrailHistory {
        std::deque<domain::Position> positions;  // Store geographic positions
    };

    std::vector<std::shared_ptr<domain::Track>> m_tracks;
    std::map<domain::TrackId, TrailHistory>     m_trails;

    /// Index of the hovered track in m_tracks, or std::nullopt.
    std::optional<std::size_t> m_hoveredIndex;

    [[nodiscard]] QPointF project(const domain::Position& position) const;
    [[nodiscard]] QPointF findNonOverlappingPosition(
        const QPointF& aircraftPos,
        const QRectF& labelRect,
        const std::vector<QRectF>& occupiedRects,
        double headingRadians) const;

    void drawBackground(QPainter& painter) const;
    void drawSector(QPainter& painter) const;
    void drawRoutes(QPainter& painter) const;
    void drawTrail(QPainter& painter, const TrailHistory& trail,
                   bool isInsideSector) const;
    void drawAircraft(QPainter& painter, const domain::Track& track,
                      const QPointF& pos, bool hovered) const;
    void drawLabel(QPainter& painter, const domain::Track& track,
                   const QPointF& pos, const std::vector<QRectF>& occupiedRects,
                   std::vector<QRectF>& newOccupiedRects) const;
    void drawHoverTooltip(QPainter& painter, const domain::Track& track,
                          QPointF screenPos) const;

    void updateTrails();
};

} // namespace cwp::ui
