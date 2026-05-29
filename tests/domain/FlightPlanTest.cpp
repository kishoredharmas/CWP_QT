#include <gtest/gtest.h>

#include <vector>
#include <string>

#include "domain/entities/FlightPlan.hpp"

using namespace cwp::domain;

namespace {

FlightPlan makeDefaultFlightPlan()
{
    return FlightPlan{
        "DLH123",
        "A320",
        "1234",
        350,
        {"EDDF", "TULSI", "EGLL"}
    };
}

} // namespace

// ── Construction ─────────────────────────────────────────────────────────────

TEST(FlightPlanTest, ConstructionStoresAllFields)
{
    const FlightPlan fp = makeDefaultFlightPlan();

    EXPECT_EQ(fp.callsign(),      "DLH123");
    EXPECT_EQ(fp.aircraftType(),  "A320");
    EXPECT_EQ(fp.squawk(),        "1234");
    EXPECT_EQ(fp.assignedLevel(), 350);
    ASSERT_EQ(fp.route().size(),  3U);
    EXPECT_EQ(fp.route().at(0),   "EDDF");
    EXPECT_EQ(fp.route().at(1),   "TULSI");
    EXPECT_EQ(fp.route().at(2),   "EGLL");
}

TEST(FlightPlanTest, EmptyRouteIsValid)
{
    const FlightPlan fp{"SWR001", "B773", "4567", 370, {}};
    EXPECT_TRUE(fp.route().empty());
}

// ── Level amendment ──────────────────────────────────────────────────────────

TEST(FlightPlanTest, AmendLevelUpdatesAssignedLevel)
{
    FlightPlan fp = makeDefaultFlightPlan();
    fp.amendLevel(390);
    EXPECT_EQ(fp.assignedLevel(), 390);
}

TEST(FlightPlanTest, AmendLevelToZeroIsAccepted)
{
    FlightPlan fp = makeDefaultFlightPlan();
    fp.amendLevel(0);
    EXPECT_EQ(fp.assignedLevel(), 0);
}

TEST(FlightPlanTest, AmendLevelDoesNotAffectOtherFields)
{
    FlightPlan fp = makeDefaultFlightPlan();
    fp.amendLevel(410);

    EXPECT_EQ(fp.callsign(),     "DLH123");
    EXPECT_EQ(fp.aircraftType(), "A320");
    EXPECT_EQ(fp.squawk(),       "1234");
}
