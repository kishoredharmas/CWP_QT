#pragma once

#include <memory>
#include <optional>
#include <vector>

#include <QColor>
#include <QPointF>
#include <QWidget>

#include "domain/entities/Track.hpp"

namespace cwp::ui {

/**
 * @brief Custom widget that renders the radar plan-view display.
 *
 * Paints all active tracks as cross symbols with associated data labels
 * using QPainter. Geographic coordinates are mapped to screen space via
 * a simple equirectangular projection centred on a configurable origin.
 *
 * Features:
 *  - Each track is rendered in a distinct colour from a fixed ATC palette.
 *  - Hovering over a track symbol shows an enlarged tooltip label with full
 *    details (callsign, FL, speed, heading, lat/lon) so data never overlaps.
 *
 * This widget is purely presentational; it holds no business logic.
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
    static constexpr double k_centerLat    {51.5};  ///< Display centre latitude (°).
    static constexpr double k_centerLon    { 0.0};  ///< Display centre longitude (°).
    static constexpr double k_scalePixPerDeg{40.0}; ///< Pixels per degree of arc.
    static constexpr int    k_rangeRingCount{5};    ///< Number of range rings.
    static constexpr int    k_symbolHalfSize{6};    ///< Half-size of cross symbol (px).
    static constexpr double k_hoverRadiusPx{18.0};  ///< Pixel radius for hover hit-test.

    // ── ATC label colour palette ──────────────────────────────────────────────
    /// Distinct colours assigned round-robin to tracks in arrival order.
    static const std::array<QColor, 8> k_trackColours;

    std::vector<std::shared_ptr<domain::Track>> m_tracks;

    /// Index of the hovered track in m_tracks, or std::nullopt.
    std::optional<std::size_t> m_hoveredIndex;

    [[nodiscard]] QPointF project(const domain::Position& position) const;
    [[nodiscard]] QColor  colourForIndex(std::size_t index) const;

    void drawRangeRings(QPainter& painter) const;
    void drawTrack(QPainter& painter, const domain::Track& track,
                   QColor colour, bool hovered) const;
    void drawHoverTooltip(QPainter& painter, const domain::Track& track,
                          QPointF screenPos, QColor colour) const;
};

} // namespace cwp::ui
