/*
 * Pure-math oracle tests for the Stage 6 motion bedrock (Easing + CurveFit + HyperSpline).
 * No Views / no App display — these exercise the interpolation math directly, mirroring the
 * LinearSystem oracle tests. Gated on ENABLE_CONSTRAINTLAYOUT.
 */
#include <gui_features.h>
#ifdef ENABLE_CONSTRAINTLAYOUT

#include <gtest/gtest.h>

#include <memory>
#include <vector>

#include <widgetEx/constraintlayout/core/motion/curvefit.h>
#include <widgetEx/constraintlayout/core/motion/easing.h>
#include <widgetEx/constraintlayout/core/motion/hyperspline.h>
#include <widgetEx/constraintlayout/core/motion/linearcurvefit.h>
#include <widgetEx/constraintlayout/core/motion/monotoniccurvefit.h>

using namespace cdroid;

// ---- CurveFit ----

// Linear interpolation between (0)->0 and (1)->10: midpoint 0.5 -> 5, slope 10.
TEST(MotionMath, LinearCurveFitInterpolates) {
    std::vector<double> time = {0.0, 1.0};
    std::vector<std::vector<double>> y = {{0.0}, {10.0}};
    LinearCurveFit cf(time, y);
    EXPECT_NEAR(cf.getPos(0.5, 0), 5.0, 1e-9);
    EXPECT_NEAR(cf.getSlope(0.5, 0), 10.0, 1e-9);
    EXPECT_NEAR(cf.getPos(0.0, 0), 0.0, 1e-9);
    EXPECT_NEAR(cf.getPos(1.0, 0), 10.0, 1e-9);
}

// The monotone Hermite spline must pass exactly through every sample point.
TEST(MotionMath, MonotonicPassesThroughPoints) {
    std::vector<double> time = {0.0, 0.5, 1.0};
    std::vector<std::vector<double>> y = {{0.0}, {50.0}, {100.0}};
    MonotonicCurveFit cf(time, y);
    EXPECT_NEAR(cf.getPos(0.0, 0), 0.0, 1e-9);
    EXPECT_NEAR(cf.getPos(0.5, 0), 50.0, 1e-9);
    EXPECT_NEAR(cf.getPos(1.0, 0), 100.0, 1e-9);
    // monotonic in [0,1]
    EXPECT_LE(cf.getPos(0.25, 0), cf.getPos(0.75, 0));
}

// Factory: a single sample point collapses to CONSTANT.
TEST(MotionMath, CurveFitConstantFactory) {
    std::vector<double> time = {7.0};
    std::vector<std::vector<double>> y = {{42.0, 99.0}};
    auto cf = CurveFit::get(CurveFit::SPLINE, time, y); // single point -> CONSTANT
    ASSERT_NE(cf, nullptr);
    std::vector<double> v(2);
    cf->getPos(0.123, v);
    EXPECT_NEAR(v[0], 42.0, 1e-9);
    EXPECT_NEAR(v[1], 99.0, 1e-9);
}

// ---- Easing ----

// The base Easing is the identity: get(x) = x, getDiff = 1.
TEST(MotionMath, EasingIdentity) {
    Easing e;
    EXPECT_NEAR(e.get(0.3), 0.3, 1e-9);
    EXPECT_NEAR(e.getDiff(0.7), 1.0, 1e-9);
}

// A cubic-bezier easing maps endpoints 0->0, 1->1, stays in range, and is NOT the identity
// (the ease-in region at x=0.25 sits below the diagonal). The last check catches a silent
// fallback to the identity Easing.
TEST(MotionMath, EasingCubicEndpoints) {
    auto e = Easing::getInterpolator("cubic(0.4, 0.0, 0.2, 1)");
    ASSERT_NE(e, nullptr);
    EXPECT_NEAR(e->get(0.0), 0.0, 1e-3);
    EXPECT_NEAR(e->get(1.0), 1.0, 1e-3);
    double mid = e->get(0.5);
    EXPECT_GE(mid, 0.0);
    EXPECT_LE(mid, 1.0);
    double q = e->get(0.25);
    EXPECT_LT(q, 0.25 - 0.01); // ease-in: y(0.25) < 0.25, distinct from identity
}

// The named "linear" easing (cubic(1,1,0,0)) is ~identity at the endpoints.
TEST(MotionMath, EasingNamedLinear) {
    auto e = Easing::getInterpolator("linear");
    ASSERT_NE(e, nullptr);
    EXPECT_NEAR(e->get(0.0), 0.0, 1e-3);
    EXPECT_NEAR(e->get(1.0), 1.0, 1e-3);
    EXPECT_NEAR(e->get(0.5), 0.5, 1e-2); // linear bezier -> y == x
}

// Empty config string -> nullptr (caller applies its own default).
TEST(MotionMath, EasingFactoryNullOnEmpty) {
    EXPECT_EQ(Easing::getInterpolator(""), nullptr);
}

// Schlick easing parses and maps 0 -> 0.
TEST(MotionMath, SchlickParses) {
    auto e = Easing::getInterpolator("Schlick(0.5, 0.5)");
    ASSERT_NE(e, nullptr);
    EXPECT_NEAR(e->get(0.0), 0.0, 1e-9);
}

// "spline(...)" parses into a StepCurve backed by a monotonic spline (0->0, 1->1).
TEST(MotionMath, StepCurveParses) {
    auto e = Easing::getInterpolator("spline(0.0, 0.3, 0.5, 0.7, 1.0)");
    ASSERT_NE(e, nullptr);
    EXPECT_NEAR(e->get(0.0), 0.0, 1e-3);
    EXPECT_NEAR(e->get(1.0), 1.0, 1e-3);
}

// ---- HyperSpline ----

// The N-d natural-cubic spline passes through its first and last sample points.
TEST(MotionMath, HyperSplineEndpoints) {
    std::vector<std::vector<double>> points = {{0.0, 0.0}, {50.0, 100.0}, {100.0, 0.0}};
    HyperSpline hs(points);
    EXPECT_NEAR(hs.getPos(0.0, 0), 0.0, 1e-6);
    EXPECT_NEAR(hs.getPos(1.0, 0), 100.0, 1e-6);
}

#endif // ENABLE_CONSTRAINTLAYOUT
