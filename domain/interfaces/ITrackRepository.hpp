#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "domain/entities/Track.hpp"

namespace cwp::domain {

/**
 * @brief Abstract repository interface for radar track storage.
 *
 * **Thread-safety contract**: all concrete implementations MUST be thread-safe.
 * The radar feed producer (network thread) and the UI consumer (main thread)
 * can both call this interface concurrently. Implementors are required to
 * protect internal state with a std::shared_mutex (shared lock for reads,
 * exclusive lock for writes).
 *
 * Application layer code depends solely on this interface (DIP). Concrete
 * implementations reside in the infrastructure layer and are injected at
 * the composition root (main).
 */
class ITrackRepository {
public:
    virtual ~ITrackRepository() = default;

    /**
     * @brief Returns a snapshot of all currently active tracks.
     *
     * Thread-safe: acquires a shared (read) lock internally.
     * Returning a value (not a reference) ensures the caller owns a
     * consistent, immutable snapshot even if the repository is mutated
     * concurrently on another thread.
     */
    [[nodiscard]] virtual std::vector<std::shared_ptr<Track>> findAll() const = 0;

    /**
     * @brief Looks up a single track by its strong TrackId.
     *
     * Thread-safe: acquires a shared (read) lock internally.
     *
     * @param id  Strong TrackId — cannot be confused with altitude or squawk.
     * @return The track or std::nullopt if absent.
     */
    [[nodiscard]] virtual std::optional<std::shared_ptr<Track>>
        findById(TrackId id) const = 0;

    /**
     * @brief Inserts or replaces a track (upsert semantics).
     *
     * Thread-safe: acquires an exclusive (write) lock internally.
     */
    virtual void save(std::shared_ptr<Track> track) = 0;

    /**
     * @brief Removes the track with the given identifier.
     *
     * Thread-safe: acquires an exclusive (write) lock internally.
     */
    virtual void remove(TrackId id) = 0;
};

} // namespace cwp::domain
