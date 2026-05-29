#pragma once

#include <memory>
#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "domain/interfaces/ITrackRepository.hpp"

namespace cwp::infrastructure {

/**
 * @brief Thread-safe in-memory track repository with JSON-file-backed persistence.
 *
 * Safety contract: satisfies the thread-safety requirement of ITrackRepository.
 * Reads are guarded by a shared lock (multiple concurrent readers allowed);
 * writes are guarded by an exclusive lock. This matches the typical ATC workload
 * where the radar-feed thread writes infrequently while the UI reads on every
 * 4-second refresh cycle.
 *
 * Holds the live track set in an unordered_map for O(1) average look-up.
 * On every mutating operation the state is serialised to a JSON file so that
 * the current session can be resumed after a restart (FR-40, FR-41, FR-42).
 */
class FileTrackRepository final : public domain::ITrackRepository {
public:
    /**
     * @brief Constructs the repository and loads any persisted state.
     * @param filePath Path to the JSON backing file; created if absent.
     */
    explicit FileTrackRepository(const std::string& filePath);

    [[nodiscard]] std::vector<std::shared_ptr<domain::Track>>
        findAll() const override;

    [[nodiscard]] std::optional<std::shared_ptr<domain::Track>>
        findById(domain::TrackId id) const override;

    void save(std::shared_ptr<domain::Track> track) override;

    void remove(domain::TrackId id) override;

private:
    // std::hash<TrackId> is provided via the specialisation in Track.hpp.
    using TrackMap =
        std::unordered_map<domain::TrackId,
                           std::shared_ptr<domain::Track>,
                           std::hash<domain::TrackId>>;

    std::string            m_filePath;
    TrackMap               m_tracks;
    mutable std::shared_mutex m_mutex; ///< Guards m_tracks for concurrent access.

    /// Loads track records from the JSON backing file (no-op if absent).
    void loadFromFile();

    /// Serialises the current in-memory state to the JSON backing file.
    /// Must be called with the exclusive lock held.
    void persistToFile() const;
};

} // namespace cwp::infrastructure
