/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * End-to-end validation of the widget model <-> solver integration: real ConstraintWidget
 * objects whose anchors become solver variables, constraints solved, positions read back via
 * updateFromSolver. Constraints are wired by hand here (the plan's "Stage 2 validation, 4-child
 * container") because ConstraintWidget::addToSolver — the auto-populator — is still stubbed
 * pending the analyzer (Stage 3). Once addToSolver lands, a parallel test will exercise it.
 *
 * Gated on ENABLE_CONSTRAINTLAYOUT (same pattern as linear_system_test.cc).
 */
#include <gui_features.h>
#ifdef ENABLE_CONSTRAINTLAYOUT

#include <gtest/gtest.h>

#include <widgetEx/constraintlayout/core/linearsystem.h>
#include <widgetEx/constraintlayout/core/solvervariable.h>
#include <widgetEx/constraintlayout/core/widgets/constraintwidget.h>
#include <widgetEx/constraintlayout/core/widgets/constraintwidgetcontainer.h>

using namespace cdroid;

// Pin the parent's horizontal extent to [0, width].
static void pinParentH(LinearSystem& system, ConstraintWidget& parent, int width) {
    system.addEquality(system.createObjectVariable(parent.getAnchor(ConstraintAnchor::Type::LEFT)), 0);
    system.addEquality(system.createObjectVariable(parent.getAnchor(ConstraintAnchor::Type::RIGHT)), width);
}

// Pin all four parent edges (full frame).
static void pinParent(LinearSystem& system, ConstraintWidget& parent, int w, int h) {
    system.addEquality(system.createObjectVariable(parent.getAnchor(ConstraintAnchor::Type::LEFT)), 0);
    system.addEquality(system.createObjectVariable(parent.getAnchor(ConstraintAnchor::Type::RIGHT)), w);
    system.addEquality(system.createObjectVariable(parent.getAnchor(ConstraintAnchor::Type::TOP)), 0);
    system.addEquality(system.createObjectVariable(parent.getAnchor(ConstraintAnchor::Type::BOTTOM)), h);
}

// A child pinned to the parent's left edge with a fixed width and left margin.
TEST(ConstraintWidget, PinnedLeftWithMargin) {
    ConstraintWidgetContainer parent(600, 400);
    ConstraintWidget child(100, 50); // fixed 100 x 50
    parent.add(&child);

    LinearSystem& system = parent.getSystem();
    pinParentH(system, parent, 600);

    SolverVariable* cL = system.createObjectVariable(child.getAnchor(ConstraintAnchor::Type::LEFT));
    SolverVariable* cR = system.createObjectVariable(child.getAnchor(ConstraintAnchor::Type::RIGHT));
    SolverVariable* pL = system.createObjectVariable(parent.getAnchor(ConstraintAnchor::Type::LEFT));
    system.addEquality(cL, pL, 20, SolverVariable::STRENGTH_FIXED); // cL = pL + 20
    system.addEquality(cR, cL, 100, SolverVariable::STRENGTH_FIXED); // width 100

    system.minimize();
    child.updateFromSolver(&system, false);

    EXPECT_EQ(child.getX(), 20);
    EXPECT_EQ(child.getWidth(), 100);
}

// A fixed-width child centered horizontally (bias 0.5) inside the parent.
TEST(ConstraintWidget, CenteredBias) {
    ConstraintWidgetContainer parent(600, 400);
    ConstraintWidget child(100, 50);
    parent.add(&child);

    LinearSystem& system = parent.getSystem();
    pinParentH(system, parent, 600);

    SolverVariable* cL = system.createObjectVariable(child.getAnchor(ConstraintAnchor::Type::LEFT));
    SolverVariable* cR = system.createObjectVariable(child.getAnchor(ConstraintAnchor::Type::RIGHT));
    SolverVariable* pL = system.createObjectVariable(parent.getAnchor(ConstraintAnchor::Type::LEFT));
    SolverVariable* pR = system.createObjectVariable(parent.getAnchor(ConstraintAnchor::Type::RIGHT));
    system.addEquality(cR, cL, 100, SolverVariable::STRENGTH_FIXED); // width 100
    // addCentering(widgetLeft, targetLeft, m1, bias, targetRight, widgetRight, m2, strength)
    system.addCentering(cL, pL, 0, 0.5f, pR, cR, 0, SolverVariable::STRENGTH_FIXED);

    system.minimize();
    child.updateFromSolver(&system, false);

    EXPECT_EQ(child.getX(), 250);
    EXPECT_EQ(child.getWidth(), 100);
}

// A child that fills the parent horizontally (left=parent.left, right=parent.right).
TEST(ConstraintWidget, FillParent) {
    ConstraintWidgetContainer parent(600, 400);
    ConstraintWidget child(0, 0);
    parent.add(&child);

    LinearSystem& system = parent.getSystem();
    pinParentH(system, parent, 600);

    SolverVariable* cL = system.createObjectVariable(child.getAnchor(ConstraintAnchor::Type::LEFT));
    SolverVariable* cR = system.createObjectVariable(child.getAnchor(ConstraintAnchor::Type::RIGHT));
    SolverVariable* pL = system.createObjectVariable(parent.getAnchor(ConstraintAnchor::Type::LEFT));
    SolverVariable* pR = system.createObjectVariable(parent.getAnchor(ConstraintAnchor::Type::RIGHT));
    system.addEquality(cL, pL, 0, SolverVariable::STRENGTH_FIXED);
    system.addEquality(cR, pR, 0, SolverVariable::STRENGTH_FIXED);

    system.minimize();
    child.updateFromSolver(&system, false);

    EXPECT_EQ(child.getX(), 0);
    EXPECT_EQ(child.getWidth(), 600);
}

// A child offset from both edges (left margin 30, right margin 50) in a 600-wide parent.
TEST(ConstraintWidget, MarginsBothSides) {
    ConstraintWidgetContainer parent(600, 400);
    ConstraintWidget child(0, 0);
    parent.add(&child);

    LinearSystem& system = parent.getSystem();
    pinParentH(system, parent, 600);

    SolverVariable* cL = system.createObjectVariable(child.getAnchor(ConstraintAnchor::Type::LEFT));
    SolverVariable* cR = system.createObjectVariable(child.getAnchor(ConstraintAnchor::Type::RIGHT));
    SolverVariable* pL = system.createObjectVariable(parent.getAnchor(ConstraintAnchor::Type::LEFT));
    SolverVariable* pR = system.createObjectVariable(parent.getAnchor(ConstraintAnchor::Type::RIGHT));
    system.addEquality(cL, pL, 30, SolverVariable::STRENGTH_FIXED); // cL = pL + 30
    system.addEquality(cR, pR, -50, SolverVariable::STRENGTH_FIXED); // cR = pR - 50

    system.minimize();
    child.updateFromSolver(&system, false);

    EXPECT_EQ(child.getX(), 30);
    EXPECT_EQ(child.getWidth(), 520); // 600 - 30 - 50
}

// ---- addToSolver end-to-end: real anchor connections auto-populate the solver ----

// A fixed-width child whose left+right are both connected to the parent → centered.
// Exercises ConstraintWidget::addToSolver -> applyConstraints (centering path).
TEST(ConstraintWidget, AddToSolverCentered) {
    ConstraintWidgetContainer parent(600, 400);
    ConstraintWidget child(100, 50); // fixed 100 x 50
    parent.add(&child);
    child.mLeft.connect(&parent.mLeft, 0);
    child.mRight.connect(&parent.mRight, 0);

    LinearSystem& system = parent.getSystem();
    pinParent(system, parent, 600, 400);
    child.addToSolver(&system, false);
    system.minimize();
    child.updateFromSolver(&system, false);

    EXPECT_EQ(child.getX(), 250); // (600 - 100) / 2
    EXPECT_EQ(child.getWidth(), 100);
}

// A fixed-width child connected leftToLeft with a margin, right unconnected → pinned left.
TEST(ConstraintWidget, AddToSolverPinnedLeft) {
    ConstraintWidgetContainer parent(600, 400);
    ConstraintWidget child(100, 50);
    parent.add(&child);
    child.mLeft.connect(&parent.mLeft, 20); // leftToLeft, margin 20

    LinearSystem& system = parent.getSystem();
    pinParent(system, parent, 600, 400);
    child.addToSolver(&system, false);
    system.minimize();
    child.updateFromSolver(&system, false);

    EXPECT_EQ(child.getX(), 20);
    EXPECT_EQ(child.getWidth(), 100);
}

// ---- container.layout() end-to-end: the real MVP driver ----
// Anchor connects + layout() only — addToSolver/Chain/minimize/updateFromSolver all internal.

// A fixed-width child whose left+right connect to the parent → centered by layout().
TEST(ConstraintWidget, LayoutCentered) {
    ConstraintWidgetContainer parent(600, 400);
    ConstraintWidget child(100, 50);
    parent.add(&child);
    child.mLeft.connect(&parent.mLeft, 0);
    child.mRight.connect(&parent.mRight, 0);

    parent.layout();

    EXPECT_EQ(child.getX(), 250); // (600 - 100) / 2
    EXPECT_EQ(child.getWidth(), 100);
}

// A fixed-width child connected leftToLeft with a margin → pinned left by layout().
TEST(ConstraintWidget, LayoutPinnedLeft) {
    ConstraintWidgetContainer parent(600, 400);
    ConstraintWidget child(100, 50);
    parent.add(&child);
    child.mLeft.connect(&parent.mLeft, 20);

    parent.layout();

    EXPECT_EQ(child.getX(), 20);
    EXPECT_EQ(child.getWidth(), 100);
}

// Two children: one pinned left, one pinned right.
TEST(ConstraintWidget, LayoutTwoChildren) {
    ConstraintWidgetContainer parent(600, 400);
    ConstraintWidget left(100, 50);
    ConstraintWidget right(80, 50);
    parent.add(&left);
    parent.add(&right);
    left.mLeft.connect(&parent.mLeft, 10);    // left child at x=10
    right.mRight.connect(&parent.mRight, 30);  // right child's right edge at 600-30=570

    parent.layout();

    EXPECT_EQ(left.getX(), 10);
    EXPECT_EQ(left.getWidth(), 100);
    EXPECT_EQ(right.getX() + right.getWidth(), 570); // right edge at 570
    EXPECT_EQ(right.getWidth(), 80);
}

#endif // ENABLE_CONSTRAINTLAYOUT
