#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <chrono>
#include <memory>

#include "application/use_cases/AssociateFlightPlanUseCase.hpp"
#include "domain/entities/FlightPlan.hpp"
#include "domain/entities/Track.hpp"
#include "domain/errors/DomainError.hpp"
#include "tests/mocks/MockTrackRepository.hpp"

using namespace cwp;
using namespace testing;

namespace {

std::shared_ptr<domain::Track> makeTrack(domain::TrackId id)
{
    return std::make_shared<domain::Track>(
        id,
        domain::Position{51.5, 0.0, 35000},
        domain::Velocity{450.0, 270.0, 0},
        std::chrono::system_clock::now());
}

domain::FlightPlan makeFlightPlan()
{
    return domain::FlightPlan{"BAW456", "B738", "5432", 370, {"EGLL", "WOTAN", "EDDF"}};
}

} // namespace

// ── Success path ──────────────────────────────────────────────────────────────

TEST(AssociateFlightPlanUseCaseTest, ReturnsSuccessAndSetsCallsignWhenTrackExists)
{
    auto mockRepo    = std::make_shared<tests::mocks::MockTrackRepository>();
    const auto track = makeTrack(domain::TrackId{7U});

    EXPECT_CALL(*mockRepo, findById(domain::TrackId{7U}))
        .WillOnce(Return(std::optional{track}));

    EXPECT_CALL(*mockRepo, save(_)).Times(1);

    application::AssociateFlightPlanUseCase useCase{mockRepo};
    const auto result = useCase.execute(domain::TrackId{7U}, makeFlightPlan());

    // std::expected: must have a value (success)
    EXPECT_TRUE(result.has_value());
    ASSERT_TRUE(track->callsign().has_value());
    EXPECT_EQ(*track->callsign(), "BAW456");
}

// ── Failure path ──────────────────────────────────────────────────────────────

TEST(AssociateFlightPlanUseCaseTest, ReturnsTrackNotFoundWhenTrackDoesNotExist)
{
    auto mockRepo = std::make_shared<tests::mocks::MockTrackRepository>();

    EXPECT_CALL(*mockRepo, findById(domain::TrackId{99U}))
        .WillOnce(Return(std::nullopt));

    EXPECT_CALL(*mockRepo, save(_)).Times(0);

    application::AssociateFlightPlanUseCase useCase{mockRepo};
    const auto result = useCase.execute(domain::TrackId{99U}, makeFlightPlan());

    // std::expected: must carry the error code, not a value
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), domain::AssociationError::TrackNotFound);
}

// ── Callsign content ──────────────────────────────────────────────────────────

TEST(AssociateFlightPlanUseCaseTest, CallsignMatchesFlightPlanCallsign)
{
    auto mockRepo    = std::make_shared<tests::mocks::MockTrackRepository>();
    const auto track = makeTrack(domain::TrackId{3U});

    EXPECT_CALL(*mockRepo, findById(domain::TrackId{3U}))
        .WillOnce(Return(std::optional{track}));
    EXPECT_CALL(*mockRepo, save(_)).Times(1);

    const domain::FlightPlan fp{"SAS123", "A319", "3210", 310, {}};
    application::AssociateFlightPlanUseCase useCase{mockRepo};
    useCase.execute(domain::TrackId{3U}, fp);

    ASSERT_TRUE(track->callsign().has_value());
    EXPECT_EQ(*track->callsign(), "SAS123");
}

// ── Re-association ─────────────────────────────────────────────────────────────

TEST(AssociateFlightPlanUseCaseTest, ReassociationOverwritesPreviousCallsign)
{
    auto mockRepo    = std::make_shared<tests::mocks::MockTrackRepository>();
    const auto track = makeTrack(domain::TrackId{5U});

    EXPECT_CALL(*mockRepo, findById(domain::TrackId{5U}))
        .WillRepeatedly(Return(std::optional{track}));
    EXPECT_CALL(*mockRepo, save(_)).Times(2);

    application::AssociateFlightPlanUseCase useCase{mockRepo};
    useCase.execute(domain::TrackId{5U},
                    domain::FlightPlan{"DLH100", "A320", "1000", 350, {}});
    useCase.execute(domain::TrackId{5U},
                    domain::FlightPlan{"DLH200", "A321", "2000", 360, {}});

    ASSERT_TRUE(track->callsign().has_value());
    EXPECT_EQ(*track->callsign(), "DLH200");
}

