// Direct ports of androidx.constraintlayout.core.ChainWrapContentTest. Each scenario runs at both
// OPTIMIZATION_NONE (0) and OPTIMIZATION_STANDARD (=DIRECT=1), via a value-parameterized fixture.
// WRAP_CONTENT cases currently FAIL on CDROID (core layout() does not collapse WRAP_CONTENT — needs
// the View-layer measure pass); those failures are kept as CDROID bug markers.
//
// Source: /home/houzh/research/constraintlayout/.../androidx/constraintlayout/core/ChainWrapContentTest.java
#include <gtest/gtest.h>
#include <widgetEx/constraintlayout/core/widgets/constraintwidget.h>
#include <widgetEx/constraintlayout/core/widgets/constraintwidgetcontainer.h>
#include "chain_helpers.h"

using namespace cdroid;
using DB = ConstraintWidget::DimensionBehaviour;

class CLCoreChainWrapContent : public ::testing::TestWithParam<int> {};
INSTANTIATE_TEST_SUITE_P(OptLevels, CLCoreChainWrapContent, ::testing::Values(0, 1));

// Ported from ChainWrapContentTest.testVerticalWrapContentChain: root vertical WRAP, B MATCH, A.top
// margin 10, C.bottom margin 32 — B collapses to 0; root wraps to 82.
TEST_P(CLCoreChainWrapContent, VerticalWrapContentChain) {
    ConstraintWidgetContainer root("root", 600, 600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::WRAP_CONTENT);
    root.setOptimizationLevel(GetParam());
    ConstraintWidget a("A", 100, 20), b("B", 100, 20), c("C", 100, 20);
    b.setVerticalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    root.add(&a); root.add(&b); root.add(&c);
    using namespace clport;
    connect(a, Side::TOP,    root, Side::TOP, 10); connect(a, Side::BOTTOM, b, Side::TOP);
    connect(b, Side::TOP,    a,    Side::BOTTOM);  connect(b, Side::BOTTOM, c, Side::TOP);
    connect(c, Side::TOP,    b,    Side::BOTTOM);  connect(c, Side::BOTTOM, root, Side::BOTTOM, 32);
    root.layout();
    EXPECT_EQ(getTop(a), 10);
    EXPECT_EQ(getTop(b), 30);
    EXPECT_EQ(getTop(c), 30);
    EXPECT_EQ(root.getHeight(), 82);
}

// Ported from ChainWrapContentTest.testHorizontalWrapContentChain: root horizontal WRAP after a
// fixed pass; then minWidth 400.
TEST_P(CLCoreChainWrapContent, HorizontalWrapContentChain) {
    ConstraintWidgetContainer root("root", 600, 600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    root.setOptimizationLevel(GetParam());
    ConstraintWidget a("A", 100, 20), b("B", 100, 20), c("C", 100, 20);
    b.setHorizontalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    root.add(&a); root.add(&b); root.add(&c);
    using namespace clport;
    connect(a, Side::LEFT,  root, Side::LEFT, 10); connect(a, Side::RIGHT, b, Side::LEFT);
    connect(b, Side::LEFT,  a,    Side::RIGHT);    connect(b, Side::RIGHT, c, Side::LEFT);
    connect(c, Side::LEFT,  b,    Side::RIGHT);    connect(c, Side::RIGHT, root, Side::RIGHT, 32);
    root.layout();
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::WRAP_CONTENT);
    root.layout();
    EXPECT_EQ(getLeft(a), 10);
    EXPECT_EQ(getLeft(b), 110);
    EXPECT_EQ(getLeft(c), 110);
    EXPECT_EQ(root.getWidth(), 242);
    root.setMinWidth(400);
    root.layout();
    EXPECT_EQ(getLeft(a), 10);
    EXPECT_EQ(getLeft(b), 110);
    EXPECT_EQ(getLeft(c), 268);
    EXPECT_EQ(root.getWidth(), 400);
}

// Ported from ChainWrapContentTest.testVerticalWrapContentChain3Elts: 4 widgets, root WRAP, B&C
// MATCH; then minHeight 300; then fixed 600.
TEST_P(CLCoreChainWrapContent, VerticalWrapContentChain3Elts) {
    ConstraintWidgetContainer root("root", 600, 600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::WRAP_CONTENT);
    root.setOptimizationLevel(GetParam());
    ConstraintWidget a("A", 100, 20), b("B", 100, 20), c("C", 100, 20), d("D", 100, 20);
    b.setVerticalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    c.setVerticalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    root.add(&a); root.add(&b); root.add(&c); root.add(&d);
    using namespace clport;
    connect(a, Side::TOP,    root, Side::TOP, 10); connect(a, Side::BOTTOM, b, Side::TOP);
    connect(b, Side::TOP,    a,    Side::BOTTOM);  connect(b, Side::BOTTOM, c, Side::TOP);
    connect(c, Side::TOP,    b,    Side::BOTTOM);  connect(c, Side::BOTTOM, d, Side::TOP);
    connect(d, Side::TOP,    c,    Side::BOTTOM);  connect(d, Side::BOTTOM, root, Side::BOTTOM, 32);
    root.layout();
    EXPECT_EQ(getTop(a), 10);
    EXPECT_EQ(getTop(b), 30);
    EXPECT_EQ(getTop(c), 30);
    EXPECT_EQ(getTop(d), 30);
    EXPECT_EQ(root.getHeight(), 82);
    root.setMinHeight(300);
    root.layout();
    EXPECT_EQ(getTop(a), 10);
    EXPECT_EQ(getTop(b), 30);
    EXPECT_EQ(getTop(c), 139);
    EXPECT_EQ(getTop(d), 248);
    EXPECT_EQ(root.getHeight(), 300);
    root.setHeight(600);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    root.layout();
    EXPECT_EQ(getTop(a), 10);
    EXPECT_EQ(getTop(b), 30);
    EXPECT_EQ(getTop(c), 289);
    EXPECT_EQ(getTop(d), 548);
    EXPECT_EQ(root.getHeight(), 600);
}

// Ported from ChainWrapContentTest.testHorizontalWrapContentChain3Elts: 4 widgets, root WRAP, B&C
// MATCH; then minWidth 300; then fixed 600.
TEST_P(CLCoreChainWrapContent, HorizontalWrapContentChain3Elts) {
    ConstraintWidgetContainer root("root", 600, 600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::WRAP_CONTENT);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    root.setOptimizationLevel(GetParam());
    ConstraintWidget a("A", 100, 20), b("B", 100, 20), c("C", 100, 20), d("D", 100, 20);
    b.setHorizontalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    c.setHorizontalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    root.add(&a); root.add(&b); root.add(&c); root.add(&d);
    using namespace clport;
    connect(a, Side::LEFT,  root, Side::LEFT, 10); connect(a, Side::RIGHT, b, Side::LEFT);
    connect(b, Side::LEFT,  a,    Side::RIGHT);    connect(b, Side::RIGHT, c, Side::LEFT);
    connect(c, Side::LEFT,  b,    Side::RIGHT);    connect(c, Side::RIGHT, d, Side::LEFT);
    connect(d, Side::LEFT,  c,    Side::RIGHT);    connect(d, Side::RIGHT, root, Side::RIGHT, 32);
    root.layout();
    EXPECT_EQ(getLeft(a), 10);
    EXPECT_EQ(getLeft(b), 110);
    EXPECT_EQ(getLeft(c), 110);
    EXPECT_EQ(getLeft(d), 110);
    EXPECT_EQ(root.getWidth(), 242);
    root.setMinWidth(300);
    root.layout();
    EXPECT_EQ(getLeft(c), 139);
    EXPECT_EQ(getLeft(d), 168);
    EXPECT_EQ(root.getWidth(), 300);
    root.setWidth(600);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.layout();
    EXPECT_EQ(getLeft(c), 289);
    EXPECT_EQ(getLeft(d), 468);
    EXPECT_EQ(root.getWidth(), 600);
}

// Ported from ChainWrapContentTest.testWrapChain: A-D horizontal chain, E under C, root width
// FIXED 1440 / height WRAP -> root.height=336.
TEST(CLCoreChainWrap, WrapChain) {
    ConstraintWidgetContainer root("root", 1440, 1944);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::WRAP_CONTENT);
    ConstraintWidget a("A", 308, 168), b("B", 308, 168), c("C", 308, 168), d("D", 308, 168), e("E", 308, 168);
    root.add(&e); root.add(&a); root.add(&b); root.add(&c); root.add(&d);
    using namespace clport;
    connect(a, Side::LEFT,  root, Side::LEFT);  connect(a, Side::RIGHT, b, Side::LEFT);
    connect(b, Side::LEFT,  a,    Side::RIGHT); connect(b, Side::RIGHT, c, Side::LEFT);
    connect(c, Side::LEFT,  b,    Side::RIGHT); connect(c, Side::RIGHT, d, Side::LEFT);
    connect(d, Side::LEFT,  c,    Side::RIGHT); connect(d, Side::RIGHT, root, Side::RIGHT);
    connect(e, Side::LEFT, root, Side::LEFT); connect(e, Side::RIGHT, root, Side::RIGHT);
    connect(e, Side::TOP,  c,     Side::BOTTOM);
    root.layout();
    EXPECT_EQ(root.getWidth(), 1440);
    EXPECT_EQ(root.getHeight(), 336);
}

// Ported from ChainWrapContentTest.testWrapDanglingChain: two widgets, B dangling (no right
// anchor to root), root both WRAP -> root 616x168, A@0 B@308.
TEST(CLCoreChainWrap, WrapDanglingChain) {
    ConstraintWidgetContainer root("root", 1440, 1944);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::WRAP_CONTENT);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::WRAP_CONTENT);
    ConstraintWidget a("A", 308, 168), b("B", 308, 168);
    root.add(&a); root.add(&b);
    using namespace clport;
    connect(a, Side::LEFT,  root, Side::LEFT);
    connect(a, Side::RIGHT, b,    Side::LEFT);
    connect(b, Side::LEFT,  a,    Side::RIGHT);
    root.layout();
    EXPECT_EQ(root.getWidth(), 616);
    EXPECT_EQ(root.getHeight(), 168);
    EXPECT_EQ(getLeft(a), 0);
    EXPECT_EQ(getLeft(b), 308);
    EXPECT_EQ(a.getWidth(), 308);
    EXPECT_EQ(b.getWidth(), 308);
}

// Ported from ChainWrapContentTest.testHorizontalWrapChain: multi-scenario (a-e) — B is
// MATCH_CONSTRAINT_WRAP with width 600 (must not expand the 600px container); then packed; then
// width 100; then root vertical WRAP; then root width 0 horizontal WRAP.
TEST(CLCoreChainWrap, HorizontalWrapChain) {
    ConstraintWidgetContainer root("root", 600, 1000);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::FIXED);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::FIXED);
    ConstraintWidget a("A", 20, 20), b("B", 100, 20), c("C", 20, 20);
    b.setHorizontalDimensionBehaviour(DB::MATCH_CONSTRAINT);
    clport::setHorizontalMatchStyle(b, ConstraintWidget::MATCH_CONSTRAINT_WRAP, 0, 0, 0);
    b.setWidth(600);
    root.add(&a); root.add(&b); root.add(&c);
    using namespace clport;
    connect(a, Side::LEFT,  root, Side::LEFT);  connect(a, Side::RIGHT, b, Side::LEFT);
    connect(b, Side::LEFT,  a,    Side::RIGHT); connect(b, Side::RIGHT, c, Side::LEFT);
    connect(c, Side::LEFT,  b,    Side::RIGHT); connect(c, Side::RIGHT, root, Side::RIGHT);
    root.layout();
    EXPECT_EQ(getLeft(a), 0);
    EXPECT_EQ(getLeft(b), 20);
    EXPECT_EQ(getLeft(c), 580);
    setHorizontalChainStyle(a, ConstraintWidget::CHAIN_PACKED);
    b.setWidth(600);
    root.layout();
    EXPECT_EQ(getLeft(a), 0);
    EXPECT_EQ(getLeft(b), 20);
    EXPECT_EQ(getLeft(c), 580);
    b.setWidth(100);
    root.layout();
    EXPECT_EQ(getLeft(a), 230);
    EXPECT_EQ(getLeft(b), 250);
    EXPECT_EQ(getLeft(c), 350);
    b.setWidth(600);
    root.setDimensionBehaviour(ConstraintWidget::VERTICAL, DB::WRAP_CONTENT);
    connect(a, Side::TOP, root, Side::TOP);
    connect(b, Side::TOP, root, Side::TOP);
    connect(c, Side::TOP, root, Side::TOP);
    root.layout();
    EXPECT_EQ(root.getHeight(), 20);
    EXPECT_EQ(getLeft(a), 0);
    EXPECT_EQ(getLeft(b), 20);
    EXPECT_EQ(getLeft(c), 580);
    b.setHorizontalDimensionBehaviour(DB::FIXED);
    b.setWidth(600);
    root.setWidth(0);
    root.setDimensionBehaviour(ConstraintWidget::HORIZONTAL, DB::WRAP_CONTENT);
    root.layout();
    EXPECT_EQ(root.getHeight(), 20);
    EXPECT_EQ(getLeft(a), 0);
    EXPECT_EQ(getLeft(b), 20);
    EXPECT_EQ(getLeft(c), 620);
}
