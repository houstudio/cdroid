#include <gtest/gtest.h>
#include <core/path.h>
#include <core/pathmeasure.h>
#include <cmath>

using namespace cdroid;

// getSegment must emit only the part of the path inside the requested
// distance window. Regression: segments entirely before the window used to be
// extrapolated (t > 1) and segments after it were emitted with negative t
// before the loop broke, garbaging the trimmed arc of an AVD spinner.
TEST(PathMeasure, GetSegmentMidWindowOnMultiSegmentLine) {
    Path path;
    path.moveTo(0, 0);
    path.lineTo(10, 0);
    path.lineTo(20, 0);
    path.lineTo(30, 0);

    PathMeasure measure(std::make_shared<Path>(path), false);
    ASSERT_NEAR(measure.getLength(), 30.0, 1e-6);

    auto dst = std::make_shared<Path>();
    ASSERT_TRUE(measure.getSegment(12.0, 17.0, dst, true));

    PathMeasure outMeasure(dst, false);
    EXPECT_NEAR(outMeasure.getLength(), 5.0, 1e-3);
    double pos[2] = {0, 0};
    ASSERT_TRUE(outMeasure.getPosTan(0.0, pos, nullptr));
    EXPECT_NEAR(pos[0], 12.0, 1e-3);
    EXPECT_NEAR(pos[1], 0.0, 1e-3);
    ASSERT_TRUE(outMeasure.getPosTan(5.0, pos, nullptr));
    EXPECT_NEAR(pos[0], 17.0, 1e-3);
    EXPECT_NEAR(pos[1], 0.0, 1e-3);
}

// A window covering exactly one whole segment keeps its endpoints intact.
TEST(PathMeasure, GetSegmentExactSegmentBounds) {
    Path path;
    path.moveTo(0, 0);
    path.lineTo(10, 0);
    path.lineTo(20, 0);

    PathMeasure measure(std::make_shared<Path>(path), false);
    auto dst = std::make_shared<Path>();
    ASSERT_TRUE(measure.getSegment(10.0, 20.0, dst, true));

    PathMeasure outMeasure(dst, false);
    EXPECT_NEAR(outMeasure.getLength(), 10.0, 1e-3);
}

// Degenerate windows produce an empty destination, not extrapolated junk.
TEST(PathMeasure, GetSegmentEmptyWindow) {
    Path path;
    path.moveTo(0, 0);
    path.lineTo(10, 0);
    path.lineTo(20, 0);

    PathMeasure measure(std::make_shared<Path>(path), false);
    auto dst = std::make_shared<Path>();
    // start == stop is rejected by the AOSP contract.
    EXPECT_FALSE(measure.getSegment(5.0, 5.0, dst, true));
}
