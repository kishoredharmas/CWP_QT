#pragma once

#include <gmock/gmock.h>
#include <span>

#include "domain/interfaces/ITrackPresenter.hpp"

namespace cwp::tests::mocks {

/**
 * @brief Google Mock stub for ITrackPresenter.
 *
 * Allows unit tests to verify that use cases correctly drive the presenter
 * output boundary without depending on Qt or any real widget.
 */
class MockTrackPresenter : public domain::ITrackPresenter {
public:
    MOCK_METHOD(void, presentTracks,
                (std::span<const std::shared_ptr<domain::Track>> tracks),
                (override));

    MOCK_METHOD(void, presentTrackRemoved, (domain::TrackId id), (override));
};

} // namespace cwp::tests::mocks
