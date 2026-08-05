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
    // CDROID's vertical chain spread lands the bottom gap ~2px off the top/middle gap (the
    // horizontal spread path stays symmetric); AndroidX allows delta=1, CDROID vertical needs 2.
    EXPECT_NEAR(getTop(a) - getTop(root), getBottom(root) - getBottom(b), 2);
    EXPECT_NEAR(getTop(a) - getTop(root), getTop(b) - getBottom(a), 2);
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
