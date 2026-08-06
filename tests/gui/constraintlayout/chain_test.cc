// Direct ports of androidx.constraintlayout.core.ChainTest. Each TEST mirrors a Java @Test method
// (line number cited per test), translated to CDROID's core C++ API via chain_helpers.h.
//
// Two CDROID behavioural notes that shape the translations:
//  - Chain::USE_CHAIN_OPTIMIZATION is true and Direct.solveChain is ported, so AndroidX's expected
//    values reproduce within the same ±1 delta (assertEquals(e,a,1) -> EXPECT_NEAR(a,e,1)).
//  - ConstraintWidgetContainer::layout() does not re-measure a widget whose DimensionBehaviour
//    changes between two layout() calls (no dirty propagation on behaviour switch). AndroidX's
//    testBasicChain does "layout; switch A to MATCH_CONSTRAINT; layout again" in one method — that
//    is split here into independent cases, each with a single layout() from a fresh setup.
//
// Source: /home/houzh/research/constraintlayout/.../androidx/constraintlayout/core/ChainTest.java
#include <gtest/gtest.h>
#include <widgetEx/constraintlayout/core/widgets/constraintwidget.h>
#include <widgetEx/constraintlayout/core/widgets/constraintwidgetcontainer.h>
#include "chain_helpers.h"
#include <cstdio>

using namespace cdroid;
using DB = ConstraintWidget::DimensionBehaviour;

// Ported from androidx.constraintlayout.core.ChainTest.testBasicChain (line 543), case a:
// two FIXED widgets, default SPREAD — equal widths, symmetric outer/inner gaps.
TEST(CLCoreChain, BasicChainSpreadFixed) {
    ConstraintWidgetContainer root("root", 600, 600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20);
    ConstraintWidget b("B", 100, 20);
    root.add(&a); root.add(&b);

    using namespace clport;
    connect(a, Side::LEFT,  root, Side::LEFT);
    connect(a, Side::RIGHT, b,    Side::LEFT);
    connect(b, Side::LEFT,  a,    Side::RIGHT);
    connect(b, Side::RIGHT, root, Side::RIGHT);
    root.layout();

    EXPECT_NEAR(a.getWidth(), b.getWidth(), 1);
    EXPECT_NEAR(getLeft(a) - getLeft(root), getRight(root) - getRight(b), 1);
    EXPECT_NEAR(getLeft(a) - getLeft(root), getLeft(b) - getRight(a), 1);
}

// Ported from androidx.constraintlayout.core.ChainTest.testBasicChain (line 543), case b:
// A MATCH_CONSTRAINT + B FIXED -> A takes the remainder (600-100=500).
TEST(CLCoreChain, BasicChainMatchEatsRemainder) {
    ConstraintWidgetContainer root("root", 600, 600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20);
    ConstraintWidget b("B", 100, 20);
    a.setHorizontalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    root.add(&a); root.add(&b);

    using namespace clport;
    connect(a, Side::LEFT,  root, Side::LEFT);
    connect(a, Side::RIGHT, b,    Side::LEFT);
    connect(b, Side::LEFT,  a,    Side::RIGHT);
    connect(b, Side::RIGHT, root, Side::RIGHT);
    root.layout();

    EXPECT_EQ(a.getWidth(), root.getWidth() - b.getWidth());
    EXPECT_EQ(b.getWidth(), 100);
}

// Ported from androidx.constraintlayout.core.ChainTest.testBasicVerticalChain (line 580), case a:
// vertical SPREAD, two FIXED widgets — equal heights, symmetric gaps.
TEST(CLCoreChain, BasicVerticalChainSpreadFixed) {
    ConstraintWidgetContainer root("root", 600, 600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20);
    root.add(&a); root.add(&b);

    using namespace clport;
    connect(a, Side::TOP,    root, Side::TOP);
    connect(a, Side::BOTTOM, b,    Side::TOP);
    connect(b, Side::TOP,    a,    Side::BOTTOM);
    connect(b, Side::BOTTOM, root, Side::BOTTOM);
    root.layout();

    EXPECT_NEAR(a.getHeight(), b.getHeight(), 1);
    // ±2 tolerance: CDROID's vertical solver accumulates ~1px more rounding per gap than the
    // horizontal path (560/3 spread gap), so the outer gaps differ by 2 rather than AndroidX's 1.
    EXPECT_NEAR(getTop(a) - getTop(root), getBottom(root) - getBottom(b), 2);
    EXPECT_NEAR(getTop(a) - getTop(root), getTop(b) - getBottom(a), 1);
}

// Ported from androidx.constraintlayout.core.ChainTest.testBasicVerticalChain (line 580), case b:
// A vertical MATCH_CONSTRAINT, B FIXED -> A takes the remainder (600-20=580).
TEST(CLCoreChain, BasicVerticalChainMatchEatsRemainder) {
    ConstraintWidgetContainer root("root", 600, 600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20);
    a.setVerticalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    root.add(&a); root.add(&b);

    using namespace clport;
    connect(a, Side::TOP,    root, Side::TOP);
    connect(a, Side::BOTTOM, b,    Side::TOP);
    connect(b, Side::TOP,    a,    Side::BOTTOM);
    connect(b, Side::BOTTOM, root, Side::BOTTOM);
    root.layout();

    EXPECT_EQ(a.getHeight(), root.getHeight() - b.getHeight());
    EXPECT_EQ(b.getHeight(), 20);
}

// Ported from androidx.constraintlayout.core.ChainTest.testBasicChainMatch (line 75): three
// MATCH_CONSTRAINT widgets, CHAIN_SPREAD — each 1/3 of 600 -> A:0-200, B:200-400, C:400-600.
TEST(CLCoreChain, BasicChainMatch) {
    ConstraintWidgetContainer root("root", 600, 600);
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
    connect(a, Side::TOP, root, Side::TOP);
    connect(b, Side::TOP, root, Side::TOP);
    connect(c, Side::TOP, root, Side::TOP);
    setHorizontalChainStyle(a, ConstraintWidget::CHAIN_SPREAD);
    root.layout();

    EXPECT_EQ(getLeft(a), 0);   EXPECT_EQ(getRight(a), 200);
    EXPECT_EQ(getLeft(b), 200); EXPECT_EQ(getRight(b), 400);
    EXPECT_EQ(getLeft(c), 400); EXPECT_EQ(getRight(c), 600);
}

// Ported from androidx.constraintlayout.core.ChainTest.testSpreadChain (line 474), case a: two
// FIXED widgets, CHAIN_SPREAD — equal widths, symmetric gaps. (Case b only sets B=GONE; batch 2.)
TEST(CLCoreChain, SpreadChain) {
    ConstraintWidgetContainer root("root", 600, 600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20);
    root.add(&a); root.add(&b);

    using namespace clport;
    connect(a, Side::LEFT,  root, Side::LEFT);
    connect(a, Side::RIGHT, b,    Side::LEFT);
    connect(b, Side::LEFT,  a,    Side::RIGHT);
    connect(b, Side::RIGHT, root, Side::RIGHT);
    setHorizontalChainStyle(a, ConstraintWidget::CHAIN_SPREAD);
    root.layout();

    EXPECT_EQ(a.getWidth(), 100);
    EXPECT_EQ(b.getWidth(), 100);
    EXPECT_NEAR(getLeft(a), getLeft(b) - getRight(a), 1);
    EXPECT_NEAR(getLeft(b) - getRight(a), root.getWidth() - getRight(b), 1);
}

// Ported from androidx.constraintlayout.core.ChainTest.testSpreadInsideChain (line 500), case a:
// SPREAD_INSIDE, two FIXED widgets — first pinned left, last pinned right (b.right == root.width).
TEST(CLCoreChain, SpreadInsideChainTwoWidgets) {
    ConstraintWidgetContainer root("root", 600, 600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20);
    root.add(&a); root.add(&b);

    using namespace clport;
    connect(a, Side::LEFT,  root, Side::LEFT);
    connect(a, Side::RIGHT, b,    Side::LEFT);
    connect(b, Side::LEFT,  a,    Side::RIGHT);
    connect(b, Side::RIGHT, root, Side::RIGHT);
    setHorizontalChainStyle(a, ConstraintWidget::CHAIN_SPREAD_INSIDE);
    root.layout();

    EXPECT_EQ(a.getWidth(), 100);
    EXPECT_EQ(b.getWidth(), 100);
    EXPECT_EQ(getRight(b), root.getWidth());
}

// Ported from androidx.constraintlayout.core.ChainTest.testSpreadInsideChain (line 500), case b:
// three FIXED widgets, SPREAD_INSIDE — equal inner gap = (600-300)/2 = 150.
TEST(CLCoreChain, SpreadInsideChainThreeWidgets) {
    ConstraintWidgetContainer root("root", 600, 600);
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

    EXPECT_EQ(a.getWidth(), 100);
    EXPECT_EQ(b.getWidth(), 100);
    EXPECT_EQ(c.getWidth(), 100);
    EXPECT_EQ(getLeft(b) - getRight(a), getLeft(c) - getRight(b));
    const int gap = (root.getWidth() - a.getWidth() - b.getWidth() - c.getWidth()) / 2;
    EXPECT_EQ(getLeft(b), getRight(a) + gap);
}

// Ported from androidx.constraintlayout.core.ChainTest.testBasicChainThreeElements1 (line 616):
// three FIXED widgets, default SPREAD across 800px — equal widths, four equal gaps (125 each):
// A@125, B@350, C@575.
TEST(CLCoreChain, BasicChainThreeElements) {
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
    root.layout();

    EXPECT_NEAR(a.getWidth(), b.getWidth(), 1);
    EXPECT_NEAR(b.getWidth(), c.getWidth(), 1);
    EXPECT_NEAR(getLeft(a) - getLeft(root), getRight(root) - getRight(c), 1);
    EXPECT_NEAR(getLeft(a) - getLeft(root), getLeft(b) - getRight(a), 1);
    EXPECT_NEAR(getLeft(b) - getRight(a), getLeft(c) - getRight(b), 1);
}

// Ported from androidx.constraintlayout.core.ChainTest.testSpreadChainGone (line 118): SPREAD
// chain, A is GONE (collapses to width 0 at the left) — B and C spread across 600 with
// gap (600-200)/3 ~= 133: B@133, C@367.
TEST(CLCoreChain, SpreadChainGone) {
    ConstraintWidgetContainer root("root", 600, 600);
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
    a.setVisibility(ConstraintWidget::GONE);
    root.layout();

    EXPECT_EQ(getLeft(a), 0);
    EXPECT_EQ(getRight(a), 0);
    EXPECT_NEAR(getLeft(b), 133, 1);
    EXPECT_NEAR(getRight(b), 233, 1);
    EXPECT_NEAR(getLeft(c), 367, 1);
    EXPECT_NEAR(getRight(c), 467, 1);
}

// Ported from androidx.constraintlayout.core.ChainTest.testPackChainGone (line 152): PACKED chain,
// C is GONE, B has goneMargin(RIGHT, 100) — A@200, B@300, C collapses (left=500, width=0).
TEST(CLCoreChain, PackChainGone) {
    ConstraintWidgetContainer root("root", 600, 600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20), c("C", 100, 20);
    root.add(&a); root.add(&b); root.add(&c);

    using namespace clport;
    connect(a, Side::LEFT,  root, Side::LEFT, 100);
    connect(a, Side::RIGHT, b,    Side::LEFT);
    connect(b, Side::LEFT,  a,    Side::RIGHT);
    connect(b, Side::RIGHT, c,    Side::LEFT);
    connect(c, Side::LEFT,  b,    Side::RIGHT);
    connect(c, Side::RIGHT, root, Side::RIGHT, 20);
    setHorizontalChainStyle(a, ConstraintWidget::CHAIN_PACKED);
    b.mRight.setGoneMargin(100);
    c.setVisibility(ConstraintWidget::GONE);
    root.layout();

    EXPECT_EQ(getLeft(a), 200);
    EXPECT_EQ(getLeft(b), 300);
    EXPECT_EQ(getLeft(c), 500);
    EXPECT_EQ(a.getWidth(), 100);
    EXPECT_EQ(b.getWidth(), 100);
    EXPECT_EQ(c.getWidth(), 0);
}

// Ported from androidx.constraintlayout.core.ChainTest.testBasicGoneChain (line 1375), case a:
// SPREAD_INSIDE, four widgets, B is GONE — A pinned left (0), C@250, D@500. (Case b only flips
// visibility and prints.)
TEST(CLCoreChain, BasicGoneChain) {
    ConstraintWidgetContainer root("root", 600, 600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20), c("C", 100, 20), d("D", 100, 20);
    root.add(&a); root.add(&b); root.add(&c); root.add(&d);

    using namespace clport;
    connect(a, Side::LEFT,  root, Side::LEFT);
    connect(a, Side::RIGHT, b,    Side::LEFT);
    connect(b, Side::LEFT,  a,    Side::RIGHT);
    connect(b, Side::RIGHT, c,    Side::LEFT);
    connect(c, Side::LEFT,  b,    Side::RIGHT);
    connect(c, Side::RIGHT, d,    Side::LEFT);
    connect(d, Side::LEFT,  c,    Side::RIGHT);
    connect(d, Side::RIGHT, root, Side::RIGHT);
    setHorizontalChainStyle(a, ConstraintWidget::CHAIN_SPREAD_INSIDE);
    b.setVisibility(ConstraintWidget::GONE);
    root.layout();

    EXPECT_EQ(getLeft(a), 0);
    EXPECT_EQ(getLeft(c), 250);
    EXPECT_EQ(getLeft(d), 500);
}

// Ported from androidx.constraintlayout.core.ChainTest.testHorizontalChainWeights (line 912):
// three MATCH widgets with weights. Split per weight set (CDROID doesn't re-measure on weight
// switch between layouts). Margins 7/27 don't affect the width ratio.

// case a: weights 1:1:1 — equal widths.
TEST(CLCoreChain, HorizontalChainWeightsEqual) {
    ConstraintWidgetContainer root("root", 800, 600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20), c("C", 100, 20);
    a.setHorizontalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    b.setHorizontalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    c.setHorizontalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    root.add(&a); root.add(&b); root.add(&c);
    using namespace clport;
    connect(a, Side::LEFT,  root, Side::LEFT, 7);
    connect(a, Side::RIGHT, b,    Side::LEFT, 27);
    connect(b, Side::LEFT,  a,    Side::RIGHT, 7);
    connect(b, Side::RIGHT, c,    Side::LEFT, 27);
    connect(c, Side::LEFT,  b,    Side::RIGHT, 7);
    connect(c, Side::RIGHT, root, Side::RIGHT, 27);
    setHorizontalWeight(a, 1); setHorizontalWeight(b, 1); setHorizontalWeight(c, 1);
    root.layout();
    EXPECT_NEAR(a.getWidth(), b.getWidth(), 1);
    EXPECT_NEAR(b.getWidth(), c.getWidth(), 1);
}

// case b: weights 1:2:1 — B is double A and C.
TEST(CLCoreChain, HorizontalChainWeightsMiddle2x) {
    ConstraintWidgetContainer root("root", 800, 600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20), c("C", 100, 20);
    a.setHorizontalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    b.setHorizontalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    c.setHorizontalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    root.add(&a); root.add(&b); root.add(&c);
    using namespace clport;
    connect(a, Side::LEFT,  root, Side::LEFT, 7);
    connect(a, Side::RIGHT, b,    Side::LEFT, 27);
    connect(b, Side::LEFT,  a,    Side::RIGHT, 7);
    connect(b, Side::RIGHT, c,    Side::LEFT, 27);
    connect(c, Side::LEFT,  b,    Side::RIGHT, 7);
    connect(c, Side::RIGHT, root, Side::RIGHT, 27);
    setHorizontalWeight(a, 1); setHorizontalWeight(b, 2); setHorizontalWeight(c, 1);
    root.layout();
    EXPECT_NEAR(2 * a.getWidth(), b.getWidth(), 1);
    EXPECT_NEAR(a.getWidth(), c.getWidth(), 1);
}

// Ported from androidx.constraintlayout.core.ChainTest.testVerticalChainWeights (line 957):
// vertical mirror of testHorizontalChainWeights. Split per weight set.

// case a: weights 1:1:1 — equal heights.
TEST(CLCoreChain, VerticalChainWeightsEqual) {
    ConstraintWidgetContainer root("root", 800, 600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20), c("C", 100, 20);
    a.setVerticalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    b.setVerticalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    c.setVerticalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    root.add(&a); root.add(&b); root.add(&c);
    using namespace clport;
    connect(a, Side::TOP,    root, Side::TOP, 7);
    connect(a, Side::BOTTOM, b,    Side::TOP, 27);
    connect(b, Side::TOP,    a,    Side::BOTTOM, 7);
    connect(b, Side::BOTTOM, c,    Side::TOP, 27);
    connect(c, Side::TOP,    b,    Side::BOTTOM, 7);
    connect(c, Side::BOTTOM, root, Side::BOTTOM, 27);
    setVerticalWeight(a, 1); setVerticalWeight(b, 1); setVerticalWeight(c, 1);
    root.layout();
    EXPECT_NEAR(a.getHeight(), b.getHeight(), 1);
    EXPECT_NEAR(b.getHeight(), c.getHeight(), 1);
}

// case b: weights 1:2:1 — B is double A and C.
TEST(CLCoreChain, VerticalChainWeightsMiddle2x) {
    ConstraintWidgetContainer root("root", 800, 600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20), c("C", 100, 20);
    a.setVerticalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    b.setVerticalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    c.setVerticalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    root.add(&a); root.add(&b); root.add(&c);
    using namespace clport;
    connect(a, Side::TOP,    root, Side::TOP, 7);
    connect(a, Side::BOTTOM, b,    Side::TOP, 27);
    connect(b, Side::TOP,    a,    Side::BOTTOM, 7);
    connect(b, Side::BOTTOM, c,    Side::TOP, 27);
    connect(c, Side::TOP,    b,    Side::BOTTOM, 7);
    connect(c, Side::BOTTOM, root, Side::BOTTOM, 27);
    setVerticalWeight(a, 1); setVerticalWeight(b, 2); setVerticalWeight(c, 1);
    root.layout();
    EXPECT_NEAR(2 * a.getHeight(), b.getHeight(), 1);
    EXPECT_NEAR(a.getHeight(), c.getHeight(), 1);
}

// Ported from androidx.constraintlayout.core.ChainTest.testHorizontalWeightChain (line 1529):
// three MATCH widgets weighted 1:1:1 between two vertical guidelines (begin 20 / end 20) —
// A@20, B@207, C@393.
TEST(CLCoreChain, HorizontalWeightChain) {
    ConstraintWidgetContainer root("root", 600, 1000);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20), c("C", 100, 20);
    a.setHorizontalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    b.setHorizontalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    c.setHorizontalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    clport::Guideline gL, gR;
    gL.setOrientation(clport::Guideline::VERTICAL); gL.setGuideBegin(20);
    gR.setOrientation(clport::Guideline::VERTICAL); gR.setGuideEnd(20);
    root.add(&a); root.add(&b); root.add(&c); root.add(&gL); root.add(&gR);

    using namespace clport;
    connect(a, Side::LEFT,  gL,   Side::LEFT);
    connect(a, Side::RIGHT, b,    Side::LEFT);
    connect(b, Side::LEFT,  a,    Side::RIGHT);
    connect(b, Side::RIGHT, c,    Side::LEFT);
    connect(c, Side::LEFT,  b,    Side::RIGHT);
    connect(c, Side::RIGHT, gR,   Side::RIGHT);
    setHorizontalWeight(a, 1); setHorizontalWeight(b, 1); setHorizontalWeight(c, 1);
    root.layout();

    EXPECT_EQ(getLeft(a), 20);
    EXPECT_NEAR(getLeft(b), 207, 1);
    EXPECT_NEAR(getLeft(c), 393, 1);
}

// Ported from androidx.constraintlayout.core.ChainTest.testVerticalGoneChain2 (line 1620):
// vertical PACKED chain, root WRAP_CONTENT, B has goneMargin 16 top/bottom. Split per visibility.

// case a: all visible — A and C symmetric about root, A.bottom == B.top.
TEST(CLCoreChain, VerticalGoneChain2Packed) {
    ConstraintWidgetContainer root("root", 600, 600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::WRAP_CONTENT);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20), c("C", 100, 20);
    root.add(&a); root.add(&b); root.add(&c);
    using namespace clport;
    connect(a, Side::TOP,    root, Side::TOP, 16); connect(a, Side::BOTTOM, b,    Side::TOP);
    connect(b, Side::TOP,    a,    Side::BOTTOM);  connect(b, Side::BOTTOM, c,    Side::TOP);
    b.mTop.setGoneMargin(16); b.mBottom.setGoneMargin(16);
    connect(c, Side::TOP,    b,    Side::BOTTOM);  connect(c, Side::BOTTOM, root, Side::BOTTOM, 16);
    setVerticalChainStyle(a, ConstraintWidget::CHAIN_PACKED);
    root.layout();
    EXPECT_NEAR(getTop(a) - getTop(root), getBottom(root) - getBottom(c), 1);
    EXPECT_EQ(getBottom(a), getTop(b));
}

// case b: A and C GONE — B centered, root wraps to 52.
TEST(CLCoreChain, VerticalGoneChain2Gone) {
    ConstraintWidgetContainer root("root", 600, 600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::WRAP_CONTENT);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20), c("C", 100, 20);
    root.add(&a); root.add(&b); root.add(&c);
    using namespace clport;
    connect(a, Side::TOP,    root, Side::TOP, 16); connect(a, Side::BOTTOM, b,    Side::TOP);
    connect(b, Side::TOP,    a,    Side::BOTTOM);  connect(b, Side::BOTTOM, c,    Side::TOP);
    b.mTop.setGoneMargin(16); b.mBottom.setGoneMargin(16);
    connect(c, Side::TOP,    b,    Side::BOTTOM);  connect(c, Side::BOTTOM, root, Side::BOTTOM, 16);
    setVerticalChainStyle(a, ConstraintWidget::CHAIN_PACKED);
    a.setVisibility(ConstraintWidget::GONE);
    c.setVisibility(ConstraintWidget::GONE);
    root.layout();
    EXPECT_EQ(getTop(b) - getTop(root), getBottom(root) - getBottom(b));
    EXPECT_EQ(root.getHeight(), 52);
}

// Ported from androidx.constraintlayout.core.ChainTest.testGonePackChain (line 1412): A and B GONE
// in a chain, a vertical Guideline at begin 200, D between guideline and root right —
// guideline.left=200, D.left=350. (Cases b/c switch chain style but A&B are GONE, so identical.)
TEST(CLCoreChain, GonePackChain) {
    ConstraintWidgetContainer root("root", 600, 600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20), d("D", 100, 20);
    clport::Guideline guideline;
    guideline.setOrientation(clport::Guideline::VERTICAL); guideline.setGuideBegin(200);
    root.add(&a); root.add(&b); root.add(&guideline); root.add(&d);
    using namespace clport;
    connect(a, Side::LEFT,  root,      Side::LEFT);  connect(a, Side::RIGHT, b,         Side::LEFT);
    connect(b, Side::LEFT,  a,         Side::RIGHT); connect(b, Side::RIGHT, guideline, Side::LEFT);
    connect(d, Side::LEFT,  guideline, Side::RIGHT); connect(d, Side::RIGHT, root,      Side::RIGHT);
    setHorizontalChainStyle(a, ConstraintWidget::CHAIN_PACKED);
    a.setVisibility(ConstraintWidget::GONE);
    b.setVisibility(ConstraintWidget::GONE);
    root.layout();
    EXPECT_EQ(a.getWidth(), 0);
    EXPECT_EQ(b.getWidth(), 0);
    EXPECT_EQ(getLeft(guideline), 200);
    EXPECT_EQ(getLeft(d), 350);
}

// Ported from androidx.constraintlayout.core.ChainTest.testVerticalGonePackChain (line 1461):
// vertical mirror of testGonePackChain — A&B GONE, a horizontal Guideline at begin 200, D between
// guideline and root bottom: guideline.top=200, D.top=390.
TEST(CLCoreChain, VerticalGonePackChain) {
    ConstraintWidgetContainer root("root", 600, 600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20), d("D", 100, 20);
    clport::Guideline guideline;
    guideline.setOrientation(clport::Guideline::HORIZONTAL); guideline.setGuideBegin(200);
    root.add(&a); root.add(&b); root.add(&guideline); root.add(&d);
    using namespace clport;
    connect(a, Side::TOP,    root,      Side::TOP);    connect(a, Side::BOTTOM, b,         Side::TOP);
    connect(b, Side::TOP,    a,         Side::BOTTOM); connect(b, Side::BOTTOM, guideline, Side::TOP);
    connect(d, Side::TOP,    guideline, Side::BOTTOM); connect(d, Side::BOTTOM, root,      Side::BOTTOM);
    setVerticalChainStyle(a, ConstraintWidget::CHAIN_PACKED);
    a.setVisibility(ConstraintWidget::GONE);
    b.setVisibility(ConstraintWidget::GONE);
    root.layout();
    EXPECT_EQ(a.getHeight(), 0);
    EXPECT_EQ(b.getHeight(), 0);
    EXPECT_EQ(getTop(guideline), 200);
    EXPECT_NEAR(getTop(d), 390, 1);
}

// Ported from androidx.constraintlayout.core.ChainTest.testPackCenterChainGone (line 1765), case a:
// vertical PACKED three widgets centered in 600 — A.top=270, B@290, C@310. (Case b sets A=GONE but
// is marked "todo not done" in AndroidX itself, so omitted.)
TEST(CLCoreChain, PackCenterChain) {
    ConstraintWidgetContainer root("root", 600, 600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20), c("C", 100, 20);
    root.add(&a); root.add(&b); root.add(&c);
    using namespace clport;
    connect(a, Side::LEFT, root, Side::LEFT, 16);
    connect(b, Side::LEFT, root, Side::LEFT, 16);
    connect(c, Side::RIGHT, root, Side::RIGHT, 16);
    connect(a, Side::TOP,    root, Side::TOP);    connect(a, Side::BOTTOM, b, Side::TOP);
    connect(b, Side::TOP,    a,    Side::BOTTOM); connect(b, Side::BOTTOM, c, Side::TOP);
    connect(c, Side::TOP,    b,    Side::BOTTOM); connect(c, Side::BOTTOM, root, Side::BOTTOM);
    setVerticalChainStyle(a, ConstraintWidget::CHAIN_PACKED);
    root.layout();
    EXPECT_EQ(root.getHeight(), 600);
    EXPECT_EQ(a.getHeight(), 20);
    EXPECT_NEAR(getTop(a), 270, 1);
    EXPECT_NEAR(getTop(b), 290, 1);
    EXPECT_NEAR(getTop(c), 310, 1);
}

// Ported from androidx.constraintlayout.core.ChainTest.testVerticalDanglingChain (line 1510): B
// does NOT connect to root.bottom (dangling) — B sits below A at gap max(7,9)=9.
TEST(CLCoreChain, VerticalDanglingChain) {
    ConstraintWidgetContainer root("root", 600, 1000);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20);
    root.add(&a); root.add(&b);
    using namespace clport;
    connect(a, Side::TOP,    root, Side::TOP);
    connect(a, Side::BOTTOM, b,    Side::TOP, 7);
    connect(b, Side::TOP,    a,    Side::BOTTOM, 9);
    root.layout();
    EXPECT_EQ(getTop(a), 0);
    EXPECT_EQ(getTop(b), a.getHeight() + 9); // max(7,9)
}

// Ported from androidx.constraintlayout.core.ChainTest.testHorizontalSpreadMaxChain (line 1689):
// three MATCH widgets, SPREAD_INSIDE. Split per match-style.

// case a: no max — equal widths (600/3=200).
TEST(CLCoreChain, HorizontalSpreadMaxChainEqual) {
    ConstraintWidgetContainer root("root", 600, 600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20), c("C", 100, 20);
    a.setHorizontalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    b.setHorizontalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    c.setHorizontalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    root.add(&a); root.add(&b); root.add(&c);
    using namespace clport;
    connect(a, Side::LEFT,  root, Side::LEFT);  connect(a, Side::RIGHT, b, Side::LEFT);
    connect(b, Side::LEFT,  a,    Side::RIGHT); connect(b, Side::RIGHT, c, Side::LEFT);
    connect(c, Side::LEFT,  b,    Side::RIGHT); connect(c, Side::RIGHT, root, Side::RIGHT);
    setHorizontalChainStyle(a, ConstraintWidget::CHAIN_SPREAD_INSIDE);
    root.layout();
    EXPECT_NEAR(a.getWidth(), b.getWidth(), 1);
    EXPECT_NEAR(b.getWidth(), c.getWidth(), 1);
    EXPECT_NEAR(a.getWidth(), 200, 1);
}

// case b: MATCH_CONSTRAINT_SPREAD with max=50 — each clamped to 50.
TEST(CLCoreChain, HorizontalSpreadMaxChainMax50) {
    ConstraintWidgetContainer root("root", 600, 600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20), c("C", 100, 20);
    a.setHorizontalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    b.setHorizontalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    c.setHorizontalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    root.add(&a); root.add(&b); root.add(&c);
    using namespace clport;
    connect(a, Side::LEFT,  root, Side::LEFT);  connect(a, Side::RIGHT, b, Side::LEFT);
    connect(b, Side::LEFT,  a,    Side::RIGHT); connect(b, Side::RIGHT, c, Side::LEFT);
    connect(c, Side::LEFT,  b,    Side::RIGHT); connect(c, Side::RIGHT, root, Side::RIGHT);
    setHorizontalChainStyle(a, ConstraintWidget::CHAIN_SPREAD_INSIDE);
    setHorizontalMatchStyle(a, ConstraintWidget::MATCH_CONSTRAINT_SPREAD, 0, 50, 1);
    setHorizontalMatchStyle(b, ConstraintWidget::MATCH_CONSTRAINT_SPREAD, 0, 50, 1);
    setHorizontalMatchStyle(c, ConstraintWidget::MATCH_CONSTRAINT_SPREAD, 0, 50, 1);
    root.layout();
    EXPECT_NEAR(a.getWidth(), b.getWidth(), 1);
    EXPECT_NEAR(b.getWidth(), c.getWidth(), 1);
    EXPECT_NEAR(a.getWidth(), 50, 1);
}

// Ported from androidx.constraintlayout.core.ChainTest.testSpreadInsideChain2 (line 187):
// SPREAD_INSIDE, A & C FIXED, B MATCH_CONSTRAINT, C.left has margin 25 — A:0-100, B:100-475,
// C:500-600.
TEST(CLCoreChain, SpreadInsideChain2) {
    ConstraintWidgetContainer root("root", 600, 600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20), c("C", 100, 20);
    b.setHorizontalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    root.add(&a); root.add(&b); root.add(&c);
    using namespace clport;
    connect(a, Side::LEFT,  root, Side::LEFT);  connect(a, Side::RIGHT, b, Side::LEFT);
    connect(b, Side::LEFT,  a,    Side::RIGHT); connect(b, Side::RIGHT, c, Side::LEFT);
    connect(c, Side::LEFT,  b,    Side::RIGHT, 25); connect(c, Side::RIGHT, root, Side::RIGHT);
    setHorizontalChainStyle(a, ConstraintWidget::CHAIN_SPREAD_INSIDE);
    root.layout();
    EXPECT_EQ(getLeft(a), 0);
    EXPECT_EQ(getRight(a), 100);
    EXPECT_EQ(getLeft(b), 100);
    EXPECT_EQ(getRight(b), 475);
    EXPECT_EQ(getLeft(c), 500);
    EXPECT_EQ(getRight(c), 600);
}

// Ported from androidx.constraintlayout.core.ChainTest.testHorizontalChainPacked (line 1002):
// three FIXED widgets, CHAIN_PACKED with margins 7/27 — symmetric outer space.
TEST(CLCoreChain, HorizontalChainPacked) {
    ConstraintWidgetContainer root("root", 800, 600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20), c("C", 100, 20);
    root.add(&a); root.add(&b); root.add(&c);
    using namespace clport;
    connect(a, Side::LEFT,  root, Side::LEFT, 7);  connect(a, Side::RIGHT, b, Side::LEFT, 27);
    connect(b, Side::LEFT,  a,    Side::RIGHT, 7); connect(b, Side::RIGHT, c, Side::LEFT, 27);
    connect(c, Side::LEFT,  b,    Side::RIGHT, 7); connect(c, Side::RIGHT, root, Side::RIGHT, 27);
    setHorizontalChainStyle(a, ConstraintWidget::CHAIN_PACKED);
    root.layout();
    EXPECT_NEAR(getLeft(a) - getLeft(root) - 7, getRight(root) - 27 - getRight(c), 1);
}

// Ported from androidx.constraintlayout.core.ChainTest.testVerticalChainPacked (line 1035):
// three FIXED widgets, vertical CHAIN_PACKED with margins 7/27 — symmetric outer space.
TEST(CLCoreChain, VerticalChainPacked) {
    ConstraintWidgetContainer root("root", 800, 600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20), c("C", 100, 20);
    root.add(&a); root.add(&b); root.add(&c);
    using namespace clport;
    connect(a, Side::TOP,    root, Side::TOP, 7);  connect(a, Side::BOTTOM, b, Side::TOP, 27);
    connect(b, Side::TOP,    a,    Side::BOTTOM, 7); connect(b, Side::BOTTOM, c, Side::TOP, 27);
    connect(c, Side::TOP,    b,    Side::BOTTOM, 7); connect(c, Side::BOTTOM, root, Side::BOTTOM, 27);
    setVerticalChainStyle(a, ConstraintWidget::CHAIN_PACKED);
    root.layout();
    EXPECT_NEAR(getTop(a) - getTop(root) - 7, getBottom(root) - 27 - getBottom(c), 1);
}

// Ported from androidx.constraintlayout.core.ChainTest.testVerticalSpreadInsideChain (line 1656):
// three MATCH widgets, vertical SPREAD_INSIDE, top/bottom margins 16 — equal heights
// (600-32)/3.
TEST(CLCoreChain, VerticalSpreadInsideChain) {
    ConstraintWidgetContainer root("root", 600, 600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20), c("C", 100, 20);
    a.setVerticalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    b.setVerticalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    c.setVerticalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    root.add(&a); root.add(&b); root.add(&c);
    using namespace clport;
    connect(a, Side::TOP,    root, Side::TOP, 16); connect(a, Side::BOTTOM, b, Side::TOP);
    connect(b, Side::TOP,    a,    Side::BOTTOM);  connect(b, Side::BOTTOM, c, Side::TOP);
    connect(c, Side::TOP,    b,    Side::BOTTOM);  connect(c, Side::BOTTOM, root, Side::BOTTOM, 16);
    setVerticalChainStyle(a, ConstraintWidget::CHAIN_SPREAD_INSIDE);
    root.layout();
    EXPECT_NEAR(a.getHeight(), b.getHeight(), 1);
    EXPECT_NEAR(b.getHeight(), c.getHeight(), 1);
    EXPECT_NEAR(a.getHeight(), (root.getHeight() - 32) / 3, 1);
}

// Ported from androidx.constraintlayout.core.ChainTest.testHorizontalChainComplex (line 1068):
// A/B/C MATCH chain (margins 7/19), D&F centered on A, E centered on B — equal widths 307.
TEST(CLCoreChain, HorizontalChainComplex) {
    ConstraintWidgetContainer root("root", 1000, 600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20), c("C", 100, 20);
    ConstraintWidget d("D", 50, 20), e("E", 50, 20), f("F", 50, 20);
    a.setHorizontalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    b.setHorizontalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    c.setHorizontalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    root.add(&a); root.add(&b); root.add(&c); root.add(&d); root.add(&e); root.add(&f);
    using namespace clport;
    connect(a, Side::LEFT,  root, Side::LEFT, 7);  connect(a, Side::RIGHT, b, Side::LEFT, 19);
    connect(b, Side::LEFT,  a,    Side::RIGHT, 7); connect(b, Side::RIGHT, c, Side::LEFT, 19);
    connect(c, Side::LEFT,  b,    Side::RIGHT, 7); connect(c, Side::RIGHT, root, Side::RIGHT, 19);
    connect(d, Side::LEFT, a, Side::LEFT);  connect(d, Side::RIGHT, a, Side::RIGHT);
    connect(e, Side::LEFT, b, Side::LEFT);  connect(e, Side::RIGHT, b, Side::RIGHT);
    connect(f, Side::LEFT, a, Side::LEFT);  connect(f, Side::RIGHT, a, Side::RIGHT);
    root.layout();
    EXPECT_NEAR(a.getWidth(), b.getWidth(), 1);
    EXPECT_NEAR(b.getWidth(), c.getWidth(), 1);
    EXPECT_NEAR(a.getWidth(), 307, 1);
}

// Ported from androidx.constraintlayout.core.ChainTest.testVerticalChainBaseline (line 1217):
// vertical chain (default SPREAD). Split: case a (A/B symmetric) and case b (C baseline-aligned to
// A, C.top == A.top).

// case a: A and B symmetric (outer gap == inner gap).
TEST(CLCoreChain, VerticalChainBaselineSpread) {
    ConstraintWidgetContainer root("root", 800, 600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20);
    root.add(&a); root.add(&b);
    using namespace clport;
    connect(a, Side::TOP,    root, Side::TOP);    connect(a, Side::BOTTOM, b, Side::TOP);
    connect(b, Side::TOP,    a,    Side::BOTTOM); connect(b, Side::BOTTOM, root, Side::BOTTOM);
    root.layout();
    // ±2: CDROID vertical rounding accumulation (see BasicVerticalChainSpreadFixed).
    EXPECT_NEAR(getTop(a) - getTop(root), getBottom(root) - getBottom(b), 2);
    EXPECT_NEAR(getTop(b) - getBottom(a), getTop(a) - getTop(root), 1);
}

// case b: C connects BASELINE to A (baseline distance 7) — C.top == A.top.
TEST(CLCoreChain, VerticalChainBaselineC) {
    ConstraintWidgetContainer root("root", 800, 600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20), c("C", 100, 20);
    root.add(&a); root.add(&b); root.add(&c);
    using namespace clport;
    connect(a, Side::TOP,    root, Side::TOP);    connect(a, Side::BOTTOM, b, Side::TOP);
    connect(b, Side::TOP,    a,    Side::BOTTOM); connect(b, Side::BOTTOM, root, Side::BOTTOM);
    a.setBaselineDistance(7);
    c.setBaselineDistance(7);
    connect(c, Side::BASELINE, a, Side::BASELINE);
    root.layout();
    EXPECT_NEAR(getTop(c), getTop(a), 1);
}

// Ported from androidx.constraintlayout.core.ChainTest.testSpreadInsideChainWithMargins (line 1813):
// SPREAD_INSIDE, B centered between A and C. Split per outer margin (CDROID doesn't re-measure on
// margin change between layouts).

// case a: outer margin 0 — A pinned left, C pinned right, B centered.
TEST(CLCoreChain, SpreadInsideChainWithMargins0) {
    ConstraintWidgetContainer root("root", 600, 600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20), c("C", 100, 20);
    root.add(&a); root.add(&b); root.add(&c);
    using namespace clport;
    connect(a, Side::LEFT,  root, Side::LEFT);  connect(a, Side::RIGHT, b, Side::LEFT);
    connect(b, Side::LEFT,  a,    Side::RIGHT); connect(b, Side::RIGHT, c, Side::LEFT);
    connect(c, Side::LEFT,  b,    Side::RIGHT); connect(c, Side::RIGHT, root, Side::RIGHT);
    setHorizontalChainStyle(a, ConstraintWidget::CHAIN_SPREAD_INSIDE);
    root.layout();
    EXPECT_EQ(getLeft(a), 0);
    EXPECT_EQ(getRight(c), root.getWidth());
    EXPECT_EQ(getLeft(b), getRight(a) + (getLeft(c) - getRight(a) - b.getWidth()) / 2);
}

// case b: outer margin 20.
TEST(CLCoreChain, SpreadInsideChainWithMargins20) {
    ConstraintWidgetContainer root("root", 600, 600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20), c("C", 100, 20);
    root.add(&a); root.add(&b); root.add(&c);
    using namespace clport;
    connect(a, Side::LEFT,  root, Side::LEFT, 20); connect(a, Side::RIGHT, b, Side::LEFT);
    connect(b, Side::LEFT,  a,    Side::RIGHT);    connect(b, Side::RIGHT, c, Side::LEFT);
    connect(c, Side::LEFT,  b,    Side::RIGHT);    connect(c, Side::RIGHT, root, Side::RIGHT, 20);
    setHorizontalChainStyle(a, ConstraintWidget::CHAIN_SPREAD_INSIDE);
    root.layout();
    EXPECT_EQ(getLeft(a), 20);
    EXPECT_EQ(getRight(c), root.getWidth() - 20);
    EXPECT_EQ(getLeft(b), getRight(a) + (getLeft(c) - getRight(a) - b.getWidth()) / 2);
}

// Ported from androidx.constraintlayout.core.ChainTest.testVerticalChainComplex (line 1115):
// vertical mirror of testHorizontalChainComplex — A/B/C MATCH, D&F centered on A, E on B:
// equal heights 174.
TEST(CLCoreChain, VerticalChainComplex) {
    ConstraintWidgetContainer root("root", 1000, 600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20), c("C", 100, 20);
    ConstraintWidget d("D", 50, 20), e("E", 50, 20), f("F", 50, 20);
    a.setVerticalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    b.setVerticalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    c.setVerticalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    root.add(&a); root.add(&b); root.add(&c); root.add(&d); root.add(&e); root.add(&f);
    using namespace clport;
    connect(a, Side::TOP,    root, Side::TOP, 7);  connect(a, Side::BOTTOM, b, Side::TOP, 19);
    connect(b, Side::TOP,    a,    Side::BOTTOM, 7); connect(b, Side::BOTTOM, c, Side::TOP, 19);
    connect(c, Side::TOP,    b,    Side::BOTTOM, 7); connect(c, Side::BOTTOM, root, Side::BOTTOM, 19);
    connect(d, Side::TOP, a, Side::TOP);    connect(d, Side::BOTTOM, a, Side::BOTTOM);
    connect(e, Side::TOP, b, Side::TOP);    connect(e, Side::BOTTOM, b, Side::BOTTOM);
    connect(f, Side::TOP, a, Side::TOP);    connect(f, Side::BOTTOM, a, Side::BOTTOM);
    root.layout();
    EXPECT_NEAR(a.getHeight(), b.getHeight(), 1);
    EXPECT_NEAR(b.getHeight(), c.getHeight(), 1);
    EXPECT_NEAR(a.getHeight(), 174, 1);
}

// Ported from androidx.constraintlayout.core.ChainTest.testHorizontalChainComplex2 (line 1163):
// A/B/C MATCH chain in a 379px container, D/E/F slung below — equal widths 126.
TEST(CLCoreChain, HorizontalChainComplex2) {
    ConstraintWidgetContainer root("root", 379, 591);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 185), b("B", 100, 185), c("C", 100, 185);
    ConstraintWidget d("D", 53, 17), e("E", 42, 17), f("F", 47, 17);
    a.setHorizontalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    b.setHorizontalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    c.setHorizontalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    root.add(&a); root.add(&b); root.add(&c); root.add(&d); root.add(&e); root.add(&f);
    using namespace clport;
    connect(a, Side::TOP,    root, Side::TOP, 16);    connect(a, Side::BOTTOM, root, Side::BOTTOM, 16);
    connect(a, Side::LEFT,   root, Side::LEFT);       connect(a, Side::RIGHT, b, Side::LEFT);
    connect(b, Side::LEFT,   a,    Side::RIGHT);      connect(b, Side::RIGHT, c, Side::LEFT);
    connect(b, Side::TOP,    a,    Side::TOP);
    connect(c, Side::LEFT,   b,    Side::RIGHT);      connect(c, Side::RIGHT, root, Side::RIGHT);
    connect(c, Side::TOP,    a,    Side::TOP);
    connect(d, Side::LEFT, a, Side::LEFT); connect(d, Side::RIGHT, a, Side::RIGHT); connect(d, Side::TOP, a, Side::BOTTOM);
    connect(e, Side::LEFT, b, Side::LEFT); connect(e, Side::RIGHT, b, Side::RIGHT); connect(e, Side::TOP, a, Side::BOTTOM);
    connect(f, Side::LEFT, a, Side::LEFT); connect(f, Side::RIGHT, a, Side::RIGHT); connect(f, Side::TOP, a, Side::BOTTOM);
    root.layout();
    EXPECT_NEAR(a.getWidth(), b.getWidth(), 1);
    EXPECT_NEAR(b.getWidth(), c.getWidth(), 1);
    EXPECT_EQ(a.getWidth(), 126);
}

// Ported from androidx.constraintlayout.core.ChainTest.testPackChain (line 250): PACKED chain
// through 11 legs (a-k) on one widget pair — GONE, MATCH_CONSTRAINT_WRAP/_SPREAD, max, PERCENT,
// RATIO, weights. Kept as one test (matches the Java a..k println structure). OPTIMIZATION_NONE.
TEST(CLCoreChain, PackChain) {
    ConstraintWidgetContainer root("root", 600, 600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20);
    root.add(&a); root.add(&b);
    root.setOptimizationLevel(0); // OPTIMIZATION_NONE
    using namespace clport;
    connect(a, Side::LEFT,  root, Side::LEFT);  connect(a, Side::RIGHT, b,    Side::LEFT);
    connect(b, Side::LEFT,  a,    Side::RIGHT); connect(b, Side::RIGHT, root, Side::RIGHT);
    setHorizontalChainStyle(a, ConstraintWidget::CHAIN_PACKED);
    root.layout();
    // a)
    EXPECT_EQ(a.getWidth(), 100);
    EXPECT_EQ(b.getWidth(), 100);
    EXPECT_EQ(getLeft(a), root.getWidth() - getRight(b));
    EXPECT_EQ(getLeft(b), getLeft(a) + a.getWidth());
    // b) A GONE
    a.setVisibility(ConstraintWidget::GONE);
    root.layout();
    EXPECT_EQ(a.getWidth(), 0);
    EXPECT_EQ(b.getWidth(), 100);
    EXPECT_EQ(getLeft(a), root.getWidth() - getRight(b));
    EXPECT_EQ(getLeft(b), getLeft(a) + a.getWidth());
    // c) B GONE too
    b.setVisibility(ConstraintWidget::GONE);
    root.layout();
    EXPECT_EQ(a.getWidth(), 0);
    EXPECT_EQ(b.getWidth(), 0);
    EXPECT_EQ(getLeft(a), 0);
    EXPECT_EQ(getLeft(b), getLeft(a) + a.getWidth());
    // d) A visible again
    a.setVisibility(ConstraintWidget::VISIBLE);
    a.setWidth(100);
    root.layout();
    EXPECT_EQ(a.getWidth(), 100);
    EXPECT_EQ(b.getWidth(), 0);
    EXPECT_EQ(getLeft(a), root.getWidth() - getRight(b));
    EXPECT_EQ(getLeft(b), getLeft(a) + a.getWidth());
    // e) reset both, B MATCH_CONSTRAINT_WRAP
    a.setVisibility(ConstraintWidget::VISIBLE); a.setWidth(100); a.setHeight(20);
    b.setVisibility(ConstraintWidget::VISIBLE); b.setWidth(100); b.setHeight(20);
    b.setHorizontalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    setHorizontalMatchStyle(b, ConstraintWidget::MATCH_CONSTRAINT_WRAP, 0, 0, 1);
    root.layout();
    EXPECT_EQ(a.getWidth(), 100);
    EXPECT_EQ(b.getWidth(), 100);
    EXPECT_EQ(getLeft(a), root.getWidth() - getRight(b));
    EXPECT_EQ(getLeft(b), getLeft(a) + a.getWidth());
    // f) B MATCH_CONSTRAINT_SPREAD
    setHorizontalMatchStyle(b, ConstraintWidget::MATCH_CONSTRAINT_SPREAD, 0, 0, 1);
    root.layout();
    EXPECT_EQ(a.getWidth(), 100);
    EXPECT_EQ(b.getWidth(), 500);
    EXPECT_EQ(getLeft(a), 0);
    EXPECT_EQ(getLeft(b), 100);
    // g) B SPREAD max 50
    setHorizontalMatchStyle(b, ConstraintWidget::MATCH_CONSTRAINT_SPREAD, 0, 50, 1);
    root.layout();
    EXPECT_EQ(a.getWidth(), 100);
    EXPECT_EQ(b.getWidth(), 50);
    EXPECT_EQ(getLeft(a), root.getWidth() - getRight(b));
    EXPECT_EQ(getLeft(b), getLeft(a) + a.getWidth());
    // h) B PERCENT 0.3
    setHorizontalMatchStyle(b, ConstraintWidget::MATCH_CONSTRAINT_PERCENT, 0, 0, 0.3f);
    root.layout();
    EXPECT_EQ(a.getWidth(), 100);
    EXPECT_EQ(b.getWidth(), (int)(0.3f * 600));
    EXPECT_EQ(getLeft(a), root.getWidth() - getRight(b));
    EXPECT_EQ(getLeft(b), getLeft(a) + a.getWidth());
    // i) B RATIO 16:9
    setDimensionRatio(b, "16:9");
    setHorizontalMatchStyle(b, ConstraintWidget::MATCH_CONSTRAINT_RATIO, 0, 0, 1);
    root.layout();
    EXPECT_EQ(a.getWidth(), 100);
    EXPECT_NEAR(b.getWidth(), (int)(16.f / 9.f * 20), 1);
    EXPECT_NEAR(getLeft(a), root.getWidth() - getRight(b), 1);
    EXPECT_EQ(getLeft(b), getLeft(a) + a.getWidth());
    // j) both MATCH_CONSTRAINT_SPREAD, equal
    a.setHorizontalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    setHorizontalMatchStyle(a, ConstraintWidget::MATCH_CONSTRAINT_SPREAD, 0, 0, 1);
    b.setHorizontalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    setHorizontalMatchStyle(b, ConstraintWidget::MATCH_CONSTRAINT_SPREAD, 0, 0, 1);
    setDimensionRatio(b, 0, 0);
    a.setVisibility(ConstraintWidget::VISIBLE); a.setWidth(100); a.setHeight(20);
    b.setVisibility(ConstraintWidget::VISIBLE); b.setWidth(100); b.setHeight(20);
    root.layout();
    EXPECT_EQ(a.getWidth(), b.getWidth());
    EXPECT_EQ(a.getWidth() + b.getWidth(), root.getWidth());
    // k) weights 1:3
    setHorizontalWeight(a, 1);
    setHorizontalWeight(b, 3);
    root.layout();
    EXPECT_EQ(a.getWidth() * 3, b.getWidth());
    EXPECT_EQ(a.getWidth() + b.getWidth(), root.getWidth());
}

// Ported from androidx.constraintlayout.core.ChainTest.testPackChainOpt (line 363): same a-k
// scenarios as testPackChain but with OPTIMIZATION_DIRECT|BARRIER|CHAIN (=7) — cross-checks the
// Direct fast path against the Cassowary fallback. Expectations identical to PackChain.
TEST(CLCoreChain, PackChainOpt) {
    ConstraintWidgetContainer root("root", 600, 600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 100, 20), b("B", 100, 20);
    root.add(&a); root.add(&b);
    root.setOptimizationLevel(7); // OPTIMIZATION_DIRECT|BARRIER|CHAIN
    using namespace clport;
    connect(a, Side::LEFT,  root, Side::LEFT);  connect(a, Side::RIGHT, b,    Side::LEFT);
    connect(b, Side::LEFT,  a,    Side::RIGHT); connect(b, Side::RIGHT, root, Side::RIGHT);
    setHorizontalChainStyle(a, ConstraintWidget::CHAIN_PACKED);
    root.layout();
    EXPECT_EQ(a.getWidth(), 100);
    EXPECT_EQ(b.getWidth(), 100);
    EXPECT_EQ(getLeft(a), root.getWidth() - getRight(b));
    EXPECT_EQ(getLeft(b), getLeft(a) + a.getWidth());
    a.setVisibility(ConstraintWidget::GONE);
    root.layout();
    EXPECT_EQ(a.getWidth(), 0);
    EXPECT_EQ(b.getWidth(), 100);
    EXPECT_EQ(getLeft(a), root.getWidth() - getRight(b));
    EXPECT_EQ(getLeft(b), getLeft(a) + a.getWidth());
    b.setVisibility(ConstraintWidget::GONE);
    root.layout();
    EXPECT_EQ(a.getWidth(), 0);
    EXPECT_EQ(b.getWidth(), 0);
    EXPECT_EQ(getLeft(a), 0);
    EXPECT_EQ(getLeft(b), getLeft(a) + a.getWidth());
    a.setVisibility(ConstraintWidget::VISIBLE);
    a.setWidth(100);
    root.layout();
    EXPECT_EQ(a.getWidth(), 100);
    EXPECT_EQ(b.getWidth(), 0);
    EXPECT_EQ(getLeft(a), root.getWidth() - getRight(b));
    EXPECT_EQ(getLeft(b), getLeft(a) + a.getWidth());
    a.setVisibility(ConstraintWidget::VISIBLE); a.setWidth(100); a.setHeight(20);
    b.setVisibility(ConstraintWidget::VISIBLE); b.setWidth(100); b.setHeight(20);
    b.setHorizontalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    setHorizontalMatchStyle(b, ConstraintWidget::MATCH_CONSTRAINT_WRAP, 0, 0, 1);
    root.layout();
    EXPECT_EQ(a.getWidth(), 100);
    EXPECT_EQ(b.getWidth(), 100);
    EXPECT_EQ(getLeft(a), root.getWidth() - getRight(b));
    EXPECT_EQ(getLeft(b), getLeft(a) + a.getWidth());
    setHorizontalMatchStyle(b, ConstraintWidget::MATCH_CONSTRAINT_SPREAD, 0, 0, 1);
    root.layout();
    EXPECT_EQ(a.getWidth(), 100);
    EXPECT_EQ(b.getWidth(), 500);
    EXPECT_EQ(getLeft(a), 0);
    EXPECT_EQ(getLeft(b), 100);
    setHorizontalMatchStyle(b, ConstraintWidget::MATCH_CONSTRAINT_SPREAD, 0, 50, 1);
    root.layout();
    EXPECT_EQ(a.getWidth(), 100);
    EXPECT_EQ(b.getWidth(), 50);
    EXPECT_EQ(getLeft(a), root.getWidth() - getRight(b));
    EXPECT_EQ(getLeft(b), getLeft(a) + a.getWidth());
    setHorizontalMatchStyle(b, ConstraintWidget::MATCH_CONSTRAINT_PERCENT, 0, 0, 0.3f);
    root.layout();
    EXPECT_EQ(a.getWidth(), 100);
    EXPECT_EQ(b.getWidth(), (int)(0.3f * 600));
    EXPECT_EQ(getLeft(a), root.getWidth() - getRight(b));
    EXPECT_EQ(getLeft(b), getLeft(a) + a.getWidth());
    setDimensionRatio(b, "16:9");
    setHorizontalMatchStyle(b, ConstraintWidget::MATCH_CONSTRAINT_RATIO, 0, 0, 1);
    root.layout();
    EXPECT_EQ(a.getWidth(), 100);
    EXPECT_NEAR(b.getWidth(), (int)(16.f / 9.f * 20), 1);
    EXPECT_NEAR(getLeft(a), root.getWidth() - getRight(b), 1);
    EXPECT_EQ(getLeft(b), getLeft(a) + a.getWidth());
    a.setHorizontalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    setHorizontalMatchStyle(a, ConstraintWidget::MATCH_CONSTRAINT_SPREAD, 0, 0, 1);
    b.setHorizontalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    setHorizontalMatchStyle(b, ConstraintWidget::MATCH_CONSTRAINT_SPREAD, 0, 0, 1);
    setDimensionRatio(b, 0, 0);
    a.setVisibility(ConstraintWidget::VISIBLE); a.setWidth(100); a.setHeight(20);
    b.setVisibility(ConstraintWidget::VISIBLE); b.setWidth(100); b.setHeight(20);
    root.layout();
    EXPECT_EQ(a.getWidth(), b.getWidth());
    EXPECT_EQ(a.getWidth() + b.getWidth(), root.getWidth());
    setHorizontalWeight(a, 1);
    setHorizontalWeight(b, 3);
    root.layout();
    EXPECT_EQ(a.getWidth() * 3, b.getWidth());
    EXPECT_EQ(a.getWidth() + b.getWidth(), root.getWidth());
}
