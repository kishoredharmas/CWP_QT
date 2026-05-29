#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <chrono>
#include <memory>
#include <span>
#include <vector>

#include "application/use_cases/DisplayTrackUseCase.hpp"
#include "domain/entities/Track.hpp"
#include "tests/mocks/MockTrackPresenter.hpp"
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

} // namespace

// ── execute() — normal path ───────────────────────────────────────────────────

TEST(DisplayTrackUseCaseTest, ExecuteForwardsTwoTracksToPresenter)
{
    auto mockRepo      = std::make_shared<tests::mocks::MockTrackRepository>();
    auto mockPresenter = std::make_shared<tests::mocks::MockTrackPresenter>();

    const auto track1 = makeTrack(domain::TrackId{1U});
    const auto track2 = makeTrack(domain::TrackId{2U});

    EXPECT_CALL(*mockRepo, findAll())
        .WillOnce(Return(
            std::vector<std::shared_ptr<domain::Track>>{track1, track2}));

    // presentTracks now takes a std::span — SizeIs still works on ranges.
    EXPECT_CALL(*mockPresenter, presentTracks(SizeIs(2)));

    application::DisplayTrackUseCase useCase{mockRepo, mockPresenter};
    useCase.execute();
}

TEST(DisplayTrackUseCaseTest, ExecuteWithEmptyRepositoryCallsPresentTracksWithEmptySpan)
{
    auto mockRepo      = std::make_shared<tests::mocks::MockTrackRepository>();
    auto mockPresenter = std::make_shared<tests::mocks::MockTrackPresenter>();

    EXPECT_CALL(*mockRepo, findAll())
        .WillOnce(Return(std::vector<std::shared_ptr<domain::Track>>{}));

    EXPECT_CALL(*mockPresenter, presentTracks(IsEmpty()));

    application::DisplayTrackUseCase useCase{mockRepo, mockPresenter};
    useCase.execute();
}

// ── Null presenter guard ──────────────────────────────────────────────────────

TEST(DisplayTrackUseCaseTest, ExecuteIsNoOpWhenPresenterIsNull)
{
    auto mockRepo = std::make_shared<tests::mocks::MockTrackRepository>();

    EXPECT_CALL(*mockRepo, findAll())
        .WillRepeatedly(Return(std::vector<std::shared_ptr<domain::Track>>{}));

    application::DisplayTrackUseCase useCase{mockRepo, nullptr};
    EXPECT_NO_THROW(useCase.execute());
}

// ── setPresenter() ────────────────────────────────────────────────────────────

TEST(DisplayTrackUseCaseTest, SetPresenterLateWiresPresenterCorrectly)
{
    auto mockRepo      = std::make_shared<tests::mocks::MockTrackRepository>();
    auto mockPresenter = std::make_shared<tests::mocks::MockTrackPresenter>();

    EXPECT_CALL(*mockRepo, findAll())
        .WillOnce(Return(std::vector<std::shared_ptr<domain::Track>>{
            makeTrack(domain::TrackId{5U})}));

    EXPECT_CALL(*mockPresenter, presentTracks(SizeIs(1)));

    application::DisplayTrackUseCase useCase{mockRepo, nullptr};
    useCase.setPresenter(mockPresenter);
    useCase.execute();
}
