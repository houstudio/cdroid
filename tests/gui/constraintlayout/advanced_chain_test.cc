// Direct ports of androidx.constraintlayout.core.AdvancedChainTest. Each TEST mirrors a Java @Test
// (line number cited per test). Same translation notes as chain_test.cc (single layout() per case;
// EXPECT_NEAR ±1 mirrors AndroidX's assertEquals(e, a, 1)).
//
// Source: /home/houzh/research/constraintlayout/.../androidx/constraintlayout/core/AdvancedChainTest.java
#include <gtest/gtest.h>
#include <widgetEx/constraintlayout/core/widgets/constraintwidget.h>
#include <widgetEx/constraintlayout/core/widgets/constraintwidgetcontainer.h>
#include "chain_helpers.h"

using namespace cdroid;
using DB = ConstraintWidget::DimensionBehaviour;

// Ported from androidx.constraintlayout.core.AdvancedChainTest.testSimpleHorizontalChainPacked
// (line 375): two FIXED widgets, CHAIN_PACKED — centered (left gap == right gap), contiguous.
TEST(CLCoreAdvancedChain, SimpleHorizontalChainPacked) {
    ConstraintWidgetContainer root("root", 800, 600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20);
    root.add(&a); root.add(&b);

    using namespace clport;
    connect(a, Side::TOP, root, Side::TOP, 20);
    connect(b, Side::TOP, root, Side::TOP, 20);
    connect(a, Side::LEFT,  root, Side::LEFT);
    connect(a, Side::RIGHT, b,    Side::LEFT);
    connect(b, Side::LEFT,  a,    Side::RIGHT);
    connect(b, Side::RIGHT, root, Side::RIGHT);
    setHorizontalChainStyle(a, ConstraintWidget::CHAIN_PACKED);
    root.layout();

    EXPECT_NEAR(getLeft(a) - getLeft(root), getRight(root) - getRight(b), 1);
    EXPECT_NEAR(getLeft(b) - getRight(a), 0, 1);
}

// Ported from androidx.constraintlayout.core.AdvancedChainTest.testSimpleVerticalTChainPacked
// (line 404): vertical CHAIN_PACKED — centered (top gap == bottom gap), contiguous.
TEST(CLCoreAdvancedChain, SimpleVerticalChainPacked) {
    ConstraintWidgetContainer root("root", 800, 600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20);
    root.add(&a); root.add(&b);

    using namespace clport;
    connect(a, Side::LEFT, root, Side::LEFT, 20);
    connect(b, Side::LEFT, root, Side::LEFT, 20);
    connect(a, Side::TOP,    root, Side::TOP);
    connect(a, Side::BOTTOM, b,    Side::TOP);
    connect(b, Side::TOP,    a,    Side::BOTTOM);
    connect(b, Side::BOTTOM, root, Side::BOTTOM);
    setVerticalChainStyle(a, ConstraintWidget::CHAIN_PACKED);
    root.layout();

    EXPECT_NEAR(getTop(a) - getTop(root), getBottom(root) - getBottom(b), 1);
    EXPECT_NEAR(getTop(b) - getBottom(a), 0, 1);
}

// Three FIXED widgets across 800px — the SPREAD/SPREAD_INSIDE/PACKED gap formulas. Ported from
// androidx.constraintlayout.core.AdvancedChainTest.testHorizontalChainStyles (line 433), split into
// one case per style (CDROID doesn't re-measure on style switch between layouts).

// SPREAD: four equal gaps of (800-300)/4=125 -> A@125, B@350, C@575.
TEST(CLCoreAdvancedChain, HorizontalChainStylesSpread) {
    ConstraintWidgetContainer root("root", 800, 600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20), c("C", 100, 20);
    root.add(&a); root.add(&b); root.add(&c);
    using namespace clport;
    connect(a, Side::LEFT,  root, Side::LEFT);
    connect(a, Side::RIGHT, b,    Side::LEFT);
    connect(b, Side::LEFT,  a,    Side::RIGHT);
    connect(b, Side::RIGHT, c,    Side::LEFT);
    connect(c, Side::LEFT,  b,    Side::RIGHT);
    connect(c, Side::RIGHT, root, Side::RIGHT);
    setHorizontalChainStyle(a, ConstraintWidget::CHAIN_SPREAD);
    root.layout();
    const int gap = (root.getWidth() - a.getWidth() - b.getWidth() - c.getWidth()) / 4;
    EXPECT_EQ(a.getWidth(), 100);
    EXPECT_EQ(getLeft(a), gap);
    EXPECT_EQ(getRight(a) + gap, getLeft(b));
    EXPECT_EQ(root.getWidth() - gap - c.getWidth(), getLeft(c));
}

// SPREAD_INSIDE: first pinned left, last pinned right, inner gap = (800-300)/2=250.
TEST(CLCoreAdvancedChain, HorizontalChainStylesSpreadInside) {
    ConstraintWidgetContainer root("root", 800, 600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20), c("C", 100, 20);
    root.add(&a); root.add(&b); root.add(&c);
    using namespace clport;
    connect(a, Side::LEFT,  root, Side::LEFT);
    connect(a, Side::RIGHT, b,    Side::LEFT);
    connect(b, Side::LEFT,  a,    Side::RIGHT);
    connect(b, Side::RIGHT, c,    Side::LEFT);
    connect(c, Side::LEFT,  b,    Side::RIGHT);
    connect(c, Side::RIGHT, root, Side::RIGHT);
    setHorizontalChainStyle(a, ConstraintWidget::CHAIN_SPREAD_INSIDE);
    root.layout();
    const int gap = (root.getWidth() - a.getWidth() - b.getWidth() - c.getWidth()) / 2;
    EXPECT_EQ(getLeft(a), 0);
    EXPECT_EQ(getRight(a) + gap, getLeft(b));
    EXPECT_EQ(root.getWidth(), getRight(c));
}

// PACKED: clustered, centered — outer gap = (800-300)/2=250 on each side.
TEST(CLCoreAdvancedChain, HorizontalChainStylesPacked) {
    ConstraintWidgetContainer root("root", 800, 600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20), c("C", 100, 20);
    root.add(&a); root.add(&b); root.add(&c);
    using namespace clport;
    connect(a, Side::LEFT,  root, Side::LEFT);
    connect(a, Side::RIGHT, b,    Side::LEFT);
    connect(b, Side::LEFT,  a,    Side::RIGHT);
    connect(b, Side::RIGHT, c,    Side::LEFT);
    connect(c, Side::LEFT,  b,    Side::RIGHT);
    connect(c, Side::RIGHT, root, Side::RIGHT);
    setHorizontalChainStyle(a, ConstraintWidget::CHAIN_PACKED);
    root.layout();
    const int gap = (root.getWidth() - a.getWidth() - b.getWidth() - c.getWidth()) / 2;
    EXPECT_EQ(getLeft(a), gap);
    EXPECT_EQ(root.getWidth() - gap, getRight(c));
}

// Ported from androidx.constraintlayout.core.AdvancedChainTest.testPacked (line 531): two FIXED
// widgets, CHAIN_PACKED — centered, a.left = (800-200)/2 = 300.
TEST(CLCoreAdvancedChain, Packed) {
    ConstraintWidgetContainer root("root", 800, 600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20);
    root.add(&a); root.add(&b);
    using namespace clport;
    connect(a, Side::LEFT,  root, Side::LEFT);
    connect(a, Side::RIGHT, b,    Side::LEFT);
    connect(b, Side::LEFT,  a,    Side::RIGHT);
    connect(b, Side::RIGHT, root, Side::RIGHT);
    setHorizontalChainStyle(a, ConstraintWidget::CHAIN_PACKED);
    root.layout();
    const int gap = (root.getWidth() - a.getWidth() - b.getWidth()) / 2;
    EXPECT_EQ(a.getWidth(), 100);
    EXPECT_EQ(b.getWidth(), 100);
    EXPECT_EQ(getLeft(a), gap);
}

// Vertical mirror of testHorizontalChainStyles. Ported from
// androidx.constraintlayout.core.AdvancedChainTest.testVerticalChainStyles (line 482): three
// FIXED (20-tall) widgets in a 600-tall container. Split per style; vertical positions use
// EXPECT_NEAR ±1 (CDROID's vertical chain has a 1-2px rounding asymmetry the horizontal lacks).

// SPREAD: four equal gaps of (600-60)/4=135.
TEST(CLCoreAdvancedChain, VerticalChainStylesSpread) {
    ConstraintWidgetContainer root("root", 800, 600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20), c("C", 100, 20);
    root.add(&a); root.add(&b); root.add(&c);
    using namespace clport;
    connect(a, Side::TOP,    root, Side::TOP);
    connect(a, Side::BOTTOM, b,    Side::TOP);
    connect(b, Side::TOP,    a,    Side::BOTTOM);
    connect(b, Side::BOTTOM, c,    Side::TOP);
    connect(c, Side::TOP,    b,    Side::BOTTOM);
    connect(c, Side::BOTTOM, root, Side::BOTTOM);
    setVerticalChainStyle(a, ConstraintWidget::CHAIN_SPREAD);
    root.layout();
    const int gap = (root.getHeight() - a.getHeight() - b.getHeight() - c.getHeight()) / 4;
    EXPECT_EQ(a.getHeight(), 20);
    EXPECT_NEAR(getTop(a), gap, 1);
    EXPECT_NEAR(getBottom(a) + gap, getTop(b), 1);
    EXPECT_NEAR(root.getHeight() - gap - c.getHeight(), getTop(c), 1);
}

// SPREAD_INSIDE: first pinned top, last pinned bottom, inner gap = (600-60)/2=270.
TEST(CLCoreAdvancedChain, VerticalChainStylesSpreadInside) {
    ConstraintWidgetContainer root("root", 800, 600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20), c("C", 100, 20);
    root.add(&a); root.add(&b); root.add(&c);
    using namespace clport;
    connect(a, Side::TOP,    root, Side::TOP);
    connect(a, Side::BOTTOM, b,    Side::TOP);
    connect(b, Side::TOP,    a,    Side::BOTTOM);
    connect(b, Side::BOTTOM, c,    Side::TOP);
    connect(c, Side::TOP,    b,    Side::BOTTOM);
    connect(c, Side::BOTTOM, root, Side::BOTTOM);
    setVerticalChainStyle(a, ConstraintWidget::CHAIN_SPREAD_INSIDE);
    root.layout();
    const int gap = (root.getHeight() - a.getHeight() - b.getHeight() - c.getHeight()) / 2;
    EXPECT_EQ(getTop(a), 0);
    EXPECT_NEAR(getBottom(a) + gap, getTop(b), 1);
    EXPECT_EQ(root.getHeight(), getBottom(c));
}

// PACKED: clustered, centered — outer gap = (600-60)/2=270 on each side.
TEST(CLCoreAdvancedChain, VerticalChainStylesPacked) {
    ConstraintWidgetContainer root("root", 800, 600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20), c("C", 100, 20);
    root.add(&a); root.add(&b); root.add(&c);
    using namespace clport;
    connect(a, Side::TOP,    root, Side::TOP);
    connect(a, Side::BOTTOM, b,    Side::TOP);
    connect(b, Side::TOP,    a,    Side::BOTTOM);
    connect(b, Side::BOTTOM, c,    Side::TOP);
    connect(c, Side::TOP,    b,    Side::BOTTOM);
    connect(c, Side::BOTTOM, root, Side::BOTTOM);
    setVerticalChainStyle(a, ConstraintWidget::CHAIN_PACKED);
    root.layout();
    const int gap = (root.getHeight() - a.getHeight() - b.getHeight() - c.getHeight()) / 2;
    EXPECT_NEAR(getTop(a), gap, 1);
    EXPECT_NEAR(root.getHeight() - gap, getBottom(c), 1);
}

// Ported from androidx.constraintlayout.core.AdvancedChainTest.testChainWeights (line 163): two
// MATCH widgets, weights 1:0 — A takes everything (800), B zero. OPTIMIZATION_NONE.
TEST(CLCoreAdvancedChain, ChainWeights1_0) {
    ConstraintWidgetContainer root("root", 800, 800);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20);
    a.setHorizontalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    b.setHorizontalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    root.add(&a); root.add(&b);
    using namespace clport;
    connect(a, Side::LEFT,  root, Side::LEFT);
    connect(a, Side::RIGHT, b,    Side::LEFT);
    connect(b, Side::LEFT,  a,    Side::RIGHT);
    connect(b, Side::RIGHT, root, Side::RIGHT);
    setHorizontalWeight(a, 1); setHorizontalWeight(b, 0);
    root.setOptimizationLevel(0); // OPTIMIZATION_NONE
    root.layout();
    EXPECT_NEAR(a.getWidth(), 800, 1);
    EXPECT_NEAR(b.getWidth(), 0, 1);
    EXPECT_NEAR(getLeft(a), 0, 1);
    EXPECT_NEAR(getLeft(b), 800, 1);
}

// Ported from androidx.constraintlayout.core.AdvancedChainTest.testChain3Weights (line 198): three
// MATCH widgets, weights 1:0:1 — A and C split 800 (400 each), B zero. OPTIMIZATION_NONE.
TEST(CLCoreAdvancedChain, Chain3Weights1_0_1) {
    ConstraintWidgetContainer root("root", 800, 800);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20), c("C", 100, 20);
    a.setHorizontalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    b.setHorizontalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    c.setHorizontalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    root.add(&a); root.add(&b); root.add(&c);
    using namespace clport;
    connect(a, Side::LEFT,  root, Side::LEFT);
    connect(a, Side::RIGHT, b,    Side::LEFT);
    connect(b, Side::LEFT,  a,    Side::RIGHT);
    connect(b, Side::RIGHT, c,    Side::LEFT);
    connect(c, Side::LEFT,  b,    Side::RIGHT);
    connect(c, Side::RIGHT, root, Side::RIGHT);
    setHorizontalWeight(a, 1); setHorizontalWeight(b, 0); setHorizontalWeight(c, 1);
    root.setOptimizationLevel(0); // OPTIMIZATION_NONE
    root.layout();
    EXPECT_EQ(a.getWidth(), 400);
    EXPECT_EQ(b.getWidth(), 0);
    EXPECT_EQ(c.getWidth(), 400);
    EXPECT_EQ(getLeft(a), 0);
    EXPECT_EQ(getLeft(b), 400);
    EXPECT_EQ(getLeft(c), 400);
}

// Ported from androidx.constraintlayout.core.AdvancedChainTest.testChainLastGone (line 246): four
// widgets in a vertical chain (each centered horizontally to root), B and D GONE — A and C spread
// across 800 (OPTIMIZATION_NONE): A.top=253, C.top=527.
TEST(CLCoreAdvancedChain, ChainLastGone) {
    ConstraintWidgetContainer root("root", 800, 800);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20), c("C", 100, 20), d("D", 100, 20);
    root.add(&a); root.add(&b); root.add(&c); root.add(&d);
    using namespace clport;
    for (auto* w : {&a, &b, &c, &d}) { connect(*w, Side::LEFT, root, Side::LEFT); connect(*w, Side::RIGHT, root, Side::RIGHT); }
    connect(a, Side::TOP,    root, Side::TOP);    connect(a, Side::BOTTOM, b, Side::TOP);
    connect(b, Side::TOP,    a,    Side::BOTTOM); connect(b, Side::BOTTOM, c, Side::TOP);
    connect(c, Side::TOP,    b,    Side::BOTTOM); connect(c, Side::BOTTOM, d, Side::TOP);
    connect(d, Side::TOP,    c,    Side::BOTTOM); connect(d, Side::BOTTOM, root, Side::BOTTOM);
    b.setVisibility(ConstraintWidget::GONE);
    d.setVisibility(ConstraintWidget::GONE);
    root.setOptimizationLevel(0); // OPTIMIZATION_NONE
    root.layout();
    EXPECT_NEAR(getTop(a), 253, 1);
    EXPECT_NEAR(getTop(c), 527, 1);
}

// Ported from androidx.constraintlayout.core.AdvancedChainTest.testTooSmall (line 124): A pinned
// left and centered vertically; B & C offset right of A by 100 and stacked B-C with mutual
// top/bottom constraints — too-tight chain (OPTIMIZATION_NONE): A.top=390, B.top=380, C.top=400.
TEST(CLCoreAdvancedChain, TooSmall) {
    ConstraintWidgetContainer root("root", 800, 800);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20), c("C", 100, 20);
    root.add(&a); root.add(&b); root.add(&c);
    using namespace clport;
    connect(a, Side::LEFT,   root, Side::LEFT);
    connect(a, Side::TOP,    root, Side::TOP);
    connect(a, Side::BOTTOM, root, Side::BOTTOM);
    connect(b, Side::LEFT, a, Side::RIGHT, 100);
    connect(c, Side::LEFT, a, Side::RIGHT, 100);
    connect(b, Side::TOP,    a, Side::TOP);
    connect(b, Side::BOTTOM, c, Side::TOP);
    connect(c, Side::TOP,    b, Side::BOTTOM);
    connect(c, Side::BOTTOM, a, Side::BOTTOM);
    root.setOptimizationLevel(0); // OPTIMIZATION_NONE
    root.layout();
    EXPECT_EQ(getTop(a), 390);
    EXPECT_EQ(getTop(b), 380);
    EXPECT_EQ(getTop(c), 400);
}

// Ported from androidx.constraintlayout.core.AdvancedChainTest.testComplexChainWeights (line 35),
// case a: A/B both MATCH_CONSTRAINT on both axes, centered horizontally, vertical chain — equal
// split (800x400 each). OPTIMIZATION_NONE. (Later legs add ratio/weight; multi-layout, omitted.)
TEST(CLCoreAdvancedChain, ComplexChainWeightsEqual) {
    ConstraintWidgetContainer root("root", 800, 800);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20);
    a.setHorizontalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    a.setVerticalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    b.setHorizontalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    b.setVerticalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    root.add(&a); root.add(&b);
    using namespace clport;
    connect(a, Side::LEFT, root, Side::LEFT); connect(a, Side::RIGHT, root, Side::RIGHT);
    connect(b, Side::LEFT, root, Side::LEFT); connect(b, Side::RIGHT, root, Side::RIGHT);
    connect(a, Side::TOP,    root, Side::TOP);    connect(a, Side::BOTTOM, b, Side::TOP);
    connect(b, Side::TOP,    a,    Side::BOTTOM); connect(b, Side::BOTTOM, root, Side::BOTTOM);
    root.setOptimizationLevel(0); // OPTIMIZATION_NONE
    root.layout();
    EXPECT_EQ(a.getWidth(), 800);
    EXPECT_EQ(b.getWidth(), 800);
    EXPECT_EQ(a.getHeight(), 400);
    EXPECT_EQ(b.getHeight(), 400);
    EXPECT_EQ(getTop(a), 0);
    EXPECT_EQ(getTop(b), 400);
}

// Ported from androidx.constraintlayout.core.AdvancedChainTest.testRatioChainGone (line 299):
// A/B/C centered horizontally, chained vertically against a ratio widget (4:3); B&C GONE —
// A.height=600. Then root WRAP_CONTENT, A.height stays 600. OPTIMIZATION_NONE.
TEST(CLCoreAdvancedChain, RatioChainGone) {
    ConstraintWidgetContainer root("root", 800, 800);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20), c("C", 100, 20), ratio("ratio", 100, 20);
    a.setHorizontalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    b.setHorizontalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    c.setHorizontalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    ratio.setHorizontalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    a.setVerticalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    b.setVerticalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    c.setVerticalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    ratio.setVerticalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    clport::setDimensionRatio(ratio, "4:3");
    root.add(&a); root.add(&b); root.add(&c); root.add(&ratio);
    using namespace clport;
    connect(a, Side::LEFT, root, Side::LEFT); connect(a, Side::RIGHT, root, Side::RIGHT);
    connect(b, Side::LEFT, root, Side::LEFT); connect(b, Side::RIGHT, root, Side::RIGHT);
    connect(c, Side::LEFT, root, Side::LEFT); connect(c, Side::RIGHT, root, Side::RIGHT);
    connect(ratio, Side::TOP, root, Side::TOP); connect(ratio, Side::LEFT, root, Side::LEFT); connect(ratio, Side::RIGHT, root, Side::RIGHT);
    connect(a, Side::TOP,    root,  Side::TOP);
    connect(a, Side::BOTTOM, b,     Side::TOP);
    connect(b, Side::BOTTOM, ratio, Side::BOTTOM);
    connect(c, Side::TOP,    b,     Side::TOP);
    connect(c, Side::BOTTOM, ratio, Side::BOTTOM);
    b.setVisibility(ConstraintWidget::GONE);
    c.setVisibility(ConstraintWidget::GONE);
    root.setOptimizationLevel(0); // OPTIMIZATION_NONE
    root.layout();
    EXPECT_EQ(a.getHeight(), 600);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::WRAP_CONTENT);
    root.layout();
    EXPECT_EQ(a.getHeight(), 600);
}
