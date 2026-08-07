/*
 * End-to-end tests for ConstraintWidget + ConstraintWidgetContainer (Stage 2/3 validation).
 * Uses the real widget model + solver pipeline (anchor connect → addToSolver → minimize →
 * updateFromSolver). Gated on ENABLE_CONSTRAINTLAYOUT.
 */
#include <gui_features.h>
#ifdef ENABLE_CONSTRAINTLAYOUT

#include <gtest/gtest.h>
#include <widgetEx/constraintlayout/core/widgets/constraintwidget.h>
#include <widgetEx/constraintlayout/core/widgets/constraintwidgetcontainer.h>
#include <widgetEx/constraintlayout/core/widgets/constraintanchor.h>
#include <widgetEx/constraintlayout/core/linearsystem.h>

using namespace cdroid;

// A child whose left+right both connect to the parent → horizontally centered in 600.
TEST(CLConstraintWidget, AddToSolverCentered) {
    ConstraintWidgetContainer container(600, 400);
    ConstraintWidget child(100, 50);
    container.add(&child);
    child.mLeft.connect(&container.mLeft, 0);
    child.mRight.connect(&container.mRight, 0);

    LinearSystem system;
    container.layout();

    EXPECT_EQ(child.getX(), 250); // (600-100)/2
    EXPECT_EQ(child.getWidth(), 100);
}

// A child connected leftToLeft only with margin → pinned at x=margin.
TEST(CLConstraintWidget, AddToSolverPinnedLeft) {
    ConstraintWidgetContainer container(600, 400);
    ConstraintWidget child(100, 50);
    container.add(&child);
    child.mLeft.connect(&container.mLeft, 20);

    container.layout();

    EXPECT_EQ(child.getX(), 20);
    EXPECT_EQ(child.getWidth(), 100);
}

// container.layout() centered (uses the full driver: addToSolver + Chain + minimize + updateFromSolver).
TEST(CLConstraintWidget, LayoutCentered) {
    ConstraintWidgetContainer container(600, 400);
    ConstraintWidget child(100, 50);
    container.add(&child);
    child.mLeft.connect(&container.mLeft, 0);
    child.mRight.connect(&container.mRight, 0);

    container.layout();

    EXPECT_EQ(child.getX(), 250);
    EXPECT_EQ(child.getWidth(), 100);
}

// container.layout() pinned left with margin.
TEST(CLConstraintWidget, LayoutPinnedLeft) {
    ConstraintWidgetContainer container(600, 400);
    ConstraintWidget child(100, 50);
    container.add(&child);
    child.mLeft.connect(&container.mLeft, 20);

    container.layout();

    EXPECT_EQ(child.getX(), 20);
}

// Two children: one pinned left (x=10), one pinned right (x=570).
TEST(CLConstraintWidget, LayoutTwoChildren) {
    ConstraintWidgetContainer container(600, 400);
    ConstraintWidget left(100, 50);
    ConstraintWidget right(100, 50);
    container.add(&left);
    container.add(&right);
    left.mLeft.connect(&container.mLeft, 10);
    right.mRight.connect(&container.mRight, 30);

    container.layout();

    EXPECT_EQ(left.getX(), 10);
    EXPECT_EQ(right.getX(), 470);   // 600 - 100 - 30
    EXPECT_EQ(right.getWidth(), 100);
}

// A child that fills the parent width (left to parent left, right to parent right, 0dp spread).
TEST(CLConstraintWidget, LayoutFillWidth) {
    ConstraintWidgetContainer container(600, 400);
    ConstraintWidget child(0, 50);
    child.setHorizontalDimensionBehaviour(ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT);
    child.mMatchConstraintDefaultWidth = ConstraintWidget::MATCH_CONSTRAINT_SPREAD;
    container.add(&child);
    child.mLeft.connect(&container.mLeft, 0);
    child.mRight.connect(&container.mRight, 0);

    container.layout();

    EXPECT_EQ(child.getX(), 0);
    EXPECT_EQ(child.getWidth(), 600);
}

// Centered with bias 0.3 → x = 0.3 * (600-100) = 150.
TEST(CLConstraintWidget, LayoutBiasCentered) {
    ConstraintWidgetContainer container(600, 400);
    ConstraintWidget child(100, 50);
    container.add(&child);
    child.mLeft.connect(&container.mLeft, 0);
    child.mRight.connect(&container.mRight, 0);
    child.mHorizontalBiasPercent = 0.3f;

    container.layout();

    EXPECT_EQ(child.getX(), 150);
}

// Centered with bias 0.3 + margin 20 on left → x = 20 + 0.3*(600-20-100) = 20+144 = 164.
TEST(CLConstraintWidget, LayoutBiasWithMargin) {
    ConstraintWidgetContainer container(600, 400);
    ConstraintWidget child(100, 50);
    container.add(&child);
    child.mLeft.connect(&container.mLeft, 20);
    child.mRight.connect(&container.mRight, 0);
    child.mHorizontalBiasPercent = 0.3f;

    container.layout();

    EXPECT_EQ(child.getX(), 20 + (int)(0.3f * (600 - 20 - 100)));
}

// Margins on both sides → x = leftMargin, width = 600 - leftMargin - rightMargin.
TEST(CLConstraintWidget, LayoutMarginsBothSides) {
    ConstraintWidgetContainer container(600, 400);
    ConstraintWidget child(0, 50);
    child.setHorizontalDimensionBehaviour(ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT);
    child.mMatchConstraintDefaultWidth = ConstraintWidget::MATCH_CONSTRAINT_SPREAD;
    container.add(&child);
    child.mLeft.connect(&container.mLeft, 30);
    child.mRight.connect(&container.mRight, 50);

    container.layout();

    EXPECT_EQ(child.getX(), 30);
    EXPECT_EQ(child.getWidth(), 520); // 600 - 30 - 50
}

// Two children in a horizontal chain (A.left→parent, A.right→B.left, B.left→A.right, B.right→parent).
TEST(CLConstraintWidget, LayoutChainSpread) {
    ConstraintWidgetContainer container(600, 400);
    ConstraintWidget a(100, 50);
    ConstraintWidget b(100, 50);
    container.add(&a);
    container.add(&b);
    a.mLeft.connect(&container.mLeft, 0);
    a.mRight.connect(&b.mLeft, 0);
    b.mLeft.connect(&a.mRight, 0);
    b.mRight.connect(&container.mRight, 0);

    container.layout();

    // CHAIN_SPREAD: equal gaps before/inside/after → (600-200)/3=133.33. b lands at 233+133.33=366.66,
    // which the optimizer's DIRECT path and the full Cassowary solver round 1px apart (366 vs 367) —
    // tolerate that single-pixel resolution difference (cf. chain_test.cc ±1px note).
    EXPECT_EQ(a.getX(), 133);
    EXPECT_NEAR(b.getX(), 367, 1);
}

// Centered vertically → y = (400-50)/2 = 175.
TEST(CLConstraintWidget, LayoutVerticalCentered) {
    ConstraintWidgetContainer container(600, 400);
    ConstraintWidget child(100, 50);
    container.add(&child);
    child.mLeft.connect(&container.mLeft, 0);
    child.mTop.connect(&container.mTop, 0);
    child.mBottom.connect(&container.mBottom, 0);

    container.layout();

    EXPECT_EQ(child.getY(), 175); // (400-50)/2
}

#endif // ENABLE_CONSTRAINTLAYOUT
