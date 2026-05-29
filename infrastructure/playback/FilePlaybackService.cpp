#include "infrastructure/playback/FilePlaybackService.hpp"

namespace cwp::infrastructure {

FilePlaybackService::~FilePlaybackService()
{
    if (m_recording.load(std::memory_order_acquire)) {
        stopRecording();
    }
    if (m_playing.load(std::memory_order_acquire)) {
        stopPlayback();
    }
}

std::expected<void, domain::PlaybackError>
FilePlaybackService::startRecording(const std::string& filePath)
{
    if (m_recording.load(std::memory_order_acquire)) {
        stopRecording(); // auto-stop prior session (FR-33)
    }
    m_recordingStream.open(filePath, std::ios::binary | std::ios::trunc);
    if (!m_recordingStream.is_open()) {
        return std::unexpected{domain::PlaybackError::FileNotAccessible};
    }
    m_recording.store(true, std::memory_order_release);
    return {};
}

void FilePlaybackService::stopRecording()
{
    m_recordingStream.flush();
    m_recordingStream.close();
    m_recording.store(false, std::memory_order_release);
}

std::expected<void, domain::PlaybackError>
FilePlaybackService::startPlayback(const std::string& filePath)
{
    if (m_playing.load(std::memory_order_acquire)) {
        stopPlayback();
    }
    m_playbackStream.open(filePath, std::ios::binary);
    if (!m_playbackStream.is_open()) {
        return std::unexpected{domain::PlaybackError::FileNotAccessible};
    }
    m_playing.store(true, std::memory_order_release);
    return {};
}

void FilePlaybackService::stopPlayback()
{
    m_playbackStream.close();
    m_playing.store(false, std::memory_order_release);
}

bool FilePlaybackService::isRecording() const noexcept
{
    return m_recording.load(std::memory_order_acquire);
}

bool FilePlaybackService::isPlaying() const noexcept
{
    return m_playing.load(std::memory_order_acquire);
}

} // namespace cwp::infrastructure
