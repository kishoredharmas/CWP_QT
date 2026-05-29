#include "ui/FlightStripView.hpp"

#include <QString>
#include <QHeaderView>
#include <QTableWidget>
#include <QVBoxLayout>

namespace cwp::ui {

namespace {
    constexpr int k_colCallsign  = 0;
    constexpr int k_colTrackId   = 1;
    constexpr int k_colAltitude  = 2;
    constexpr int k_colSpeed     = 3;
    constexpr int k_colHeading   = 4;
    constexpr int k_columnCount  = 5;
} // namespace

FlightStripView::FlightStripView(QWidget* parent)
    : QWidget{parent}
{
    setupUi();
}

void FlightStripView::setupUi()
{
    m_table = new QTableWidget{0, k_columnCount, this};
    m_table->setHorizontalHeaderLabels(
        {"Callsign", "Track ID", "Alt (ft)", "Speed (kt)", "Heading (°)"});
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSortingEnabled(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->setAlternatingRowColors(true);

    // ATC-style dark theme for the strip panel
    setStyleSheet(
        "QTableWidget { background:#1a1a2e; color:#00ff88; "
        "  gridline-color:#003300; font-family:monospace; font-size:12px; }"
        "QHeaderView::section { background:#003300; color:#00ff88; "
        "  border:1px solid #005500; padding:4px; }"
        "QTableWidget::item:selected { background:#004400; }");

    auto* layout = new QVBoxLayout{this};
    layout->setContentsMargins(4, 4, 4, 4);
    layout->addWidget(m_table);
}

void FlightStripView::updateTracks(
    std::span<const std::shared_ptr<domain::Track>> tracks)
{
    // Disable sorting during update to avoid row index shifts mid-insert.
    m_table->setSortingEnabled(false);
    m_table->setRowCount(0);

    for (const auto& track : tracks) {
        if (!track->callsign().has_value()) {
            continue; // Only show strips for associated tracks.
        }

        const int row = m_table->rowCount();
        m_table->insertRow(row);

        m_table->setItem(row, k_colCallsign,
            new QTableWidgetItem{QString::fromStdString(*track->callsign())});
        m_table->setItem(row, k_colTrackId,
            new QTableWidgetItem{QString::number(track->id().value)});
        m_table->setItem(row, k_colAltitude,
            new QTableWidgetItem{QString::number(track->position().altitude())});
        m_table->setItem(row, k_colSpeed,
            new QTableWidgetItem{
                QString::number(track->velocity().speed(), 'f', 0)});
        m_table->setItem(row, k_colHeading,
            new QTableWidgetItem{
                QString::number(track->velocity().heading(), 'f', 0)});
    }

    m_table->setSortingEnabled(true);
}

void FlightStripView::removeTrack(domain::TrackId id)
{
    for (int row = 0; row < m_table->rowCount(); ++row) {
        const auto* item = m_table->item(row, k_colTrackId);
        if (item && item->text().toUInt() == id.value) {
            m_table->removeRow(row);
            return;
        }
    }
}

} // namespace cwp::ui
