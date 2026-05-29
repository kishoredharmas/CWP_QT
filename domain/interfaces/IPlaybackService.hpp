#pragma once

#include <expected>
#include <string>

#include "domain/errors/DomainError.hpp"

namespace cwp::domain {

/**
 * @brief Abstract service interface for ATC session recording and playback.
 *
 * Replaces the legacy RPS (Recording & Playback System). All fallible
 * operations return std::expected (C++23) so callers are forced to handle
 * error cases — no silent failures in safety-critical recording paths.
 *
 * Concrete implementations live in the infrastructure layer.
 */
class IPlaybackService {
public:
    virtual ~IPlaybackService() = default;

    /**
     * @brief Begins recording session data to the specified file.
     *
     * Any prior active recording is automatically stopped first (FR-33).
     *
     * @param filePath Absolute path of the output recording file.
     * @return std::unexpected(PlaybackError::FileNotAccessible) if the file
     *         cannot be created or opened.
     */
    [[nodiscard]] virtual
    std::expected<void, PlaybackError>
    startRecording(const std::string& filePath) = 0;

    /// Stops the active recording and flushes all buffered data to disk.
    virtual void stopRecording() = 0;

    /**
     * @brief Begins replaying a previously recorded session.
     *
     * @param filePath Absolute path of the recording file to replay.
     * @return std::unexpected(PlaybackError::FileNotAccessible) if the file
     *         cannot be opened.
     */
    [[nodiscard]] virtual
    std::expected<void, PlaybackError>
    startPlayback(const std::string& filePath) = 0;

    /// Stops the active playback.
    virtual void stopPlayback() = 0;

    /// @return True if a recording is currently in progress.
    [[nodiscard]] virtual bool isRecording() const noexcept = 0;

    /// @return True if a playback session is currently in progress.
    [[nodiscard]] virtual bool isPlaying() const noexcept = 0;
};

} // namespace cwp::domain
