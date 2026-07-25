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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.LinearSystemTest.
 *
 * Direct-API smoke test of the Cassowary solver. The AndroidX original is built on a
 * test-only LinearEquation DSL (~44KB); per the chosen approach we instead exercise the real
 * LinearSystem public API (addEquality / addCentering / addGreaterThan / addSynonym) and assert
 * the same deterministic oracle values. Cases that need arbitrary linear expressions
 * (Al - Rl = Rr - Ar, 2*Xm = Xl + Xr) are deferred to a later DSL port.
 *
 * Gated on ENABLE_CONSTRAINTLAYOUT: when the feature is off the solver objects are not in the
 * cdroid library, so this TU compiles to nothing (same pattern as image_unittests.cc's
 * ENABLE_CAIROSVG guard).
 */
#include <gui_features.h>
#ifdef ENABLE_CONSTRAINTLAYOUT

#include <gtest/gtest.h>

#include <utility>

#include <widgetEx/constraintlayout/core/linearsystem.h>
#include <widgetEx/constraintlayout/core/solvervariable.h>

using namespace cdroid;

// LinearSystemTest.testAddEquation1 — a single equality pins a variable.
TEST(LinearSystem, SimpleEquality) {
    LinearSystem ls;
    SolverVariable* a = ls.getVariable("A", SolverVariable::Type::UNRESTRICTED);
    ls.addEquality(a, 100);
    EXPECT_FLOAT_EQ(ls.getValueFor("A"), 100.0f);
}

// LinearSystemTest.testAddEquation2 — two independent equalities.
TEST(LinearSystem, TwoEqualities) {
    LinearSystem ls;
    SolverVariable* left  = ls.getVariable("W3.left",  SolverVariable::Type::UNRESTRICTED);
    SolverVariable* right = ls.getVariable("W3.right", SolverVariable::Type::UNRESTRICTED);
    ls.addEquality(left, 0);
    ls.addEquality(right, 600);
    EXPECT_FLOAT_EQ(ls.getValueFor("W3.left"), 0.0f);
    EXPECT_FLOAT_EQ(ls.getValueFor("W3.right"), 600.0f);
}

// LinearSystemTest.testAddEquation3 — variable == variable (W4.left == W3.left).
TEST(LinearSystem, VariableEquality) {
    LinearSystem ls;
    SolverVariable* w3l = ls.getVariable("W3.left", SolverVariable::Type::UNRESTRICTED);
    SolverVariable* w3r = ls.getVariable("W3.right", SolverVariable::Type::UNRESTRICTED);
    SolverVariable* w4l = ls.getVariable("W4.left", SolverVariable::Type::UNRESTRICTED);
    ls.addEquality(w3l, 0);
    ls.addEquality(w3r, 600);
    ls.addEquality(w4l, w3l, 0, SolverVariable::STRENGTH_FIXED); // W4.left = W3.left
    EXPECT_FLOAT_EQ(ls.getValueFor("W3.left"), 0.0f);
    EXPECT_FLOAT_EQ(ls.getValueFor("W3.right"), 600.0f);
    EXPECT_FLOAT_EQ(ls.getValueFor("W4.left"), 0.0f);
}

// Center a widget span of given width inside [lo, hi] at a bias.
// Call convention (ConstraintWidget/Chain): addCentering(widgetLeft, targetLeft, m1, bias,
// targetRight, widgetRight, m2). leftGap = bias * slack, slack = (hi - lo) - width.
// Returns {widgetLeft, widgetRight}. Exercises addCentering -> createRowCentering -> pivot.
static std::pair<float, float> centeredSpan(int lo, int hi, int width, float bias) {
    LinearSystem ls;
    SolverVariable* tl = ls.getVariable("TL", SolverVariable::Type::UNRESTRICTED);
    SolverVariable* tr = ls.getVariable("TR", SolverVariable::Type::UNRESTRICTED);
    SolverVariable* wl = ls.getVariable("WL", SolverVariable::Type::UNRESTRICTED);
    SolverVariable* wr = ls.getVariable("WR", SolverVariable::Type::UNRESTRICTED);
    ls.addEquality(tl, lo);
    ls.addEquality(tr, hi);
    ls.addEquality(wr, wl, width, SolverVariable::STRENGTH_FIXED); // width: WR = WL + width
    ls.addCentering(wl, tl, 0, bias, tr, wr, 0, SolverVariable::STRENGTH_FIXED);
    return { ls.getValueFor("WL"), ls.getValueFor("WR") };
}

TEST(LinearSystem, CenterSpan) {
    auto p0 = centeredSpan(0, 600, 100, 0.0f);
    EXPECT_NEAR(p0.first, 0.0f,   0.5f);   EXPECT_NEAR(p0.second, 100.0f, 0.5f); // pinned left
    auto p1 = centeredSpan(0, 600, 100, 0.25f);
    EXPECT_NEAR(p1.first, 125.0f, 0.5f);   EXPECT_NEAR(p1.second, 225.0f, 0.5f); // quarter
    auto p2 = centeredSpan(0, 600, 100, 0.5f);
    EXPECT_NEAR(p2.first, 250.0f, 0.5f);   EXPECT_NEAR(p2.second, 350.0f, 0.5f); // centered
    auto p3 = centeredSpan(0, 600, 100, 1.0f);
    EXPECT_NEAR(p3.first, 500.0f, 0.5f);   EXPECT_NEAR(p3.second, 600.0f, 0.5f); // pinned right
}

// Greater-than resolves the variable to its lower bound (slack collapses to 0) when there is
// no competing soft goal. Oracle: A >= B + 100, B = 0  =>  A == 100.
TEST(LinearSystem, GreaterThanBound) {
    LinearSystem ls;
    SolverVariable* b = ls.getVariable("B", SolverVariable::Type::UNRESTRICTED);
    SolverVariable* a = ls.getVariable("A", SolverVariable::Type::UNRESTRICTED);
    ls.addEquality(b, 0);
    ls.addGreaterThan(a, b, 100, SolverVariable::STRENGTH_FIXED);
    ls.minimize();
    EXPECT_NEAR(ls.getValueFor("A"), 100.0f, 0.5f);
    EXPECT_GE(ls.getValueFor("A"), 100.0f - 0.5f);
}

// Lower-than: A <= 50 with A also pulled toward 0 by a soft equality collapses to the bound.
TEST(LinearSystem, LowerThanBound) {
    LinearSystem ls;
    SolverVariable* a = ls.getVariable("A", SolverVariable::Type::UNRESTRICTED);
    ls.addLowerThan(a, /*b=*/ls.getVariable("zero", SolverVariable::Type::UNRESTRICTED),
                    50, SolverVariable::STRENGTH_FIXED);
    ls.addEquality(ls.getVariable("zero", SolverVariable::Type::UNRESTRICTED), 0);
    EXPECT_LE(ls.getValueFor("A"), 50.0f + 0.5f);
}

// addSynonym with a non-zero margin delegates to addEquality(a, b, margin, FIXED),
// so A = B + margin is a real, readable variable. (The margin==0 path substitutes A out of
// the system entirely; by design getValueFor does not resolve synonyms, matching Android's
// computeValues which only updates each row's mVariable — so that path is not asserted here.)
TEST(LinearSystem, SynonymWithMargin) {
    LinearSystem ls;
    SolverVariable* b = ls.getVariable("B", SolverVariable::Type::UNRESTRICTED);
    SolverVariable* a = ls.getVariable("A", SolverVariable::Type::UNRESTRICTED);
    ls.addEquality(b, 250);
    ls.addSynonym(a, b, 5); // A = B + 5
    EXPECT_NEAR(ls.getValueFor("A"), 255.0f, 0.5f);
    EXPECT_NEAR(ls.getValueFor("B"), 250.0f, 0.5f);
}

#endif // ENABLE_CONSTRAINTLAYOUT
