#pragma once

#include <memory>
#include <vector>

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
 * This widget is purely presentational; it holds no business logic.
 */
class RadarView final : public QWidget {
    Q_OBJECT

public:
    explicit RadarView(QWidget* parent = nullptr);

    /**
     * @brief Replaces the current displayed track set and triggers a repaint.
     * @param tracks New track snapshot delivered by the application layer.
     */
    void updateTracks(std::vector<std::shared_ptr<domain::Track>> tracks);

    /**
     * @brief Removes a single track from the display and triggers a repaint.
     * @param id Identifier of the track to remove.
     */
    void removeTrack(domain::TrackId id);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    static constexpr double k_centerLat{51.5};      ///< Display centre latitude (degrees).
    static constexpr double k_centerLon{0.0};        ///< Display centre longitude (degrees).
    static constexpr double k_scalePixPerDeg{40.0};  ///< Pixels per degree of arc.
    static constexpr int    k_rangeRingCount{5};     ///< Number of range rings to draw.
    static constexpr int    k_symbolHalfSize{6};     ///< Half-size of the track cross (px).

    std::vector<std::shared_ptr<domain::Track>> m_tracks;

    /**
     * @brief Projects a geographic position to widget-local pixel coordinates.
     * @param position Geographic position to project.
     * @return Screen-space point as QPointF.
     */
    [[nodiscard]] QPointF project(const domain::Position& position) const;

    /// Renders a single track symbol and its data label.
    void drawTrack(QPainter& painter, const domain::Track& track) const;

    /// Renders decorative range rings centred on the display origin.
    void drawRangeRings(QPainter& painter) const;
};

} // namespace cwp::ui
