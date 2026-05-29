#include "ui/PlaybackView.hpp"

#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include <utility>

namespace cwp::ui {

PlaybackView::PlaybackView(
    std::shared_ptr<domain::IPlaybackService> playbackService,
    QWidget* parent)
    : QWidget{parent}
    , m_playbackService{std::move(playbackService)}
{
    setupUi();
    updateButtonStates();
}

void PlaybackView::setupUi()
{
    setStyleSheet(
        "QWidget { background:#111; color:#ccc; font-family:monospace; }"
        "QGroupBox { border:1px solid #444; margin-top:8px; color:#aaa; }"
        "QGroupBox::title { subcontrol-origin:margin; left:8px; }"
        "QPushButton { background:#222; color:#0f0; border:1px solid #0a0; "
        "  padding:6px 14px; min-width:100px; }"
        "QPushButton:hover  { background:#1a3a1a; }"
        "QPushButton:disabled { color:#444; border-color:#333; }"
        "QLineEdit { background:#1a1a1a; color:#0f0; border:1px solid #444; "
        "  padding:4px; }");

    // ── File path row ────────────────────────────────────────────────────────
    auto* fileGroup  = new QGroupBox{QStringLiteral("Session file"), this};
    auto* fileLayout = new QHBoxLayout{fileGroup};

    m_filePathEdit = new QLineEdit{QStringLiteral("session.cwp"), fileGroup};
    auto* browseBtn = new QPushButton{QStringLiteral("Browse…"), fileGroup};
    fileLayout->addWidget(m_filePathEdit);
    fileLayout->addWidget(browseBtn);
    connect(browseBtn, &QPushButton::clicked, this, [this] {
        const QString path = QFileDialog::getSaveFileName(
            this, QStringLiteral("Session file"), {}, QStringLiteral("CWP sessions (*.cwp);;All files (*)"));
        if (!path.isEmpty()) {
            m_filePathEdit->setText(path);
        }
    });

    // ── Recording controls ───────────────────────────────────────────────────
    auto* recGroup  = new QGroupBox{QStringLiteral("Recording"), this};
    auto* recLayout = new QHBoxLayout{recGroup};

    m_recordBtn     = new QPushButton{QStringLiteral("⏺  Record"), recGroup};
    m_stopRecordBtn = new QPushButton{QStringLiteral("⏹  Stop"), recGroup};
    recLayout->addWidget(m_recordBtn);
    recLayout->addWidget(m_stopRecordBtn);
    recLayout->addStretch();

    connect(m_recordBtn,     &QPushButton::clicked, this, &PlaybackView::onStartRecording);
    connect(m_stopRecordBtn, &QPushButton::clicked, this, &PlaybackView::onStopRecording);

    // ── Playback controls ────────────────────────────────────────────────────
    auto* pbGroup  = new QGroupBox{QStringLiteral("Playback"), this};
    auto* pbLayout = new QHBoxLayout{pbGroup};

    m_playBtn     = new QPushButton{QStringLiteral("▶  Play"), pbGroup};
    m_stopPlayBtn = new QPushButton{QStringLiteral("⏹  Stop"), pbGroup};
    pbLayout->addWidget(m_playBtn);
    pbLayout->addWidget(m_stopPlayBtn);
    pbLayout->addStretch();

    connect(m_playBtn,     &QPushButton::clicked, this, &PlaybackView::onStartPlayback);
    connect(m_stopPlayBtn, &QPushButton::clicked, this, &PlaybackView::onStopPlayback);

    // ── Status label ─────────────────────────────────────────────────────────
    m_statusLabel = new QLabel{QStringLiteral("Ready."), this};
    m_statusLabel->setStyleSheet(QStringLiteral("color:#aaa; padding:4px;"));

    // ── Outer layout ─────────────────────────────────────────────────────────
    auto* outer = new QVBoxLayout{this};
    outer->setContentsMargins(16, 16, 16, 16);
    outer->setSpacing(12);
    outer->addWidget(fileGroup);
    outer->addWidget(recGroup);
    outer->addWidget(pbGroup);
    outer->addWidget(m_statusLabel);
    outer->addStretch();
}

void PlaybackView::onStartRecording()
{
    const auto path = m_filePathEdit->text().toStdString();
    const auto result = m_playbackService->startRecording(path);
    if (result.has_value()) {
        setStatus(QStringLiteral("Recording to: %1").arg(m_filePathEdit->text()));
    } else {
        setStatus(QStringLiteral("Error: cannot open file for recording."), /*isError=*/true);
    }
    updateButtonStates();
}

void PlaybackView::onStopRecording()
{
    m_playbackService->stopRecording();
    setStatus(QStringLiteral("Recording stopped."));
    updateButtonStates();
}

void PlaybackView::onStartPlayback()
{
    const auto path = m_filePathEdit->text().toStdString();
    const auto result = m_playbackService->startPlayback(path);
    if (result.has_value()) {
        setStatus(QStringLiteral("Playing: %1").arg(m_filePathEdit->text()));
    } else {
        setStatus(QStringLiteral("Error: cannot open file for playback."), /*isError=*/true);
    }
    updateButtonStates();
}

void PlaybackView::onStopPlayback()
{
    m_playbackService->stopPlayback();
    setStatus(QStringLiteral("Playback stopped."));
    updateButtonStates();
}

void PlaybackView::updateButtonStates()
{
    if (!m_playbackService) {
        return;
    }
    const bool recording = m_playbackService->isRecording();
    const bool playing   = m_playbackService->isPlaying();

    m_recordBtn->setEnabled(!recording && !playing);
    m_stopRecordBtn->setEnabled(recording);
    m_playBtn->setEnabled(!recording && !playing);
    m_stopPlayBtn->setEnabled(playing);
}

void PlaybackView::setStatus(const QString& message, bool isError)
{
    m_statusLabel->setText(message);
    m_statusLabel->setStyleSheet(
        isError ? QStringLiteral("color:#ff4444; padding:4px;")
                : QStringLiteral("color:#aaa; padding:4px;"));
}

} // namespace cwp::ui
