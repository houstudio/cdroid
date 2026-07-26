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
#include <widgetEx/constraintlayout/core/motion/motion.h>
#include <widgetEx/constraintlayout/core/motion/motionkeyattributes.h>
#include <widgetEx/constraintlayout/core/motion/motionkeyposition.h>
#include <widgetEx/constraintlayout/core/motion/motionkeycycle.h>
#include <widgetEx/constraintlayout/core/motion/motionwidget.h>
#include <widgetEx/constraintlayout/core/motion/oscillator.h>
#include <widgetEx/constraintlayout/core/motion/splineset.h>

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

// ---- Oscillator ----

// A sine oscillator at zero phase starts at sin(0) = 0; cosine starts at cos(0) = 1.
TEST(MotionMath, OscillatorStartValue) {
    Oscillator osc;
    osc.setType(Oscillator::SIN_WAVE, "");
    osc.addPoint(0.0, 1.0f);
    osc.addPoint(1.0, 1.0f);
    osc.normalize();
    EXPECT_NEAR(osc.getValue(0.0, 0.0), 0.0, 1e-6);

    Oscillator cosc;
    cosc.setType(Oscillator::COS_WAVE, "");
    cosc.addPoint(0.0, 1.0f);
    cosc.addPoint(1.0, 1.0f);
    cosc.normalize();
    EXPECT_NEAR(cosc.getValue(0.0, 0.0), 1.0, 1e-6);
}

// All wave types stay bounded in [-1, 1] across the progress axis.
TEST(MotionMath, OscillatorBounded) {
    Oscillator osc;
    osc.addPoint(0.0, 1.0f);
    osc.addPoint(0.5f, 2.0f);
    osc.addPoint(1.0, 1.0f);
    osc.normalize();
    const int types[] = {Oscillator::SIN_WAVE, Oscillator::SQUARE_WAVE, Oscillator::TRIANGLE_WAVE,
                         Oscillator::SAW_WAVE, Oscillator::REVERSE_SAW_WAVE, Oscillator::COS_WAVE,
                         Oscillator::BOUNCE};
    for (int type : types) {
        osc.setType(type, "");
        for (int i = 0; i <= 20; i++) {
            double v = osc.getValue(i / 20.0, 0.0);
            EXPECT_GE(v, -1.0 - 1e-6) << "type=" << type << " i=" << i;
            EXPECT_LE(v, 1.0 + 1e-6) << "type=" << type << " i=" << i;
        }
    }
}

// ---- Motion engine (linear MVP) ----

// A Motion interpolates a child linearly between a start and end frame at progress 0.5.
// start (0,0,100,50) -> end (500,300,200,100): midpoint (250,150,150,75).
TEST(MotionMath, MotionInterpolatesLinearly) {
    MotionWidget start; start.setBounds(0, 0, 100, 50);
    MotionWidget end;   end.setBounds(500, 300, 700, 400); // w=200, h=100

    Motion m;
    m.setStart(&start);
    m.setEnd(&end);

    MotionWidget child;
    m.interpolate(&child, 0.5f);
    EXPECT_EQ(child.getLeft(), 250);
    EXPECT_EQ(child.getTop(), 150);
    EXPECT_EQ(child.getWidth(), 150); // (100+200)/2
    EXPECT_EQ(child.getHeight(), 75); // (50+100)/2
}

// At progress 0 the child matches the start; at 1 it matches the end.
TEST(MotionMath, MotionEndpoints) {
    MotionWidget start; start.setBounds(10, 20, 110, 70);   // w=100, h=50
    MotionWidget end;   end.setBounds(200, 400, 400, 450);  // w=200, h=50
    Motion m;
    m.setStart(&start);
    m.setEnd(&end);

    MotionWidget c0; m.interpolate(&c0, 0.0f);
    EXPECT_EQ(c0.getLeft(), 10);
    EXPECT_EQ(c0.getTop(), 20);
    EXPECT_EQ(c0.getWidth(), 100);

    MotionWidget c1; m.interpolate(&c1, 1.0f);
    EXPECT_EQ(c1.getLeft(), 200);
    EXPECT_EQ(c1.getTop(), 400);
    EXPECT_EQ(c1.getWidth(), 200);
}

// An "accelerate" easing curve makes the midpoint position lag the linear midpoint.
// start(0,0,100,50) -> end(1000,0,1100,50): linear midpoint = 500; accelerated < 500.
TEST(MotionMath, MotionEasingAccelerate) {
    MotionWidget start; start.setBounds(0, 0, 100, 50);
    MotionWidget end;   end.setBounds(1000, 0, 1100, 50);
    Motion m;
    m.setStart(&start);
    m.setEnd(&end);
    m.setValue(TypedValues::MotionType::TYPE_EASING, std::string("accelerate"));

    MotionWidget mid; m.interpolate(&mid, 0.5f);
    EXPECT_GT(mid.getLeft(), 0);
    EXPECT_LT(mid.getLeft(), 500); // ease-in: behind the linear midpoint
    EXPECT_EQ(mid.getTop(), 0);    // y unchanged
}

// A KeyAttributes keyframe sets alpha=0 at frame 50; the alpha dips to 0 at progress 0.5
// and interpolates linearly back to the endpoints (alpha=1) at 0.25/0.75.
TEST(MotionMath, MotionKeyframeAlpha) {
    MotionWidget start; start.setBounds(0, 0, 100, 50); start.setAlpha(1.0f);
    MotionWidget end;   end.setBounds(100, 0, 200, 50); end.setAlpha(1.0f);
    Motion m;
    m.setStart(&start);
    m.setEnd(&end);

    MotionKeyAttributes kf;
    kf.setFramePosition(50);
    kf.setValue(TypedValues::AttributesType::TYPE_ALPHA, 0.0f);
    m.addKey(&kf);

    MotionWidget atMid; m.interpolate(&atMid, 0.5f);
    EXPECT_NEAR(atMid.getAlpha(), 0.0f, 1e-3); // keyframe: alpha 0 at midpoint

    MotionWidget atQuarter; m.interpolate(&atQuarter, 0.25f);
    EXPECT_NEAR(atQuarter.getAlpha(), 0.5f, 1e-3); // halfway between start(1) and keyframe(0)
}

// A KeyPosition at frame 50 with a perpendicular offset makes the widget deviate from the
// straight-line path. start(0,0,100,50) -> end(500,0,600,50) (horizontal). At progress 0.5
// without keyframe the midpoint is (250,0); with altPercentY=0.5 it arcs down to (250,250).
TEST(MotionMath, MotionKeyPositionOffset) {
    MotionWidget start; start.setBounds(0, 0, 100, 50);
    MotionWidget end;   end.setBounds(500, 0, 600, 50);
    Motion m;
    m.setStart(&start);
    m.setEnd(&end);

    MotionKeyPosition kp;
    kp.mFramePosition = 50;
    kp.mPercentX = 0.5f;      // halfway along the path
    kp.mAltPercentY = 0.5f;   // perpendicular offset (downward)
    m.addKey(&kp);

    MotionWidget mid; m.interpolate(&mid, 0.5f);
    EXPECT_EQ(mid.getLeft(), 250); // keyframe x
    EXPECT_EQ(mid.getTop(), 250);  // arced below the linear path (y=0)
}

// A KeyCycle superimposes a sine-wave oscillation on the base alpha.
// base alpha=1, cycle amplitude=0.3, period=2 (2 full sine cycles over [0,1]).
// At progress 0.125: overlay = sin(2π·0.125·2)·0.3 = sin(π/2)·0.3 = 0.3 → alpha=1.3.
TEST(MotionMath, MotionKeyCycleAlpha) {
    MotionWidget start; start.setBounds(0, 0, 100, 50); start.setAlpha(1.0f);
    MotionWidget end;   end.setBounds(100, 0, 200, 50); end.setAlpha(1.0f);
    Motion m;
    m.setStart(&start);
    m.setEnd(&end);

    MotionKeyCycle cyc;
    cyc.mFramePosition = 50;
    cyc.mAlpha = 0.3f;       // oscillation amplitude
    cyc.mWavePeriod = 2.0f;  // 2 full cycles over the transition
    cyc.mWaveShape = Oscillator::SIN_WAVE;
    m.addKey(&cyc);

    MotionWidget atEighth; m.interpolate(&atEighth, 0.125f);
    EXPECT_NEAR(atEighth.getAlpha(), 1.3f, 0.01); // base(1) + sin(π/2)·0.3

    MotionWidget atStart; m.interpolate(&atStart, 0.0f);
    EXPECT_NEAR(atStart.getAlpha(), 1.0f, 0.01);  // base(1) + sin(0)·0.3
}

// ---- SplineSet (spline-based keyframe interpolation) ----

// SplineSet builds a CurveFit from (framePosition, value) pairs and interpolates smoothly.
// Points: (0, 0) → (50, 100) → (100, 0). At t=0.5 the spline passes through the keyframe (100).
TEST(MotionMath, SplineSetInterpolates) {
    SplineSet ss;
    ss.setPoint(0, 0.0f);
    ss.setPoint(50, 100.0f);
    ss.setPoint(100, 0.0f);
    ss.setup(CurveFit::SPLINE);
    EXPECT_NEAR(ss.get(0.0f), 0.0f, 0.5);
    EXPECT_NEAR(ss.get(0.5f), 100.0f, 0.5); // passes through keyframe
    EXPECT_NEAR(ss.get(1.0f), 0.0f, 0.5);
    float mid = ss.get(0.25f);
    EXPECT_GT(mid, 0.0f);
    EXPECT_LT(mid, 100.0f); // between endpoints
}

#endif // ENABLE_CONSTRAINTLAYOUT
