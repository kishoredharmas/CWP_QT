#pragma once

#include <memory>
#include <span>
#include <vector>

#include "domain/entities/Track.hpp"

namespace cwp::domain {

/**
 * @brief Output boundary for pushing track data to the presentation layer.
 *
 * The application layer drives the UI exclusively through this interface,
 * keeping business logic decoupled from any Qt or rendering concern.
 *
 * presentTracks() accepts a std::span (C++23) to convey a read-only, non-
 * owning view of the current snapshot — zero-copy on the hot radar refresh
 * path. Implementors that need to store the data must copy it explicitly.
 */
class ITrackPresenter {
public:
    virtual ~ITrackPresenter() = default;

    /**
     * @brief Delivers a fresh snapshot of all active tracks to the UI.
     *
     * @param tracks Read-only view of the current track set. The span is only
     *               valid for the duration of this call.
     */
    virtual void presentTracks(
        std::span<const std::shared_ptr<Track>> tracks) = 0;

    /**
     * @brief Notifies the UI that a specific track has been dropped.
     * @param id Identifier of the removed track.
     */
    virtual void presentTrackRemoved(TrackId id) = 0;
};

} // namespace cwp::domain
