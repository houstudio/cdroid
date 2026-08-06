// Regression tests for ConstraintLayout widget chains (androidx.constraintlayout.core Chain).
// A chain is a sequence of widgets linked side-to-side (A.right -> B.left, B.left -> A.right, ...),
// head anchored to the container on the leading side and tail on the trailing side, with every
// member at MATCH_CONSTRAINT on the chain axis. SPREAD (the default) distributes the free space
// so the members are equal-width (when no explicit weights are set) and contiguous.
//
// These exercise the solver path directly (ConstraintWidgetContainer::layout, which the View layer
// also drives via BasicMeasure::solverMeasure -> solveLinearSystem -> layout), independent of the
// View/inflation/resource layer.
#include <gtest/gtest.h>
#include <widgetEx/constraintlayout/core/widgets/constraintwidget.h>
#include <widgetEx/constraintlayout/core/widgets/constraintwidgetcontainer.h>

using namespace cdroid;

namespace {
void setMatchChain(ConstraintWidget* w, int fixedOtherDim, bool horizontal) {
    if (horizontal) {
        w->setHorizontalDimensionBehaviour(ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT);
        w->setWidth(0);
        w->setVerticalDimensionBehaviour(ConstraintWidget::DimensionBehaviour::FIXED);
        w->setHeight(fixedOtherDim);
    } else {
        w->setVerticalDimensionBehaviour(ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT);
        w->setHeight(0);
        w->setHorizontalDimensionBehaviour(ConstraintWidget::DimensionBehaviour::FIXED);
        w->setWidth(fixedOtherDim);
    }
}
} // namespace

// Three 0dp-width widgets in a horizontal SPREAD chain across a 1000px container: each ~333,
// contiguous, summing to 1000.
TEST(CLChain, HorizontalSpreadDistributesEvenly) {
    ConstraintWidgetContainer root("root", 1000, 1000);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, ConstraintWidget::DimensionBehaviour::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, ConstraintWidget::DimensionBehaviour::FIXED);

    ConstraintWidget a("a"), b("b"), c("c");
    for (auto* w : {&a, &b, &c}) setMatchChain(w, 40, /*horizontal=*/true);

    a.mLeft.connect(root.mLeft, 0);   a.mRight.connect(b.mLeft, 0);
    b.mLeft.connect(a.mRight, 0);     b.mRight.connect(c.mLeft, 0);
    c.mLeft.connect(b.mRight, 0);     c.mRight.connect(root.mRight, 0);
    a.mTop.connect(root.mTop, 0);     a.mBottom.connect(root.mTop, 40);

    root.add(&a); root.add(&b); root.add(&c);
    root.layout();

    EXPECT_GT(a.getWidth(), 0);
    EXPECT_GT(b.getWidth(), 0);
    EXPECT_GT(c.getWidth(), 0);
    EXPECT_NEAR(a.getWidth() + b.getWidth() + c.getWidth(), 1000, 4);
    // Contiguous: b starts where a ends, c where b ends.
    EXPECT_EQ(b.getX(), a.getX() + a.getWidth());
    EXPECT_EQ(c.getX(), b.getX() + b.getWidth());
}

// Vertical chain of three 0dp-height widgets across a 900px-tall container: equal heights,
// contiguous.
TEST(CLChain, VerticalSpreadDistributesEvenly) {
    ConstraintWidgetContainer root("root", 200, 900);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, ConstraintWidget::DimensionBehaviour::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, ConstraintWidget::DimensionBehaviour::FIXED);

    ConstraintWidget a("a"), b("b"), c("c");
    for (auto* w : {&a, &b, &c}) setMatchChain(w, 40, /*horizontal=*/false);

    a.mTop.connect(root.mTop, 0);     a.mBottom.connect(b.mTop, 0);
    b.mTop.connect(a.mBottom, 0);     b.mBottom.connect(c.mTop, 0);
    c.mTop.connect(b.mBottom, 0);     c.mBottom.connect(root.mBottom, 0);
    a.mLeft.connect(root.mLeft, 0);   a.mRight.connect(root.mLeft, 40);

    root.add(&a); root.add(&b); root.add(&c);
    root.layout();

    EXPECT_GT(a.getHeight(), 0);
    EXPECT_GT(b.getHeight(), 0);
    EXPECT_GT(c.getHeight(), 0);
    EXPECT_NEAR(a.getHeight() + b.getHeight() + c.getHeight(), 900, 4);
    EXPECT_EQ(b.getY(), a.getY() + a.getHeight());
    EXPECT_EQ(c.getY(), b.getY() + b.getHeight());
}

#include <cstdio>

// Flow WRAP_NONE infeasible case: six 80px FIXED widgets in one horizontal SPREAD chain anchored
// across a 320px container (6*80 = 480 > 320). CDROID omits Direct.solveChain
// (Chain::USE_CHAIN_OPTIMIZATION is false), so this prints where the pure-Cassowary path lands
// each widget — to compare against Android's Direct.solveChain and the observed flowtest rendering
// at 320x640 (n1/n6 clipped, middle four fill the row).
TEST(CLChain, HorizontalSpreadFixedOverflow) {
    ConstraintWidgetContainer root("root", 320, 200);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, ConstraintWidget::DimensionBehaviour::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, ConstraintWidget::DimensionBehaviour::FIXED);

    ConstraintWidget a("a"), b("b"), c("c"), d("d"), e("e"), f("f");
    ConstraintWidget* ws[] = {&a, &b, &c, &d, &e, &f};
    for (auto* w : ws) {
        w->setHorizontalDimensionBehaviour(ConstraintWidget::DimensionBehaviour::FIXED);
        w->setWidth(80);
        w->setVerticalDimensionBehaviour(ConstraintWidget::DimensionBehaviour::FIXED);
        w->setHeight(40);
    }
    a.mLeft.connect(root.mLeft, 0);   a.mRight.connect(b.mLeft, 0);
    b.mLeft.connect(a.mRight, 0);     b.mRight.connect(c.mLeft, 0);
    c.mLeft.connect(b.mRight, 0);     c.mRight.connect(d.mLeft, 0);
    d.mLeft.connect(c.mRight, 0);     d.mRight.connect(e.mLeft, 0);
    e.mLeft.connect(d.mRight, 0);     e.mRight.connect(f.mLeft, 0);
    f.mLeft.connect(e.mRight, 0);     f.mRight.connect(root.mRight, 0);
    a.mTop.connect(root.mTop, 0);     a.mBottom.connect(root.mTop, 40);
    a.mHorizontalChainStyle = ConstraintWidget::CHAIN_SPREAD;

    for (auto* w : ws) root.add(w);
    root.layout();

    // Overflow (480 > 320): Direct returns false (distance<totalSize, Direct:984), so Cassowary
    // solves by pushing the excess to both ends — n1/n6 land outside [0,320], middle four fill it.
    EXPECT_LT(a.getX(), 0);
    EXPECT_EQ(b.getX(), 0);
    EXPECT_EQ(e.getX() + e.getWidth(), 320);
    EXPECT_GT(f.getX() + f.getWidth(), 320);
    for (auto* w : ws) EXPECT_EQ(w->getWidth(), 80);
}

// Direct fast-path probe: three 100px FIXED widgets in a 1000px SPREAD chain. If Direct.solveChain
// takes over, gap = (1000-300)/(3+1) = 175 -> a@175, b@450, c@725. Printed to compare against the
// Cassowary fallback (which would be ~the same, ±1px).
TEST(CLChain, HorizontalSpreadFixedFits) {
    ConstraintWidgetContainer root("root", 1000, 200);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, ConstraintWidget::DimensionBehaviour::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, ConstraintWidget::DimensionBehaviour::FIXED);

    ConstraintWidget a("a"), b("b"), c("c");
    ConstraintWidget* ws[] = {&a, &b, &c};
    for (auto* w : ws) {
        w->setHorizontalDimensionBehaviour(ConstraintWidget::DimensionBehaviour::FIXED);
        w->setWidth(100);
        w->setVerticalDimensionBehaviour(ConstraintWidget::DimensionBehaviour::FIXED);
        w->setHeight(40);
    }
    a.mLeft.connect(root.mLeft, 0);   a.mRight.connect(b.mLeft, 0);
    b.mLeft.connect(a.mRight, 0);     b.mRight.connect(c.mLeft, 0);
    c.mLeft.connect(b.mRight, 0);     c.mRight.connect(root.mRight, 0);
    a.mTop.connect(root.mTop, 0);     a.mBottom.connect(root.mTop, 40);
    a.mHorizontalChainStyle = ConstraintWidget::CHAIN_SPREAD;

    for (auto* w : ws) root.add(w);
    root.layout();

    // Direct.solveChain SPREAD: gap = (1000-300)/(3+1) = 175 -> a@175, b@450, c@725.
    EXPECT_EQ(a.getX(), 175);
    EXPECT_EQ(b.getX(), 450);
    EXPECT_EQ(c.getX(), 725);
    for (auto* w : ws) EXPECT_EQ(w->getWidth(), 100);
}

// Direct vertical SPREAD: mirrors horizontal across a 1000px-tall container.
TEST(CLChain, VerticalSpreadFixedFits) {
    ConstraintWidgetContainer root("root", 200, 1000);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, ConstraintWidget::DimensionBehaviour::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, ConstraintWidget::DimensionBehaviour::FIXED);
    ConstraintWidget a("a"), b("b"), c("c");
    ConstraintWidget* ws[] = {&a, &b, &c};
    for (auto* w : ws) {
        w->setHorizontalDimensionBehaviour(ConstraintWidget::DimensionBehaviour::FIXED);
        w->setWidth(40);
        w->setVerticalDimensionBehaviour(ConstraintWidget::DimensionBehaviour::FIXED);
        w->setHeight(100);
    }
    a.mTop.connect(root.mTop, 0);     a.mBottom.connect(b.mTop, 0);
    b.mTop.connect(a.mBottom, 0);     b.mBottom.connect(c.mTop, 0);
    c.mTop.connect(b.mBottom, 0);     c.mBottom.connect(root.mBottom, 0);
    a.mLeft.connect(root.mLeft, 0);   a.mRight.connect(root.mLeft, 40);
    a.mVerticalChainStyle = ConstraintWidget::CHAIN_SPREAD;
    for (auto* w : ws) root.add(w);
    root.layout();
    EXPECT_EQ(a.getY(), 175);
    EXPECT_EQ(b.getY(), 450);
    EXPECT_EQ(c.getY(), 725);
}

// Direct SPREAD_INSIDE with two widgets: first pinned to start, last pinned to end.
TEST(CLChain, HorizontalSpreadInsideFixedTwoWidgets) {
    ConstraintWidgetContainer root("root", 1000, 200);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, ConstraintWidget::DimensionBehaviour::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, ConstraintWidget::DimensionBehaviour::FIXED);
    ConstraintWidget a("a"), b("b");
    ConstraintWidget* ws[] = {&a, &b};
    for (auto* w : ws) {
        w->setHorizontalDimensionBehaviour(ConstraintWidget::DimensionBehaviour::FIXED);
        w->setWidth(100);
        w->setVerticalDimensionBehaviour(ConstraintWidget::DimensionBehaviour::FIXED);
        w->setHeight(40);
    }
    a.mLeft.connect(root.mLeft, 0);   a.mRight.connect(b.mLeft, 0);
    b.mLeft.connect(a.mRight, 0);     b.mRight.connect(root.mRight, 0);
    a.mTop.connect(root.mTop, 0);     a.mBottom.connect(root.mTop, 40);
    a.mHorizontalChainStyle = ConstraintWidget::CHAIN_SPREAD_INSIDE;
    for (auto* w : ws) root.add(w);
    root.layout();
    EXPECT_EQ(a.getX(), 0);
    EXPECT_EQ(b.getX(), 900);
    EXPECT_EQ(b.getX() + b.getWidth(), 1000);
}

// Single-visible SPREAD: widget centered in the container, (1000-100)/2 = 450.
TEST(CLChain, HorizontalSingleVisibleSpread) {
    ConstraintWidgetContainer root("root", 1000, 200);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, ConstraintWidget::DimensionBehaviour::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, ConstraintWidget::DimensionBehaviour::FIXED);
    ConstraintWidget a("a");
    a.setHorizontalDimensionBehaviour(ConstraintWidget::DimensionBehaviour::FIXED);
    a.setWidth(100);
    a.setVerticalDimensionBehaviour(ConstraintWidget::DimensionBehaviour::FIXED);
    a.setHeight(40);
    a.mLeft.connect(root.mLeft, 0);
    a.mRight.connect(root.mRight, 0);
    a.mTop.connect(root.mTop, 0);
    a.mBottom.connect(root.mTop, 40);
    a.mHorizontalChainStyle = ConstraintWidget::CHAIN_SPREAD;
    root.add(&a);
    root.layout();
    EXPECT_EQ(a.getX(), 450);
    EXPECT_EQ(a.getWidth(), 100);
}

// PACKED chain: Direct returns false (isChainPacked), Cassowary solves. Packed bias 0.5 clusters
// the 300px group centered in 1000 -> starts at 350.
TEST(CLChain, HorizontalPackedFallsBack) {
    ConstraintWidgetContainer root("root", 1000, 200);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, ConstraintWidget::DimensionBehaviour::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, ConstraintWidget::DimensionBehaviour::FIXED);
    ConstraintWidget a("a"), b("b"), c("c");
    ConstraintWidget* ws[] = {&a, &b, &c};
    for (auto* w : ws) {
        w->setHorizontalDimensionBehaviour(ConstraintWidget::DimensionBehaviour::FIXED);
        w->setWidth(100);
        w->setVerticalDimensionBehaviour(ConstraintWidget::DimensionBehaviour::FIXED);
        w->setHeight(40);
    }
    a.mLeft.connect(root.mLeft, 0);   a.mRight.connect(b.mLeft, 0);
    b.mLeft.connect(a.mRight, 0);     b.mRight.connect(c.mLeft, 0);
    c.mLeft.connect(b.mRight, 0);     c.mRight.connect(root.mRight, 0);
    a.mTop.connect(root.mTop, 0);     a.mBottom.connect(root.mTop, 40);
    a.mHorizontalChainStyle = ConstraintWidget::CHAIN_PACKED;
    for (auto* w : ws) root.add(w);
    root.layout();
    for (auto* w : ws) EXPECT_EQ(w->getWidth(), 100);
    EXPECT_EQ(a.getX(), 350);
    EXPECT_EQ(b.getX(), 450);
    EXPECT_EQ(c.getX(), 550);
}
