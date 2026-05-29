#pragma once

#include <memory>

#include <QWidget>

#include "domain/interfaces/IPlaybackService.hpp"

class QLabel;
class QLineEdit;
class QPushButton;

namespace cwp::ui {

/**
 * @brief Control panel for ATC session recording and playback.
 *
 * Provides buttons to start/stop recording and playback via the
 * IPlaybackService interface. All fallible operations display an inline
 * status message so errors are never silently swallowed.
 *
 * The view is purely presentational — it holds no business logic.
 */
class PlaybackView final : public QWidget {
    Q_OBJECT

public:
    explicit PlaybackView(
        std::shared_ptr<domain::IPlaybackService> playbackService,
        QWidget* parent = nullptr);

private slots:
    void onStartRecording();
    void onStopRecording();
    void onStartPlayback();
    void onStopPlayback();

private:
    void setupUi();
    void updateButtonStates();
    void setStatus(const QString& message, bool isError = false);

    std::shared_ptr<domain::IPlaybackService> m_playbackService;

    QLineEdit*   m_filePathEdit{nullptr};
    QPushButton* m_recordBtn{nullptr};
    QPushButton* m_stopRecordBtn{nullptr};
    QPushButton* m_playBtn{nullptr};
    QPushButton* m_stopPlayBtn{nullptr};
    QLabel*      m_statusLabel{nullptr};
};

} // namespace cwp::ui
