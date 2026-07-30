/*
 * Pure-math oracle tests for the LinearSystem Cassowary solver.
 * Uses ConstraintWidget anchors + createObjectVariable (the public API).
 * Gated on ENABLE_CONSTRAINTLAYOUT.
 */
#include <gui_features.h>
#ifdef ENABLE_CONSTRAINTLAYOUT

#include <gtest/gtest.h>
#include <widgetEx/constraintlayout/core/linearsystem.h>
#include <widgetEx/constraintlayout/core/solvervariable.h>
#include <widgetEx/constraintlayout/core/widgets/constraintwidget.h>

using namespace cdroid;

static ConstraintWidget makeWidget() { return ConstraintWidget(100, 50); }

// Pin a widget's left anchor to 42.
TEST(LinearSystem, SimpleEquality) {
    LinearSystem ls;
    ConstraintWidget w = makeWidget();
    SolverVariable* left = ls.createObjectVariable(&w.mLeft);
    ls.addEquality(left, 42);
    ls.minimize();
    EXPECT_NEAR(left->computedValue, 42.0, 1e-6);
}

// Two independent pins.
TEST(LinearSystem, TwoEqualities) {
    LinearSystem ls;
    ConstraintWidget a = makeWidget();
    ConstraintWidget b = makeWidget();
    SolverVariable* av = ls.createObjectVariable(&a.mLeft);
    SolverVariable* bv = ls.createObjectVariable(&b.mLeft);
    ls.addEquality(av, 10);
    ls.addEquality(bv, 20);
    ls.minimize();
    EXPECT_NEAR(av->computedValue, 10.0, 1e-6);
    EXPECT_NEAR(bv->computedValue, 20.0, 1e-6);
}

// a = b (variable-to-variable equality at FIXED strength).
TEST(LinearSystem, VariableEquality) {
    LinearSystem ls;
    ConstraintWidget a = makeWidget();
    ConstraintWidget b = makeWidget();
    SolverVariable* av = ls.createObjectVariable(&a.mLeft);
    SolverVariable* bv = ls.createObjectVariable(&b.mLeft);
    ls.addEquality(bv, 50);
    ls.addEquality(av, bv, 0, SolverVariable::STRENGTH_FIXED);
    ls.minimize();
    EXPECT_NEAR(av->computedValue, 50.0, 1e-6);
}

// Center: widgetLeft centered between 0 and 600 with bias 0.5.
TEST(LinearSystem, CenterSpan) {
    LinearSystem ls;
    ConstraintWidget parent(600, 400);
    ConstraintWidget child(100, 50);
    // Pin parent left=0, parent right=600.
    SolverVariable* pLeft = ls.createObjectVariable(&parent.mLeft);
    SolverVariable* pRight = ls.createObjectVariable(&parent.mRight);
    SolverVariable* cLeft = ls.createObjectVariable(&child.mLeft);
    SolverVariable* cRight = ls.createObjectVariable(&child.mRight);
    ls.addEquality(pLeft, 0);
    ls.addEquality(pRight, 600);
    ls.addEquality(cRight, cLeft, 100, SolverVariable::STRENGTH_FIXED); // width=100
    ls.addCentering(cLeft, pLeft, 0, 0.5f, pRight, cRight, 0, SolverVariable::STRENGTH_FIXED);
    ls.minimize();
    // centered: (600-100)/2 = 250
    EXPECT_NEAR(cLeft->computedValue, 250.0, 1.0);
}

// a >= b + margin (greater-than bound).
TEST(LinearSystem, GreaterThanBound) {
    LinearSystem ls;
    ConstraintWidget a = makeWidget();
    ConstraintWidget b = makeWidget();
    SolverVariable* av = ls.createObjectVariable(&a.mLeft);
    SolverVariable* bv = ls.createObjectVariable(&b.mLeft);
    ls.addEquality(bv, 100);
    ls.addGreaterThan(av, bv, 50, SolverVariable::STRENGTH_FIXED);
    ls.minimize();
    // a >= 150, minimized → 150
    EXPECT_GE(av->computedValue, 149.0);
}

// a <= b + margin (lower-than bound).
TEST(LinearSystem, LowerThanBound) {
    LinearSystem ls;
    ConstraintWidget a = makeWidget();
    ConstraintWidget b = makeWidget();
    SolverVariable* av = ls.createObjectVariable(&a.mLeft);
    SolverVariable* bv = ls.createObjectVariable(&b.mLeft);
    ls.addEquality(bv, 200);
    ls.addLowerThan(av, bv, 30, SolverVariable::STRENGTH_FIXED);
    ls.minimize();
    EXPECT_LE(av->computedValue, 231.0);
}

// addEquality(a, b, margin, strength) — synonym with margin.
TEST(LinearSystem, SynonymWithMargin) {
    LinearSystem ls;
    ConstraintWidget a = makeWidget();
    ConstraintWidget b = makeWidget();
    SolverVariable* av = ls.createObjectVariable(&a.mLeft);
    SolverVariable* bv = ls.createObjectVariable(&b.mLeft);
    ls.addEquality(bv, 100);
    ls.addEquality(av, bv, 30, SolverVariable::STRENGTH_FIXED);
    ls.minimize();
    EXPECT_NEAR(av->computedValue, 130.0, 1e-6);
}

#endif // ENABLE_CONSTRAINTLAYOUT
