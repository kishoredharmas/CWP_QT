#pragma once

#include <memory>
#include <span>
#include <vector>

#include <QWidget>

#include "domain/entities/Track.hpp"

class QTableWidget;

namespace cwp::ui {

/**
 * @brief Displays the active flight strips as a tabular list.
 *
 * A flight strip shows the association between a radar track and its
 * corresponding flight plan. Strips are shown only for tracks that have
 * an associated callsign (i.e. AssociateFlightPlanUseCase has been run).
 *
 * The view is purely presentational — it holds no business logic.
 */
class FlightStripView final : public QWidget {
    Q_OBJECT

public:
    explicit FlightStripView(QWidget* parent = nullptr);

    /// Rebuilds the strip table from the current track snapshot.
    void updateTracks(std::span<const std::shared_ptr<domain::Track>> tracks);

    /// Removes the strip row matching the given track ID (if present).
    void removeTrack(domain::TrackId id);

private:
    void setupUi();

    QTableWidget* m_table{nullptr};
};

} // namespace cwp::ui
