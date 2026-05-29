#pragma once

#include <atomic>
#include <expected>
#include <fstream>
#include <string>

#include "domain/interfaces/IPlaybackService.hpp"

namespace cwp::infrastructure {

/**
 * @brief Thread-safe file-based ATC session recorder and replayer.
 *
 * Safety hardening over the naive implementation:
 *
 * - **std::atomic<bool>** flags: `isRecording()` and `isPlaying()` are
 *   lock-free and safe to query from any thread without a race condition.
 *
 * - **std::expected** return values: no exceptions escape into caller code;
 *   all error conditions are communicated through the type system, forcing
 *   the UI to handle them explicitly (FR-35).
 *
 * Only one mode (recording or playback) may be active at a time. Starting
 * a new recording automatically stops any prior recording (FR-33).
 */
class FilePlaybackService final : public domain::IPlaybackService {
public:
    FilePlaybackService() = default;

    /// Stops any active recording or playback on destruction.
    ~FilePlaybackService() override;

    // Non-copyable, non-movable — file streams must not be duplicated.
    FilePlaybackService(const FilePlaybackService&)            = delete;
    FilePlaybackService& operator=(const FilePlaybackService&) = delete;
    FilePlaybackService(FilePlaybackService&&)                 = delete;
    FilePlaybackService& operator=(FilePlaybackService&&)      = delete;

    [[nodiscard]] std::expected<void, domain::PlaybackError>
    startRecording(const std::string& filePath) override;

    void stopRecording() override;

    [[nodiscard]] std::expected<void, domain::PlaybackError>
    startPlayback(const std::string& filePath) override;

    void stopPlayback() override;

    /// Lock-free: safe to query from any thread.
    [[nodiscard]] bool isRecording() const noexcept override;

    /// Lock-free: safe to query from any thread.
    [[nodiscard]] bool isPlaying()   const noexcept override;

private:
    std::ofstream        m_recordingStream;
    std::ifstream        m_playbackStream;
    std::atomic<bool>    m_recording{false}; ///< Lock-free recording flag.
    std::atomic<bool>    m_playing  {false}; ///< Lock-free playback flag.
};

} // namespace cwp::infrastructure
