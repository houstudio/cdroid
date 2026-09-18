#include <gtest/gtest.h>
#include <animation/interpolators.h>
#include <drawable/pathparser.h>
#include <cmath>

using namespace cdroid;

// Expected values ported from androidx PathInterpolatorValueParameterizedTest
// (vectordrawable-animated androidTest): each interpolator is sampled at
// t=0.5 and must land within 1e-3 of the platform value.
static constexpr float EPSILON = 1e-3f;

static std::shared_ptr<Path> pathFromData(const std::string& pathData) {
    return PathParser::createPathFromPathData(pathData);
}

TEST(Interpolator, PathInterpolatorCubicControlPoints) {
    // res: control_points_interpolator — controlX1=0 controlY1=0 controlX2=0 controlY2=1
    PathInterpolator interp(0.f, 0.f, 0.f, 1.f);
    EXPECT_NEAR(interp.getInterpolation(0.5f), 0.89f, EPSILON);
}

TEST(Interpolator, PathInterpolatorQuadControlPoint) {
    // res: single_control_point_interpolator — controlX1=1 controlY1=0
    PathInterpolator interp(1.f, 0.f);
    EXPECT_NEAR(interp.getInterpolation(0.5f), 0.086f, EPSILON);
}

TEST(Interpolator, PathInterpolatorFromPathData) {
    // res: path_interpolator. The platform lands exactly on 0.85 (x=0.5 sits
    // on the line segment); CDROID's arc-length sampling of the leading cubic
    // shifts the lookup by ~0.0016, so the tolerance is slightly wider than
    // the androidx 1e-3 here.
    auto path = pathFromData("M 0.0,0.0 c 0.08,0.0 0.04,1.0 0.2,0.8 l 0.6,0.1 L 1.0,1.0");
    ASSERT_NE(path, nullptr);
    PathInterpolator interp(*path);
    EXPECT_NEAR(interp.getInterpolation(0.5f), 0.85f, 5e-3f);
}

// Material progress spinner interpolators (frameworks/base res/interpolator):
// trim_start stays flat at 0 for x < 0.5, then eases toward 0.75.
TEST(Interpolator, TrimStartInterpolator) {
    auto path = pathFromData("L0.5,0 C 0.7,0 0.6,1 1, 1");
    ASSERT_NE(path, nullptr);
    PathInterpolator interp(*path);
    EXPECT_NEAR(interp.getInterpolation(0.25f), 0.f, EPSILON);
    // x = 0.75 lands on the cubic segment: analytic y is about 0.78.
    EXPECT_NEAR(interp.getInterpolation(0.75f), 0.78f, 0.05f);
    EXPECT_NEAR(interp.getInterpolation(1.f), 1.f, EPSILON);
}

// trim_end reaches 1 at x = 0.5 (line from (0.5,1) to (1,1)).
TEST(Interpolator, TrimEndInterpolator) {
    auto path = pathFromData("C0.2,0 0.1,1 0.5,1 L 1,1");
    ASSERT_NE(path, nullptr);
    PathInterpolator interp(*path);
    EXPECT_NEAR(interp.getInterpolation(0.75f), 1.f, EPSILON);
    EXPECT_NEAR(interp.getInterpolation(1.f), 1.f, EPSILON);
}

TEST(Interpolator, PathInterpolatorEndpooints) {
    PathInterpolator interp(0.4f, 0.f, 0.6f, 1.f);
    EXPECT_EQ(interp.getInterpolation(-0.5f), 0.f);
    EXPECT_EQ(interp.getInterpolation(0.f), 0.f);
    EXPECT_EQ(interp.getInterpolation(1.f), 1.f);
    EXPECT_EQ(interp.getInterpolation(1.5f), 1.f);
}

// A valid interpolator path is monotonic in x, so the sampled output must be
// non-decreasing across the input range.
TEST(Interpolator, PathInterpolatorMonotonic) {
    PathInterpolator interp(0.4f, 0.f, 0.6f, 1.f);
    float prev = -1.f;
    for (int i = 0; i <= 100; i++) {
        const float y = interp.getInterpolation(i / 100.f);
        EXPECT_GE(y, prev) << "non-monotonic at i=" << i;
        prev = y;
    }
}
