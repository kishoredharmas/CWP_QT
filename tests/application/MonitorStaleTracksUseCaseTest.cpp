#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <chrono>
#include <memory>
#include <vector>

#include "application/use_cases/MonitorStaleTracksUseCase.hpp"
#include "domain/entities/Track.hpp"
#include "tests/mocks/MockTrackPresenter.hpp"
#include "tests/mocks/MockTrackRepository.hpp"

using namespace cwp;
using namespace testing;
using namespace std::chrono_literals;

namespace {

std::shared_ptr<domain::Track>
makeTrack(domain::TrackId id,
          std::chrono::system_clock::time_point timestamp)
{
    return std::make_shared<domain::Track>(
        id,
        domain::Position{51.5, 0.0, 35000},
        domain::Velocity{450.0, 270.0, 0},
        timestamp);
}

} // namespace

// ── No stale tracks ───────────────────────────────────────────────────────────

TEST(MonitorStaleTracksUseCaseTest, ReturnsZeroWhenNoTracksAreStale)
{
    auto mockRepo      = std::make_shared<tests::mocks::MockTrackRepository>();
    auto mockPresenter = std::make_shared<tests::mocks::MockTrackPresenter>();

    const auto now   = std::chrono::system_clock::now();
    const auto fresh = makeTrack(domain::TrackId{1U}, now - 10s);

    EXPECT_CALL(*mockRepo, findAll())
        .WillOnce(Return(std::vector<std::shared_ptr<domain::Track>>{fresh}));

    EXPECT_CALL(*mockRepo, remove(_)).Times(0);
    EXPECT_CALL(*mockPresenter, presentTrackRemoved(_)).Times(0);

    application::MonitorStaleTracksUseCase useCase{mockRepo, mockPresenter, 60s};
    EXPECT_EQ(useCase.execute(now), 0U);
}

// ── One stale track ───────────────────────────────────────────────────────────

TEST(MonitorStaleTracksUseCaseTest, RemovesAndNotifiesOneStaleTrack)
{
    auto mockRepo      = std::make_shared<tests::mocks::MockTrackRepository>();
    auto mockPresenter = std::make_shared<tests::mocks::MockTrackPresenter>();

    const auto now   = std::chrono::system_clock::now();
    const auto stale = makeTrack(domain::TrackId{42U}, now - 120s);

    EXPECT_CALL(*mockRepo, findAll())
        .WillOnce(Return(std::vector<std::shared_ptr<domain::Track>>{stale}));

    EXPECT_CALL(*mockRepo, remove(domain::TrackId{42U})).Times(1);
    EXPECT_CALL(*mockPresenter, presentTrackRemoved(domain::TrackId{42U})).Times(1);

    application::MonitorStaleTracksUseCase useCase{mockRepo, mockPresenter, 60s};
    EXPECT_EQ(useCase.execute(now), 1U);
}

// ── Mixed fresh and stale ─────────────────────────────────────────────────────

TEST(MonitorStaleTracksUseCaseTest, OnlyRemovesStaleTracks)
{
    auto mockRepo      = std::make_shared<tests::mocks::MockTrackRepository>();
    auto mockPresenter = std::make_shared<tests::mocks::MockTrackPresenter>();

    const auto now   = std::chrono::system_clock::now();
    const auto fresh = makeTrack(domain::TrackId{1U}, now - 5s);
    const auto stale = makeTrack(domain::TrackId{2U}, now - 90s);

    EXPECT_CALL(*mockRepo, findAll())
        .WillOnce(Return(
            std::vector<std::shared_ptr<domain::Track>>{fresh, stale}));

    EXPECT_CALL(*mockRepo, remove(domain::TrackId{2U})).Times(1);
    EXPECT_CALL(*mockPresenter, presentTrackRemoved(domain::TrackId{2U})).Times(1);

    application::MonitorStaleTracksUseCase useCase{mockRepo, mockPresenter, 60s};
    EXPECT_EQ(useCase.execute(now), 1U);
}

// ── Exactly at threshold ──────────────────────────────────────────────────────

TEST(MonitorStaleTracksUseCaseTest, TrackExactlyAtThresholdIsEvicted)
{
    auto mockRepo      = std::make_shared<tests::mocks::MockTrackRepository>();
    auto mockPresenter = std::make_shared<tests::mocks::MockTrackPresenter>();

    const auto now   = std::chrono::system_clock::now();
    const auto track = makeTrack(domain::TrackId{7U}, now - 60s);

    EXPECT_CALL(*mockRepo, findAll())
        .WillOnce(Return(std::vector<std::shared_ptr<domain::Track>>{track}));
    EXPECT_CALL(*mockRepo, remove(domain::TrackId{7U})).Times(1);
    EXPECT_CALL(*mockPresenter, presentTrackRemoved(domain::TrackId{7U})).Times(1);

    application::MonitorStaleTracksUseCase useCase{mockRepo, mockPresenter, 60s};
    EXPECT_EQ(useCase.execute(now), 1U);
}

// ── Null presenter safe ───────────────────────────────────────────────────────

TEST(MonitorStaleTracksUseCaseTest, DoesNotCrashWithNullPresenter)
{
    auto mockRepo = std::make_shared<tests::mocks::MockTrackRepository>();

    const auto now   = std::chrono::system_clock::now();
    const auto stale = makeTrack(domain::TrackId{99U}, now - 200s);

    EXPECT_CALL(*mockRepo, findAll())
        .WillOnce(Return(std::vector<std::shared_ptr<domain::Track>>{stale}));
    EXPECT_CALL(*mockRepo, remove(_)).Times(1);

    application::MonitorStaleTracksUseCase useCase{mockRepo, nullptr, 60s};
    EXPECT_NO_THROW(useCase.execute(now));
}
