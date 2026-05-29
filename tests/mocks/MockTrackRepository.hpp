#pragma once

#include <gmock/gmock.h>

#include "domain/interfaces/ITrackRepository.hpp"

namespace cwp::tests::mocks {

/**
 * @brief Google Mock stub for ITrackRepository.
 *
 * Injected into application-layer unit tests to isolate use cases from any
 * concrete storage implementation.
 */
class MockTrackRepository : public domain::ITrackRepository {
public:
    MOCK_METHOD(std::vector<std::shared_ptr<domain::Track>>,
                findAll,  (), (const, override));

    MOCK_METHOD((std::optional<std::shared_ptr<domain::Track>>),
                findById, (domain::TrackId id), (const, override));

    MOCK_METHOD(void, save,   (std::shared_ptr<domain::Track> track), (override));
    MOCK_METHOD(void, remove, (domain::TrackId id),                   (override));
};

} // namespace cwp::tests::mocks
