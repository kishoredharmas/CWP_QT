#include "ui/FlightListView.hpp"

#include <chrono>

#include <QString>
#include <QHeaderView>
#include <QTableWidget>
#include <QVBoxLayout>

namespace cwp::ui {

namespace {
    constexpr int k_colCallsign = 0;
    constexpr int k_colTrackId  = 1;
    constexpr int k_colLat      = 2;
    constexpr int k_colLon      = 3;
    constexpr int k_colAlt      = 4;
    constexpr int k_colSpeed    = 5;
    constexpr int k_colHeading  = 6;
    constexpr int k_colAge      = 7;
    constexpr int k_columnCount = 8;
} // namespace

FlightListView::FlightListView(QWidget* parent)
    : QWidget{parent}
{
    setupUi();
}

void FlightListView::setupUi()
{
    m_table = new QTableWidget{0, k_columnCount, this};
    m_table->setHorizontalHeaderLabels(
        {"Callsign", "Track ID", "Lat", "Lon",
         "Alt (ft)", "Speed (kt)", "Heading (°)", "Age (s)"});
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSortingEnabled(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->setAlternatingRowColors(true);

    setStyleSheet(
        "QTableWidget { background:#0d0d1a; color:#ccddff; "
        "  gridline-color:#222244; font-family:monospace; font-size:12px; }"
        "QHeaderView::section { background:#111133; color:#8899ff; "
        "  border:1px solid #222255; padding:4px; }"
        "QTableWidget::item:selected { background:#1a1a44; }");

    auto* layout = new QVBoxLayout{this};
    layout->setContentsMargins(4, 4, 4, 4);
    layout->addWidget(m_table);
}

void FlightListView::updateTracks(
    std::span<const std::shared_ptr<domain::Track>> tracks)
{
    m_table->setSortingEnabled(false);
    m_table->setRowCount(0);

    const auto now = std::chrono::system_clock::now();

    for (const auto& track : tracks) {
        const int row = m_table->rowCount();
        m_table->insertRow(row);

        const QString callsign = track->callsign().has_value()
            ? QString::fromStdString(*track->callsign())
            : QStringLiteral("---");

        const auto ageSecs =
            std::chrono::duration_cast<std::chrono::seconds>(
                now - track->timestamp()).count();

        m_table->setItem(row, k_colCallsign,
            new QTableWidgetItem{callsign});
        m_table->setItem(row, k_colTrackId,
            new QTableWidgetItem{QString::number(track->id().value)});
        m_table->setItem(row, k_colLat,
            new QTableWidgetItem{
                QString::number(track->position().latitude(), 'f', 4)});
        m_table->setItem(row, k_colLon,
            new QTableWidgetItem{
                QString::number(track->position().longitude(), 'f', 4)});
        m_table->setItem(row, k_colAlt,
            new QTableWidgetItem{
                QString::number(track->position().altitude())});
        m_table->setItem(row, k_colSpeed,
            new QTableWidgetItem{
                QString::number(track->velocity().speed(), 'f', 0)});
        m_table->setItem(row, k_colHeading,
            new QTableWidgetItem{
                QString::number(track->velocity().heading(), 'f', 0)});
        m_table->setItem(row, k_colAge,
            new QTableWidgetItem{QString::number(ageSecs)});
    }

    m_table->setSortingEnabled(true);
}

void FlightListView::removeTrack(domain::TrackId id)
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
