#include <gtest/gtest.h>

#include <chrono>
#include <string>

#include "domain/entities/Track.hpp"
#include "domain/value_objects/Position.hpp"
#include "domain/value_objects/Velocity.hpp"

using namespace cwp::domain;
using namespace std::chrono_literals;

namespace {

/// Helper: constructs a valid Track for reuse across tests.
Track makeDefaultTrack()
{
    return Track{
        TrackId{1U},
        Position{51.5, 0.0, 35000},
        Velocity{450.0, 270.0, 0},
        std::chrono::system_clock::now()
    };
}

} // namespace

// ── Construction ─────────────────────────────────────────────────────────────

TEST(TrackTest, ConstructionStoresAllFields)
{
    const auto now = std::chrono::system_clock::now();
    const Position pos{48.85, 2.35, 30000};
    const Velocity vel{480.0, 90.0, 200};

    const Track track{TrackId{42U}, pos, vel, now};

    EXPECT_EQ(track.id(), TrackId{42U});
    EXPECT_DOUBLE_EQ(track.position().latitude(),  48.85);
    EXPECT_DOUBLE_EQ(track.position().longitude(), 2.35);
    EXPECT_EQ(track.position().altitude(),         30000);
    EXPECT_DOUBLE_EQ(track.velocity().speed(),     480.0);
    EXPECT_DOUBLE_EQ(track.velocity().heading(),   90.0);
    EXPECT_EQ(track.velocity().verticalRate(),     200);
    EXPECT_EQ(track.timestamp(),                   now);
}

// ── Callsign ─────────────────────────────────────────────────────────────────

TEST(TrackTest, CallsignIsAbsentAfterConstruction)
{
    const Track track = makeDefaultTrack();
    EXPECT_FALSE(track.callsign().has_value());
}

TEST(TrackTest, AssociateCallsignSetsCallsign)
{
    Track track = makeDefaultTrack();
    track.associateCallsign("DLH123");

    ASSERT_TRUE(track.callsign().has_value());
    EXPECT_EQ(*track.callsign(), "DLH123");
}

TEST(TrackTest, AssociateCallsignOverwritesPreviousCallsign)
{
    Track track = makeDefaultTrack();
    track.associateCallsign("DLH123");
    track.associateCallsign("BAW456");

    ASSERT_TRUE(track.callsign().has_value());
    EXPECT_EQ(*track.callsign(), "BAW456");
}

// ── Update ───────────────────────────────────────────────────────────────────

TEST(TrackTest, UpdateChangesPositionVelocityAndTimestamp)
{
    Track track = makeDefaultTrack();

    const auto      newTime = std::chrono::system_clock::now();
    const Position  newPos{52.0, 1.0, 36000};
    const Velocity  newVel{460.0, 280.0, 100};

    track.update(newPos, newVel, newTime);

    EXPECT_DOUBLE_EQ(track.position().latitude(),  52.0);
    EXPECT_DOUBLE_EQ(track.position().longitude(), 1.0);
    EXPECT_EQ(track.position().altitude(),         36000);
    EXPECT_DOUBLE_EQ(track.velocity().speed(),     460.0);
    EXPECT_DOUBLE_EQ(track.velocity().heading(),   280.0);
    EXPECT_EQ(track.velocity().verticalRate(),     100);
    EXPECT_EQ(track.timestamp(),                   newTime);
}

TEST(TrackTest, UpdatePreservesCallsign)
{
    Track track = makeDefaultTrack();
    track.associateCallsign("DLH123");

    track.update(Position{52.0, 1.0, 36000},
                 Velocity{460.0, 280.0, 100},
                 std::chrono::system_clock::now());

    ASSERT_TRUE(track.callsign().has_value());
    EXPECT_EQ(*track.callsign(), "DLH123");
}

// ── Stale track detection ────────────────────────────────────────────────────

TEST(TrackTest, FreshTrackIsNotStale)
{
    const auto now   = std::chrono::system_clock::now();
    const Track track{TrackId{2U}, Position{0.0, 0.0, 0},
                      Velocity{0.0, 0.0, 0}, now};

    EXPECT_FALSE(track.isStale(60s, now + 30s));
}

TEST(TrackTest, TrackBeyondThresholdIsStale)
{
    const auto now   = std::chrono::system_clock::now();
    const Track track{TrackId{3U}, Position{0.0, 0.0, 0},
                      Velocity{0.0, 0.0, 0}, now};

    EXPECT_TRUE(track.isStale(60s, now + 61s));
}

TEST(TrackTest, TrackExactlyAtThresholdIsStale)
{
    const auto now   = std::chrono::system_clock::now();
    const Track track{TrackId{4U}, Position{0.0, 0.0, 0},
                      Velocity{0.0, 0.0, 0}, now};

    // age == threshold → stale (>= comparison)
    EXPECT_TRUE(track.isStale(60s, now + 60s));
}

TEST(TrackTest, AgeReturnsElapsedSeconds)
{
    const auto baseline = std::chrono::system_clock::now();
    const Track track{TrackId{5U}, Position{0.0, 0.0, 0},
                      Velocity{0.0, 0.0, 0}, baseline};

    EXPECT_EQ(track.age(baseline + 45s), 45s);
}

// ── Strong TrackId ────────────────────────────────────────────────────────────

TEST(TrackIdTest, EqualityHoldsForSameValue)
{
    EXPECT_EQ(TrackId{10U}, TrackId{10U});
}

TEST(TrackIdTest, InequalityForDifferentValues)
{
    EXPECT_NE(TrackId{10U}, TrackId{11U});
}

TEST(TrackIdTest, OrderingWorksWithSpaceship)
{
    EXPECT_LT(TrackId{1U}, TrackId{2U});
    EXPECT_GT(TrackId{5U}, TrackId{3U});
}

// ── Value objects ─────────────────────────────────────────────────────────────

TEST(PositionTest, CreateSucceedsForValidCoordinates)
{
    const auto result = Position::create(51.5, 0.0, 35000);
    ASSERT_TRUE(result.has_value());
    EXPECT_DOUBLE_EQ(result->latitude(),  51.5);
    EXPECT_DOUBLE_EQ(result->longitude(), 0.0);
    EXPECT_EQ(result->altitude(),         35000);
}

TEST(PositionTest, CreateFailsForLatitudeAbove90)
{
    const auto result = Position::create(91.0, 0.0, 0);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), PositionError::LatitudeOutOfRange);
}

TEST(PositionTest, CreateFailsForLongitudeBeyond180)
{
    const auto result = Position::create(0.0, 181.0, 0);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), PositionError::LongitudeOutOfRange);
}

TEST(PositionTest, EqualityHoldsForIdenticalValues)
{
    const Position a{51.5, 0.0, 35000};
    const Position b{51.5, 0.0, 35000};
    EXPECT_EQ(a, b);
}

TEST(PositionTest, InequalityForDifferentAltitude)
{
    const Position a{51.5, 0.0, 35000};
    const Position b{51.5, 0.0, 36000};
    EXPECT_NE(a, b);
}

TEST(VelocityTest, CreateSucceedsForValidValues)
{
    const auto result = Velocity::create(450.0, 270.0, 0);
    ASSERT_TRUE(result.has_value());
    EXPECT_DOUBLE_EQ(result->speed(),   450.0);
    EXPECT_DOUBLE_EQ(result->heading(), 270.0);
}

TEST(VelocityTest, CreateFailsForNegativeSpeed)
{
    const auto result = Velocity::create(-1.0, 0.0, 0);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), VelocityError::NegativeSpeed);
}

TEST(VelocityTest, CreateFailsForHeadingAt360)
{
    const auto result = Velocity::create(100.0, 360.0, 0);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), VelocityError::HeadingOutOfRange);
}

TEST(VelocityTest, EqualityHoldsForIdenticalValues)
{
    const Velocity a{450.0, 270.0, 0};
    const Velocity b{450.0, 270.0, 0};
    EXPECT_EQ(a, b);
}

