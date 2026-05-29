#pragma once

#include <memory>
#include <span>
#include <vector>

#include <QWidget>

#include "domain/entities/Track.hpp"

class QTableWidget;

namespace cwp::ui {

/**
 * @brief Sortable table of all currently tracked aircraft.
 *
 * Shows all tracks (associated or not), with position, velocity, and age
 * columns. Intended as an overview complement to the RadarView — a controller
 * can sort by altitude, speed or heading to quickly find specific traffic.
 *
 * The view is purely presentational — it holds no business logic.
 */
class FlightListView final : public QWidget {
    Q_OBJECT

public:
    explicit FlightListView(QWidget* parent = nullptr);

    /// Rebuilds the list from the current track snapshot.
    void updateTracks(std::span<const std::shared_ptr<domain::Track>> tracks);

    /// Removes the row matching the given track ID (if present).
    void removeTrack(domain::TrackId id);

private:
    void setupUi();

    QTableWidget* m_table{nullptr};
};

} // namespace cwp::ui
