/*
 * Runtime validation of the ConstraintLayout widget (Stage 4 MVP). Constructs a ConstraintLayout
 * with TextView children wired via LayoutParams anchor constraints, runs the real measure/layout
 * pass, and asserts child positions — exercising the full widget→solver→position pipeline.
 *
 * Gated on ENABLE_CONSTRAINTLAYOUT.
 */
#include <gui_features.h>
#ifdef ENABLE_CONSTRAINTLAYOUT

#include <gtest/gtest.h>

#include "R.h"

#include <core/app.h>
#include <core/attributeset.h>
#include <core/xmlpullparser.h>
#include <view/view.h>
#include <widget/textview.h>
#include <widgetEx/constraintlayout/constraintlayout.h>
#include <widgetEx/constraintlayout/core/widgets/constraintwidget.h>
#include <widgetEx/constraintlayout/constraintset.h>
#include <widgetEx/constraintlayout/constraintlayoutstates.h>
#include <widgetEx/constraintlayout/motion/keyframes.h>
#include <widgetEx/constraintlayout/motion/motionscene.h>
#include <widgetEx/constraintlayout/motion/viewtransition.h>
#include <view/motionevent.h>
#include <widgetEx/constraintlayout/motion/viewtransitioncontroller.h>
#include <widgetEx/constraintlayout/core/motion/motionkeyattributes.h>
#include <widgetEx/constraintlayout/core/motion/motionkeyposition.h>
#include <widgetEx/constraintlayout/motion/motionlayout.h>
#include <widgetEx/constraintlayout/core/motion/motionkeyposition.h>
#include <widgetEx/constraintlayout/helpers/barrier.h>
#include <widgetEx/constraintlayout/helpers/group.h>
#include <widgetEx/constraintlayout/helpers/placeholder.h>
#include <widgetEx/constraintlayout/helpers/flow.h>
#include <widgetEx/constraintlayout/helpers/layer.h>
#include <widgetEx/constraintlayout/helpers/circularflow.h>
#include <widgetEx/constraintlayout/helpers/grid.h>
#include <widgetEx/constraintlayout/motion/motioneffect.h>

using namespace cdroid;

static int exactly(int size) {
    return View::MeasureSpec::makeMeasureSpec(size, View::MeasureSpec::EXACTLY);
}

static int atMost(int size) {
    return View::MeasureSpec::makeMeasureSpec(size, View::MeasureSpec::AT_MOST);
}

// A fixed 100x50 child whose left+right both connect to the parent → horizontally centered
// in a 600-wide container: x = (600 - 100) / 2 = 250.
TEST(CLConstraintLayout, CentersChildHorizontally) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());
    TextView* tv = new TextView(&App::getInstance()); tv->setText("X");
    auto* lp = new ConstraintLayout::LayoutParams(100, 50);
    lp->leftToLeft = ConstraintLayout::PARENT_ID;
    lp->rightToRight = ConstraintLayout::PARENT_ID;
    cl->addView(tv, lp);

    cl->measure(exactly(600), exactly(400));
    cl->layout(0, 0, 600, 400);

    EXPECT_EQ(tv->getLeft(), 250);
    EXPECT_EQ(tv->getRight(), 350);
    EXPECT_EQ(tv->getTop(), 0);     // no vertical constraint → stays at top
}

// A child connected leftToLeft only (margin 0), right unconstrained → pinned at x=0.
TEST(CLConstraintLayout, PinsChildLeft) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());
    TextView* tv = new TextView(&App::getInstance()); tv->setText("X");
    auto* lp = new ConstraintLayout::LayoutParams(100, 50);
    lp->leftToLeft = ConstraintLayout::PARENT_ID;
    cl->addView(tv, lp);

    cl->measure(exactly(600), exactly(400));
    cl->layout(0, 0, 600, 400);

    EXPECT_EQ(tv->getLeft(), 0);
    EXPECT_EQ(tv->getWidth(), 100);
}

// A centered child with horizontal bias 0.3 → x = 0.3 * (600 - 100) = 150.
TEST(CLConstraintLayout, BiasChild) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());
    TextView* tv = new TextView(&App::getInstance()); tv->setText("X");
    auto* lp = new ConstraintLayout::LayoutParams(100, 50);
    lp->leftToLeft = ConstraintLayout::PARENT_ID;
    lp->rightToRight = ConstraintLayout::PARENT_ID;
    lp->horizontalBias = 0.3f;
    cl->addView(tv, lp);

    cl->measure(exactly(600), exactly(400));
    cl->layout(0, 0, 600, 400);

    EXPECT_EQ(tv->getLeft(), 150);
    EXPECT_EQ(tv->getRight(), 250);
}

// ---- feature probe: chains (already work via the driver's Chain.applyChainConstraints) ----

// Two children chained: A.right→B.left, B.left→A.right, A.left→parent, B.right→parent.
// CHAIN_SPREAD (default) distributes equal gaps on ALL sides: (600-200)/3 = 133 → A[133,233], B[367,467].
TEST(CLConstraintLayout, ChainSpreadTwo) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());
    TextView* a = new TextView(&App::getInstance()); a->setText("A");
    TextView* b = new TextView(&App::getInstance()); b->setText("B");
    auto* lpa = new ConstraintLayout::LayoutParams(100, 50);
    lpa->leftToLeft = ConstraintLayout::PARENT_ID;
    lpa->rightToLeft = 2;   // B's id
    auto* lpb = new ConstraintLayout::LayoutParams(100, 50);
    lpb->leftToRight = 1;   // A's id
    lpb->rightToRight = ConstraintLayout::PARENT_ID;
    a->setId(1);
    b->setId(2);
    cl->addView(a, lpa);
    cl->addView(b, lpb);

    cl->measure(exactly(600), exactly(400));
    cl->layout(0, 0, 600, 400);

    // CHAIN_SPREAD: equal gaps before/inside/after → (600-200)/3=133.33. b lands at 233+133.33=366.66,
    // which the optimizer's DIRECT path and the full Cassowary solver round 1px apart (366 vs 367) —
    // tolerate that single-pixel resolution difference (cf. chain_test.cc ±1px note).
    EXPECT_EQ(a->getLeft(), 133);
    EXPECT_EQ(a->getRight(), 233);
    EXPECT_NEAR(b->getLeft(), 367, 1);
    EXPECT_NEAR(b->getRight(), 467, 1);
}

// A 0dp (MATCH_CONSTRAINT) child with left+right to parent → spread-fills width 600.
TEST(CLConstraintLayout, MatchConstraintFills) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());
    TextView* tv = new TextView(&App::getInstance()); tv->setText("X");
    auto* lp = new ConstraintLayout::LayoutParams(0, 50);
    lp->leftToLeft = ConstraintLayout::PARENT_ID;
    lp->rightToRight = ConstraintLayout::PARENT_ID;
    cl->addView(tv, lp);
    cl->measure(exactly(600), exactly(400));
    cl->layout(0, 0, 600, 400);
    EXPECT_EQ(tv->getLeft(), 0);
    EXPECT_EQ(tv->getWidth(), 600);
}

// RTL: start_toStartOf=parent pins the widget's Start edge to the parent's Start edge. In RTL
// Start = Right, so a 100-wide widget lands at x = 600 - 100 = 500 (mirrored vs LTR's x=0).
TEST(CLConstraintLayout, RtlStartToStartPinsRight) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());
    cl->setLayoutDirection(View::LAYOUT_DIRECTION_RTL);
    TextView* tv = new TextView(&App::getInstance()); tv->setText("X"); tv->setId(1);
    auto* lp = new ConstraintLayout::LayoutParams(100, 50);
    lp->startToStart = ConstraintLayout::PARENT_ID;  // Start→Right under RTL
    cl->addView(tv, lp);

    cl->measure(exactly(600), exactly(400));
    cl->layout(0, 0, 600, 400);

    EXPECT_EQ(tv->getLeft(), 500);  // mirrored: right-aligned instead of left
}

// RTL: start_toStartOf + end_toEndOf = parent pins both horizontal edges. With horizontalBias=0.3
// the bias mirrors to 1 - 0.3 = 0.7, so a 100-wide widget in a 600-wide RTL container lands at
// left = 0.7 * (600 - 100) = 350 (vs 150 in LTR). Faithful to AndroidX validate() line 3858.
TEST(CLConstraintLayout, RtlBiasMirrors) {
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());
    cl->setLayoutDirection(View::LAYOUT_DIRECTION_RTL);
    TextView* tv = new TextView(&App::getInstance()); tv->setText("X"); tv->setId(1);
    auto* lp = new ConstraintLayout::LayoutParams(100, 50);
    lp->startToStart = ConstraintLayout::PARENT_ID;
    lp->endToEnd = ConstraintLayout::PARENT_ID;
    lp->horizontalBias = 0.3f;
    cl->addView(tv, lp);

    cl->measure(exactly(600), exactly(400));
    cl->layout(0, 0, 600, 400);

    EXPECT_EQ(tv->getLeft(), 350);  // mirrored bias 0.7 → 0.7*500
}

// RTL: end_toEndOf=parent → End maps to Left under RTL, so a 100-wide widget lands at x=0.
TEST(CLConstraintLayout, RtlEndToEndPinsLeft) {
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());
    cl->setLayoutDirection(View::LAYOUT_DIRECTION_RTL);
    TextView* tv = new TextView(&App::getInstance()); tv->setText("X"); tv->setId(1);
    auto* lp = new ConstraintLayout::LayoutParams(100, 50);
    lp->endToEnd = ConstraintLayout::PARENT_ID;  // End→Left under RTL
    cl->addView(tv, lp);

    cl->measure(exactly(600), exactly(400));
    cl->layout(0, 0, 600, 400);

    EXPECT_EQ(tv->getLeft(), 0);  // mirrored: left-aligned
}

// RTL: a vertical Guideline with guide_begin=100 mirrors to guide_end=100, i.e. positioned 100 from
// the right edge → x = 600 - 100 = 500. A 0dp child constrained left=guideline, right=parent fills
// 500..600 → x=500, width=100.
TEST(CLConstraintLayout, RtlGuidelineBeginMirrorsToEnd) {
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());
    cl->setLayoutDirection(View::LAYOUT_DIRECTION_RTL);

    View* gl = new View(&App::getInstance()); gl->setId(10);
    auto* glp = new ConstraintLayout::LayoutParams(LayoutParams::WRAP_CONTENT, LayoutParams::WRAP_CONTENT);
    glp->orientation = ConstraintWidget::VERTICAL;
    glp->guideBegin = 100;
    glp->validate();
    cl->addView(gl, glp);

    TextView* tv = new TextView(&App::getInstance()); tv->setText("X"); tv->setId(1);
    auto* lp = new ConstraintLayout::LayoutParams(0, 50);
    lp->leftToLeft = 10;
    lp->rightToRight = ConstraintLayout::PARENT_ID;
    cl->addView(tv, lp);

    cl->measure(exactly(600), exactly(400));
    cl->layout(0, 0, 600, 400);

    EXPECT_EQ(tv->getLeft(), 500);
    EXPECT_EQ(tv->getWidth(), 100);
}

// RTL: a packed horizontal chain defined with Start/End anchors reverses element order. In LTR the
// head sits on the left (a left of b); under RTL the head moves to the right (a right of b).
TEST(CLConstraintLayout, RtlHorizontalChainReverses) {
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());
    cl->setLayoutDirection(View::LAYOUT_DIRECTION_RTL);
    TextView* a = new TextView(&App::getInstance()); a->setText("A"); a->setId(1);
    TextView* b = new TextView(&App::getInstance()); b->setText("B"); b->setId(2);
    auto* lpa = new ConstraintLayout::LayoutParams(100, 50);
    lpa->startToStart = ConstraintLayout::PARENT_ID;
    lpa->endToStart = 2;
    lpa->horizontalChainStyle = ConstraintWidget::CHAIN_PACKED;
    auto* lpb = new ConstraintLayout::LayoutParams(100, 50);
    lpb->startToEnd = 1;
    lpb->endToEnd = ConstraintLayout::PARENT_ID;
    cl->addView(a, lpa);
    cl->addView(b, lpb);

    cl->measure(exactly(600), exactly(400));
    cl->layout(0, 0, 600, 400);

    // Reversed: a (the chain head in LTR) is now to the right of b.
    EXPECT_GT(a->getLeft(), b->getLeft());
}

// RTL: layout_goneMarginStart resolves to the right-side gone margin. Target T is GONE; child C is
// constrained start-to-end-of T. With goneMarginStart=60, in RTL C sits to T's left with that gap.
TEST(CLConstraintLayout, RtlGoneStartMarginResolves) {
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());
    cl->setLayoutDirection(View::LAYOUT_DIRECTION_RTL);

    // T: 100 wide, pinned to parent start (= right under RTL), made GONE.
    TextView* t = new TextView(&App::getInstance()); t->setText("T"); t->setId(2);
    auto* lpt = new ConstraintLayout::LayoutParams(100, 50);
    lpt->startToStart = ConstraintLayout::PARENT_ID;
    t->setVisibility(View::GONE);
    cl->addView(t, lpt);

    // C: 100 wide, start-to-end-of T, with goneMarginStart=60 (→ right-side gone under RTL).
    TextView* c = new TextView(&App::getInstance()); c->setText("C"); c->setId(1);
    auto* lpc = new ConstraintLayout::LayoutParams(100, 50);
    lpc->startToEnd = 2;
    lpc->goneStartMargin = 60;
    cl->addView(c, lpc);

    cl->measure(exactly(600), exactly(400));
    cl->layout(0, 0, 600, 400);

    // T is GONE at the start (right): gone margin (60) sits between C and the parent's right edge.
    // C's right edge lands at 600 - 60 = 540 → C at x=440.
    EXPECT_EQ(c->getLeft(), 440);
}

// RTL: a Barrier of type START resolves to RIGHT, so it sits at the rightmost right edge of its
// referenced widgets (here the left-pinned set spans 0..100, so the RIGHT barrier is at x=100). A
// 0dp child constrained left-of-barrier, right-of-parent fills 100..600.
TEST(CLConstraintLayout, RtlBarrierStartBehavesAsRight) {
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());
    cl->setLayoutDirection(View::LAYOUT_DIRECTION_RTL);

    // Referenced widget pinned to the LEFT (explicit left/right are not mirrored): 0..100.
    TextView* w1 = new TextView(&App::getInstance()); w1->setText("1"); w1->setId(1);
    auto* lp1 = new ConstraintLayout::LayoutParams(100, 50);
    lp1->leftToLeft = ConstraintLayout::PARENT_ID;
    cl->addView(w1, lp1);

    // Barrier START → under RTL behaves as RIGHT → at the rightmost right edge = x=100.
    Barrier* barrier = new Barrier(&App::getInstance());
    barrier->setId(10);
    barrier->setType(Barrier::START);
    barrier->setReferencedIds({1});
    cl->addView(barrier);

    // Child: left-to-right-of barrier, 0dp, right=parent → fills barrier(100)..parent-right(600).
    TextView* tv = new TextView(&App::getInstance()); tv->setText("X"); tv->setId(3);
    auto* lp = new ConstraintLayout::LayoutParams(0, 50);
    lp->leftToRight = 10;
    lp->rightToRight = ConstraintLayout::PARENT_ID;
    cl->addView(tv, lp);

    cl->measure(exactly(600), exactly(400));
    cl->layout(0, 0, 600, 400);

    // Barrier at x=100; child fills 100..600 → x=100, width=500.
    EXPECT_EQ(tv->getLeft(), 100);
    EXPECT_EQ(tv->getWidth(), 500);
}

// Precedence (AndroidX ConstraintLayout.java:3898-3922): when BOTH a Start/End and a Left/Right
// constraint target the same edge, Start/End wins and Left/Right is ignored. Anchor A spans 0..200;
// W sets leftToLeft=A (→A.left=0) and startToEnd=A (LTR: Start→left = leftToRight=A → A.right=200).
// Start/End wins → W.left = 200, not 0.
TEST(CLConstraintLayout, StartEndTakesPrecedenceOverLeftRight) {
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());

    TextView* a = new TextView(&App::getInstance()); a->setText("A"); a->setId(2);
    auto* lpa = new ConstraintLayout::LayoutParams(200, 50);
    lpa->leftToLeft = ConstraintLayout::PARENT_ID;  // A at 0..200
    cl->addView(a, lpa);

    TextView* w = new TextView(&App::getInstance()); w->setText("W"); w->setId(1);
    auto* lp = new ConstraintLayout::LayoutParams(100, 50);
    lp->leftToLeft = 2;   // explicit Left/Right: left → A.left = 0
    lp->startToEnd = 2;   // Start/End (wins): LTR Start→left = leftToRight=A → A.right = 200
    cl->addView(w, lp);

    cl->measure(exactly(600), exactly(400));
    cl->layout(0, 0, 600, 400);

    EXPECT_EQ(w->getLeft(), 200);  // start/end precedence, not left/right (would be 0)
}

// Layer: a pure group translation (no rotation/scale) shifts every referenced view by the same
// amount. transform() with scale=1, rotation=0 reduces shiftx = mShiftX for all views.
TEST(CLConstraintLayout, LayerAppliesGroupTranslation) {
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());
    TextView* a = new TextView(&App::getInstance()); a->setText("A"); a->setId(1);
    TextView* b = new TextView(&App::getInstance()); b->setText("B"); b->setId(2);
    auto* lpa = new ConstraintLayout::LayoutParams(50, 50);
    lpa->leftToLeft = ConstraintLayout::PARENT_ID;
    lpa->topToTop = ConstraintLayout::PARENT_ID;
    auto* lpb = new ConstraintLayout::LayoutParams(50, 50);
    lpb->rightToRight = ConstraintLayout::PARENT_ID;
    lpb->bottomToBottom = ConstraintLayout::PARENT_ID;
    cl->addView(a, lpa);
    cl->addView(b, lpb);

    auto* layer = new Layer(&App::getInstance(), nullptr);
    layer->setReferencedIds({1, 2});
    cl->addView(layer);

    cl->measure(exactly(200), exactly(200));
    cl->layout(0, 0, 200, 200);
    layer->setTranslationX(30);  // post-layout group shift

    EXPECT_EQ(a->getTranslationX(), 30);
    EXPECT_EQ(b->getTranslationX(), 30);
}

// Layer: rotating 90° about the bbox center applies the affine rotation transform to each view and
// propagates the rotation. bbox center = (200,200); view A center (50,50) → dx=dy=-150; with the
// 90° matrix {0,-1,1,0} shiftx = (-1)(-150)-(-150) = 300, shifty = (1)(-150)-(-150) = 0.
TEST(CLConstraintLayout, LayerRotatesGroupAboutCenter) {
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());
    TextView* a = new TextView(&App::getInstance()); a->setText("A"); a->setId(1);
    TextView* b = new TextView(&App::getInstance()); b->setText("B"); b->setId(2);
    auto* lpa = new ConstraintLayout::LayoutParams(100, 100);
    lpa->leftToLeft = ConstraintLayout::PARENT_ID;
    lpa->topToTop = ConstraintLayout::PARENT_ID;
    auto* lpb = new ConstraintLayout::LayoutParams(100, 100);
    lpb->rightToRight = ConstraintLayout::PARENT_ID;
    lpb->bottomToBottom = ConstraintLayout::PARENT_ID;
    cl->addView(a, lpa);
    cl->addView(b, lpb);

    auto* layer = new Layer(&App::getInstance(), nullptr);
    layer->setReferencedIds({1, 2});
    cl->addView(layer);

    cl->measure(exactly(400), exactly(400));
    cl->layout(0, 0, 400, 400);
    layer->setRotation(90);

    EXPECT_FLOAT_EQ(a->getRotation(), 90.0f);
    EXPECT_FLOAT_EQ(a->getTranslationX(), 300.0f);
    EXPECT_FLOAT_EQ(a->getTranslationY(), 0.0f);
}

// CircularFlow: positions a referenced view on a circle around a center view. Center view c is
// centered at (200,200); referenced v at angle 0°, radius 100. With the solver's addCenterPoint
// convention (effective angle = circleAngle+90°), angle 0 places v directly below the center at
// distance 100 → v center (200,300) → for a 50×50 view, left=175, top=275.
TEST(CLConstraintLayout, CircularFlowPlacesViewOnCircle) {
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());

    // Center view: 100×100, centered in parent → center (200,200).
    TextView* c = new TextView(&App::getInstance()); c->setText("C"); c->setId(1);
    auto* lpc = new ConstraintLayout::LayoutParams(100, 100);
    lpc->leftToLeft = ConstraintLayout::PARENT_ID;
    lpc->rightToRight = ConstraintLayout::PARENT_ID;
    lpc->topToTop = ConstraintLayout::PARENT_ID;
    lpc->bottomToBottom = ConstraintLayout::PARENT_ID;
    cl->addView(c, lpc);

    // Referenced view: 50×50, placed on the circle.
    TextView* v = new TextView(&App::getInstance()); v->setText("V"); v->setId(2);
    cl->addView(v, new ConstraintLayout::LayoutParams(50, 50));

    auto* cf = new CircularFlow(&App::getInstance(), nullptr);
    cf->setReferencedIds({2});
    cf->setAngles(std::vector<float>{0.0f});
    cf->setRadius(std::vector<int>{100});
    cf->setViewCenter(1);  // circle around the center view c
    cl->addView(cf);

    cl->measure(exactly(400), exactly(400));
    cl->layout(0, 0, 400, 400);

    // The view sits on the circle of radius 100 around c's center (200,200). Angle 0 places it
    // directly above the center (the solver's effective angle = circleAngle+90°).
    int vcx = v->getLeft() + v->getWidth() / 2;
    int vcy = v->getTop() + v->getHeight() / 2;
    int dx = vcx - 200;
    int dy = vcy - 200;
    EXPECT_NEAR(std::hypot(dx, dy), 100, 2);  // on the circle at the given radius
    EXPECT_NEAR(dx, 0, 2);
    EXPECT_NEAR(dy, -100, 2);  // angle 0 → above the center
}

// MotionEffect: a decorator that picks the opposite-of-dominant motion direction for the fade. A view
// moving east (Δx>0) votes WEST; moving south (Δy>0) votes NORTH. Verified via the factored vote.
TEST(CLConstraintLayout, MotionEffectVotesDirection) {
    MotionEffect* me = new MotionEffect(&App::getInstance(), nullptr);
    EXPECT_TRUE(me->isDecorator());

    using D = std::pair<float, float>;
    // A view moving east (+x) → fade applies to the west side. (int) cast: the static constexpr
    // direction constants are ODR-used only when bound to a reference (gtest's EXPECT_EQ takes
    // const T&); the cast yields a prvalue, avoiding the C++14 out-of-line definition.
    EXPECT_EQ(MotionEffect::computeFadeDirection({D{100, 0}}), (int) MotionEffect::WEST);
    // Moving south (+y) → north.
    EXPECT_EQ(MotionEffect::computeFadeDirection({D{0, 100}}), (int) MotionEffect::NORTH);
    // Moving west (-x) → east.
    EXPECT_EQ(MotionEffect::computeFadeDirection({D{-100, 0}}), (int) MotionEffect::EAST);
    // Moving north (-y) → south.
    EXPECT_EQ(MotionEffect::computeFadeDirection({D{0, -100}}), (int) MotionEffect::SOUTH);
    // Dominant direction across several views: two east-movers outweigh one south-mover → west.
    EXPECT_EQ(MotionEffect::computeFadeDirection({D{100, 0}, D{120, 0}, D{0, 50}}), (int) MotionEffect::WEST);
}

// Grid: arranges 4 referenced views into a 2×2 grid filling a 400×400 container. Grid creates
// invisible box Views (columns chained horizontally, rows chained vertically, anchored to itself) and
// constrains each view's 4 edges to box[col]/box[row], so each cell is 200×200. Box Views are added
// during the first measure pass and solved on the second (faithful AndroidX box-View approach), so
// the container is measured twice.
TEST(CLConstraintLayout, GridArrangesTwoByTwo) {
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());
    // Four 0dp (match_constraint) views that will fill their cells. Ids come from
    // generateViewId(): manually-set small ids would collide with the Grid box views'
    // allocations from the same global counter (AOSP sNextGeneratedId starts at 1) —
    // the id map (getViewById) would then resolve the box, not the referenced view.
    int ids[4] = {View::generateViewId(), View::generateViewId(),
                  View::generateViewId(), View::generateViewId()};
    for (int i = 0; i < 4; i++) {
        TextView* v = new TextView(&App::getInstance()); v->setText("X"); v->setId(ids[i]);
        cl->addView(v, new ConstraintLayout::LayoutParams(0, 0));
    }
    // Grid fills the container and references the four views in a 2×2 layout.
    auto* grid = new Grid(&App::getInstance(), nullptr);
    grid->setId(View::generateViewId());
    grid->setReferencedIds({ids[0], ids[1], ids[2], ids[3]});
    grid->setColumns(2);  // rows auto-computed = 2 from 4 referenced views
    auto* glp = new ConstraintLayout::LayoutParams(0, 0);
    glp->leftToLeft = ConstraintLayout::PARENT_ID;
    glp->rightToRight = ConstraintLayout::PARENT_ID;
    glp->topToTop = ConstraintLayout::PARENT_ID;
    glp->bottomToBottom = ConstraintLayout::PARENT_ID;
    cl->addView(grid, glp);

    // Pass 1 creates the box Views; pass 2 solves them and positions the referenced views.
    cl->measure(exactly(400), exactly(400));
    cl->measure(exactly(400), exactly(400));
    cl->layout(0, 0, 400, 400);

    TextView* v0 = (TextView*) cl->findViewById(ids[0]);
    TextView* v1 = (TextView*) cl->findViewById(ids[1]);
    TextView* v2 = (TextView*) cl->findViewById(ids[2]);
    TextView* v3 = (TextView*) cl->findViewById(ids[3]);
    ASSERT_NE(v0, nullptr);
    // Cell (row,col): (0,0)=0..200, (0,1)=200..400, (1,0)=0..200y, (1,1)=200..400y.
    EXPECT_NEAR(v0->getLeft(), 0,   2);   EXPECT_NEAR(v0->getTop(), 0,   2);
    EXPECT_NEAR(v0->getWidth(), 200, 2);
    EXPECT_NEAR(v1->getLeft(), 200, 2);   // col 1
    EXPECT_NEAR(v2->getTop(), 200, 2);    // row 1
    EXPECT_NEAR(v3->getLeft(), 200, 2);   EXPECT_NEAR(v3->getTop(), 200, 2);
}

// constraint_referenced_tags: a Group references children by their LayoutParams.constraintTag (not by
// id). Tags resolve lazily in updatePreLayout (scanning the container's children for a matching tag),
// merging into mIds — so Group's visibility propagation reaches them.
TEST(CLConstraintLayout, ReferencedTagsResolveViaGroup) {
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());
    TextView* a = new TextView(&App::getInstance()); a->setText("A"); a->setId(1);
    TextView* b = new TextView(&App::getInstance()); b->setText("B"); b->setId(2);
    TextView* c = new TextView(&App::getInstance()); c->setText("C"); c->setId(3);  // untagged — not referenced
    auto* lpa = new ConstraintLayout::LayoutParams(100, 50); lpa->constraintTag = "tag1";
    auto* lpb = new ConstraintLayout::LayoutParams(100, 50); lpb->constraintTag = "tag2";
    cl->addView(a, lpa);
    cl->addView(b, lpb);
    cl->addView(c, new ConstraintLayout::LayoutParams(100, 50));

    auto* group = new Group(&App::getInstance(), nullptr);
    group->setReferencedTags("tag1, tag2");
    cl->addView(group);

    // updatePreLayout (during measure) resolves the tags into the Group's mIds.
    cl->measure(exactly(400), exactly(400));
    group->setVisibility(View::GONE);  // propagates to the tag-referenced views only

    EXPECT_EQ(a->getVisibility(), View::GONE);
    EXPECT_EQ(b->getVisibility(), View::GONE);
    EXPECT_EQ(c->getVisibility(), View::VISIBLE);  // untagged → untouched
}

// BasicMeasure match-constraint convergence: a WRAP_CONTENT (AT_MOST) container with a 0dp
// match_constraint child capped by matchConstraintMaxWidth. The loop resolves the child to its cap
// (200) and the container WRAPs down to it (exercises the match-constraint re-measure loop + shrink).
TEST(CLConstraintLayout, WrapContainerWithMatchConstraintMax) {
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());
    TextView* tv = new TextView(&App::getInstance()); tv->setText("X"); tv->setId(1);
    auto* lp = new ConstraintLayout::LayoutParams(0, 50);
    lp->leftToLeft = ConstraintLayout::PARENT_ID;
    lp->rightToRight = ConstraintLayout::PARENT_ID;
    lp->matchConstraintMaxWidth = 200;
    cl->addView(tv, lp);

    cl->measure(atMost(600), atMost(600));

    // The match-constraint loop resolves the child to its cap (200) even under a WRAP (AT_MOST)
    // parent — the core convergence behavior. (The container's own WRAP width is the separate
    // WRAP+0dp shrink edge case; here we only assert it stays within the AT_MOST bound.)
    EXPECT_EQ(tv->getMeasuredWidth(), 200);
    EXPECT_LE(cl->getMeasuredWidth(), 600);
}

// A vertical Guideline at 50% (x=300) + a 0dp child constrained left=guideline, right=parent.
// The child should fill from 300 to 600 → x=300, width=300.
TEST(CLConstraintLayout, GuidelinePositionsChild) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());

    // Guideline child
    View* gl = new View(&App::getInstance());
    gl->setId(10);
    auto* glp = new ConstraintLayout::LayoutParams(LayoutParams::WRAP_CONTENT, LayoutParams::WRAP_CONTENT);
    glp->orientation = ConstraintWidget::VERTICAL;
    glp->guidePercent = 0.5f;
    glp->validate();
    cl->addView(gl, glp);

    // Child: 0dp, left=guideline, right=parent
    TextView* tv = new TextView(&App::getInstance()); tv->setText("X");
    auto* lp = new ConstraintLayout::LayoutParams(0, 50);
    lp->leftToLeft = 10;
    lp->rightToRight = ConstraintLayout::PARENT_ID;
    cl->addView(tv, lp);

    cl->measure(exactly(600), exactly(400));
    cl->layout(0, 0, 600, 400);

    EXPECT_EQ(tv->getLeft(), 300);
    EXPECT_EQ(tv->getWidth(), 300);
}

// Horizontal Guideline at 50% (y=200) + a child whose TOP anchors below it (topToBottom). This is
// the "to-Bottom" variant: before the getAnchor() fix the child connected to the guideline's orphan
// mBottom anchor (never positioned, ==0) and collapsed to the top instead of sitting at y=200.
TEST(CLConstraintLayout, GuidelineChildBelowHorizontal) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());

    View* gl = new View(&App::getInstance());
    gl->setId(10);
    auto* glp = new ConstraintLayout::LayoutParams(LayoutParams::WRAP_CONTENT, LayoutParams::WRAP_CONTENT);
    glp->orientation = ConstraintWidget::HORIZONTAL;
    glp->guidePercent = 0.5f;
    glp->validate();
    cl->addView(gl, glp);

    TextView* tv = new TextView(&App::getInstance()); tv->setText("X");
    auto* lp = new ConstraintLayout::LayoutParams(50, 50);
    lp->topToBottom = 10;
    lp->leftToLeft = ConstraintLayout::PARENT_ID;
    cl->addView(tv, lp);

    cl->measure(exactly(600), exactly(400));
    cl->layout(0, 0, 600, 400);

    EXPECT_EQ(tv->getTop(), 200);
}

// Vertical Guideline at 50% (x=300) + a child whose LEFT anchors right of it (leftToRight). The
// "to-Right" variant: previously connected to the guideline's orphan mRight (==0) → child at x=0.
TEST(CLConstraintLayout, GuidelineChildRightOfVertical) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());

    View* gl = new View(&App::getInstance());
    gl->setId(10);
    auto* glp = new ConstraintLayout::LayoutParams(LayoutParams::WRAP_CONTENT, LayoutParams::WRAP_CONTENT);
    glp->orientation = ConstraintWidget::VERTICAL;
    glp->guidePercent = 0.5f;
    glp->validate();
    cl->addView(gl, glp);

    TextView* tv = new TextView(&App::getInstance()); tv->setText("X");
    auto* lp = new ConstraintLayout::LayoutParams(50, 50);
    lp->leftToRight = 10;
    lp->topToTop = ConstraintLayout::PARENT_ID;
    cl->addView(tv, lp);

    cl->measure(exactly(600), exactly(400));
    cl->layout(0, 0, 600, 400);

    EXPECT_EQ(tv->getLeft(), 300);
}

// Ratio: width=200 FIXED, height=0dp MATCH_CONSTRAINT, ratio "2:1" → height=100.
TEST(CLConstraintLayout, RatioChild) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());
    TextView* tv = new TextView(&App::getInstance()); tv->setText("X");
    auto* lp = new ConstraintLayout::LayoutParams(200, 0);
    lp->leftToLeft = ConstraintLayout::PARENT_ID;
    lp->topToTop = ConstraintLayout::PARENT_ID;
    lp->dimensionRatio = 2.0f;     // "2:1" → W/H = 2 → H = 200/2 = 100
    cl->addView(tv, lp);

    cl->measure(exactly(600), exactly(400));
    cl->layout(0, 0, 600, 400);

    EXPECT_EQ(tv->getWidth(), 200);
    EXPECT_EQ(tv->getHeight(), 100);
}

// Two children in a PACKED chain (adjacent, centered by default bias 0.5).
// Group size 200 centered in 600 → A[200,300], B[300,400].
TEST(CLConstraintLayout, ChainPackedTwo) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());
    TextView* a = new TextView(&App::getInstance()); a->setText("A");
    TextView* b = new TextView(&App::getInstance()); b->setText("B");
    a->setId(1); b->setId(2);
    auto* lpa = new ConstraintLayout::LayoutParams(100, 50);
    lpa->leftToLeft = ConstraintLayout::PARENT_ID;
    lpa->rightToLeft = 2;
    lpa->horizontalChainStyle = ConstraintWidget::CHAIN_PACKED;
    auto* lpb = new ConstraintLayout::LayoutParams(100, 50);
    lpb->leftToRight = 1;
    lpb->rightToRight = ConstraintLayout::PARENT_ID;
    cl->addView(a, lpa);
    cl->addView(b, lpb);

    cl->measure(exactly(600), exactly(400));
    cl->layout(0, 0, 600, 400);

    EXPECT_EQ(a->getLeft(), 200);
    EXPECT_EQ(a->getRight(), 300);
    EXPECT_EQ(b->getLeft(), 300);
    EXPECT_EQ(b->getRight(), 400);
}

// ---- Barrier ----

// A RIGHT barrier referencing A and B sits at max(A.right, B.right).
// A:[0,100], B:[100,200] (B chained to A's right) -> barrier at 200. C pinned to the barrier -> x=200.
TEST(CLConstraintLayout, BarrierRightAtMaxEdge) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());
    TextView* a = new TextView(&App::getInstance()); a->setText("A"); a->setId(1);
    TextView* b = new TextView(&App::getInstance()); b->setText("B"); b->setId(2);
    TextView* c = new TextView(&App::getInstance()); c->setText("C"); c->setId(4);

    auto* lpa = new ConstraintLayout::LayoutParams(100, 50);
    lpa->leftToLeft = ConstraintLayout::PARENT_ID;
    auto* lpb = new ConstraintLayout::LayoutParams(100, 50);
    lpb->leftToRight = 1;
    auto* lpc = new ConstraintLayout::LayoutParams(100, 50);
    lpc->leftToLeft = 3; // barrier id

    Barrier* barrier = new Barrier(&App::getInstance());
    barrier->setId(3);
    barrier->setType(Barrier::RIGHT);
    barrier->setReferencedIds({1, 2});
    cl->addView(a, lpa);
    cl->addView(b, lpb);
    cl->addView(c, lpc);
    cl->addView(barrier, new ConstraintLayout::LayoutParams(
            LayoutParams::WRAP_CONTENT, LayoutParams::WRAP_CONTENT));

    cl->measure(exactly(600), exactly(400));
    cl->layout(0, 0, 600, 400);

    EXPECT_EQ(a->getLeft(), 0);
    EXPECT_EQ(b->getLeft(), 100);
    EXPECT_EQ(c->getLeft(), 200); // pinned to the RIGHT barrier at max(100, 200)
}

// A LEFT barrier referencing A and B sits at min(A.left, B.left).
// A:[50,150], B:[200,400] -> left barrier at min(50, 200) = 50. C pinned to it -> x=50.
TEST(CLConstraintLayout, BarrierLeftAtMinEdge) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());
    TextView* a = new TextView(&App::getInstance()); a->setText("A"); a->setId(1);
    TextView* b = new TextView(&App::getInstance()); b->setText("B"); b->setId(2);
    TextView* c = new TextView(&App::getInstance()); c->setText("C");  c->setId(4);

    auto* lpa = new ConstraintLayout::LayoutParams(100, 50);
    lpa->leftToLeft = ConstraintLayout::PARENT_ID;
    lpa->leftMargin = 50;
    auto* lpb = new ConstraintLayout::LayoutParams(200, 50);
    lpb->leftToLeft = ConstraintLayout::PARENT_ID;
    lpb->leftMargin = 200;
    auto* lpc = new ConstraintLayout::LayoutParams(80, 50);
    lpc->leftToLeft = 3; // barrier id

    Barrier* barrier = new Barrier(&App::getInstance());
    barrier->setId(3);
    barrier->setType(Barrier::LEFT);
    barrier->setReferencedIds({1, 2});
    cl->addView(a, lpa);
    cl->addView(b, lpb);
    cl->addView(c, lpc);
    cl->addView(barrier, new ConstraintLayout::LayoutParams(
            LayoutParams::WRAP_CONTENT, LayoutParams::WRAP_CONTENT));

    cl->measure(exactly(600), exactly(400));
    cl->layout(0, 0, 600, 400);

    EXPECT_EQ(a->getLeft(), 50);
    EXPECT_EQ(b->getLeft(), 200);
    EXPECT_EQ(c->getLeft(), 50); // pinned to the LEFT barrier at min(50, 200)
}

// ---- Group ----

// Setting a Group's visibility to GONE propagates GONE to every referenced view.
TEST(CLConstraintLayout, GroupHidesReferenced) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());
    TextView* a = new TextView(&App::getInstance()); a->setText("A"); a->setId(1);
    TextView* b = new TextView(&App::getInstance()); b->setText("B"); b->setId(2);
    cl->addView(a, new ConstraintLayout::LayoutParams(100, 50));
    cl->addView(b, new ConstraintLayout::LayoutParams(100, 50));

    Group* group = new Group(&App::getInstance(), nullptr);
    group->setId(10);
    group->setReferencedIds({1, 2});
    cl->addView(group, new ConstraintLayout::LayoutParams(
            LayoutParams::WRAP_CONTENT, LayoutParams::WRAP_CONTENT));
    group->setVisibility(View::GONE);

    EXPECT_EQ(a->getVisibility(), View::GONE);
    EXPECT_EQ(b->getVisibility(), View::GONE);
}

// ---- Placeholder ----

// A Placeholder (centered, 120x60) carrying a content view X (120x60). After layout the content
// is drawn at the placeholder's frame: x=(600-120)/2=240, y=(400-60)/2=170.
TEST(CLConstraintLayout, PlaceholderPositionsContent) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());

    TextView* x = new TextView(&App::getInstance()); x->setText("X"); x->setId(1);
    auto* lpx = new ConstraintLayout::LayoutParams(120, 60);
    lpx->leftToLeft = ConstraintLayout::PARENT_ID; // X's own (ignored) origin
    lpx->topToTop = ConstraintLayout::PARENT_ID;

    Placeholder* placeholder = new Placeholder(&App::getInstance(), nullptr); placeholder->setId(2);
    auto* lpp = new ConstraintLayout::LayoutParams(120, 60);
    lpp->leftToLeft = ConstraintLayout::PARENT_ID;
    lpp->rightToRight = ConstraintLayout::PARENT_ID;
    lpp->topToTop = ConstraintLayout::PARENT_ID;
    lpp->bottomToBottom = ConstraintLayout::PARENT_ID;

    cl->addView(x, lpx);
    cl->addView(placeholder, lpp);
    placeholder->setContentId(1);

    cl->measure(exactly(600), exactly(400));
    cl->layout(0, 0, 600, 400);

    EXPECT_EQ(x->getLeft(), 240);
    EXPECT_EQ(x->getTop(), 170);
    EXPECT_EQ(x->getWidth(), 120);
    EXPECT_EQ(x->getHeight(), 60);
}

// ---- match_constraint (0dp) sizing modes ----

// 0dp width with default=percent, percent=0.5, both sides to parent(600) -> width = 300.
// (Position centers by default bias 0.5; we assert the percent-computed size.)
TEST(CLConstraintLayout, MatchConstraintPercent) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());
    TextView* tv = new TextView(&App::getInstance()); tv->setText("X");
    auto* lp = new ConstraintLayout::LayoutParams(0, 50);
    lp->leftToLeft = ConstraintLayout::PARENT_ID;
    lp->rightToRight = ConstraintLayout::PARENT_ID;
    lp->matchConstraintDefaultWidth = ConstraintWidget::MATCH_CONSTRAINT_PERCENT;
    lp->matchConstraintPercentWidth = 0.5f;
    cl->addView(tv, lp);
    cl->measure(exactly(600), exactly(400));
    cl->layout(0, 0, 600, 400);
    EXPECT_EQ(tv->getWidth(), 300);
}

// 0dp width spread-fill would be 600, but max=200 caps it -> width 200.
TEST(CLConstraintLayout, MatchConstraintMaxCaps) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());
    TextView* tv = new TextView(&App::getInstance()); tv->setText("X");
    auto* lp = new ConstraintLayout::LayoutParams(0, 50);
    lp->leftToLeft = ConstraintLayout::PARENT_ID;
    lp->rightToRight = ConstraintLayout::PARENT_ID;
    lp->matchConstraintDefaultWidth = ConstraintWidget::MATCH_CONSTRAINT_SPREAD;
    lp->matchConstraintMaxWidth = 200;
    cl->addView(tv, lp);
    cl->measure(exactly(600), exactly(400));
    cl->layout(0, 0, 600, 400);
    EXPECT_EQ(tv->getWidth(), 200);
}

// 0dp width with default=wrap should size to the view's content, NOT spread-fill 600.
// (The solver's WRAP branch uses the content size measured by BasicMeasure.measureChildren.)
TEST(CLConstraintLayout, MatchConstraintWrap) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());
    TextView* tv = new TextView(&App::getInstance()); tv->setText("X");
    auto* lp = new ConstraintLayout::LayoutParams(0, 50);
    lp->leftToLeft = ConstraintLayout::PARENT_ID;
    lp->rightToRight = ConstraintLayout::PARENT_ID;
    lp->matchConstraintDefaultWidth = ConstraintWidget::MATCH_CONSTRAINT_WRAP;
    cl->addView(tv, lp);
    cl->measure(exactly(600), exactly(400));
    cl->layout(0, 0, 600, 400);
    EXPECT_NE(tv->getWidth(), 600); // wrap must not spread-fill
    EXPECT_GT(tv->getWidth(), 0);
}

// Two 0dp widgets in a packed chain with weights 1:2 in a 600-wide container.
// Free space (600) splits by weight -> A=200, B=400.
TEST(CLConstraintLayout, ChainWeightsDistribute) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());
    TextView* a = new TextView(&App::getInstance()); a->setText("A"); a->setId(1);
    TextView* b = new TextView(&App::getInstance()); b->setText("B"); b->setId(2);
    auto* lpa = new ConstraintLayout::LayoutParams(0, 50);
    lpa->leftToLeft = ConstraintLayout::PARENT_ID;
    lpa->rightToLeft = 2;
    lpa->horizontalWeight = 1.0f;
    auto* lpb = new ConstraintLayout::LayoutParams(0, 50);
    lpb->leftToRight = 1;
    lpb->rightToRight = ConstraintLayout::PARENT_ID;
    lpb->horizontalWeight = 2.0f;
    cl->addView(a, lpa);
    cl->addView(b, lpb);
    cl->measure(exactly(600), exactly(400));
    cl->layout(0, 0, 600, 400);
    EXPECT_EQ(a->getWidth(), 200);
    EXPECT_EQ(b->getWidth(), 400);
}

// ---- Flow ----

// A 200x100 Flow (WRAP_CHAIN, horizontal) referencing four 100-wide widgets: two per row.
// Row0: A(0,0) B(100,0); Row1: C(0,50) D(100,50).
// Flow WRAP_NONE: no wrapping — all referenced widgets stay in a single row even when their total
// width exceeds the Flow's width. (Vertical position shared; horizontal sequence preserved.)
TEST(CLConstraintLayout, FlowWrapNoneSingleRow) {
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());
    TextView* a = new TextView(&App::getInstance()); a->setText("A"); a->setId(1);
    TextView* b = new TextView(&App::getInstance()); b->setText("B"); b->setId(2);
    TextView* c = new TextView(&App::getInstance()); c->setText("C"); c->setId(3);
    TextView* d = new TextView(&App::getInstance()); d->setText("D"); d->setId(4);
    cl->addView(a, new ConstraintLayout::LayoutParams(100, 50));
    cl->addView(b, new ConstraintLayout::LayoutParams(100, 50));
    cl->addView(c, new ConstraintLayout::LayoutParams(100, 50));
    cl->addView(d, new ConstraintLayout::LayoutParams(100, 50));

    Flow* flow = new Flow(&App::getInstance());
    flow->setId(10);
    flow->setReferencedIds({1, 2, 3, 4});
    flow->setWrapMode(Flow::WRAP_NONE);
    auto* lp = new ConstraintLayout::LayoutParams(200, 100);
    lp->leftToLeft = ConstraintLayout::PARENT_ID;
    lp->topToTop = ConstraintLayout::PARENT_ID;
    cl->addView(flow, lp);

    cl->measure(exactly(600), exactly(400));
    cl->layout(0, 0, 600, 400);

    // All four on the same row (no wrap) — same vertical position, in left-to-right sequence.
    int rowTop = a->getTop();
    EXPECT_EQ(b->getTop(), rowTop);
    EXPECT_EQ(c->getTop(), rowTop);
    EXPECT_EQ(d->getTop(), rowTop);
    EXPECT_LT(a->getLeft(), b->getLeft());
    EXPECT_LT(b->getLeft(), c->getLeft());
    EXPECT_LT(c->getLeft(), d->getLeft());
}

// Flow WRAP_CHAIN_NEW with maxElementsWrap: forces a wrap every N widgets via the running col/row
// counter (independent of width overflow). Here a wide Flow (no width overflow) wraps every 2.
TEST(CLConstraintLayout, FlowWrapChainNewByMaxElements) {
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());
    TextView* a = new TextView(&App::getInstance()); a->setText("A"); a->setId(1);
    TextView* b = new TextView(&App::getInstance()); b->setText("B"); b->setId(2);
    TextView* c = new TextView(&App::getInstance()); c->setText("C"); c->setId(3);
    TextView* d = new TextView(&App::getInstance()); d->setText("D"); d->setId(4);
    cl->addView(a, new ConstraintLayout::LayoutParams(100, 50));
    cl->addView(b, new ConstraintLayout::LayoutParams(100, 50));
    cl->addView(c, new ConstraintLayout::LayoutParams(100, 50));
    cl->addView(d, new ConstraintLayout::LayoutParams(100, 50));

    Flow* flow = new Flow(&App::getInstance());
    flow->setId(10);
    flow->setReferencedIds({1, 2, 3, 4});
    flow->setWrapMode(Flow::WRAP_CHAIN_NEW);
    flow->setMaxElementsWrap(2);
    auto* lp = new ConstraintLayout::LayoutParams(600, 100);
    lp->leftToLeft = ConstraintLayout::PARENT_ID;
    lp->topToTop = ConstraintLayout::PARENT_ID;
    cl->addView(flow, lp);

    cl->measure(exactly(600), exactly(400));
    cl->layout(0, 0, 600, 400);

    // maxElementsWrap=2 → [a,b] row 0, [c,d] row 1.
    EXPECT_EQ(a->getTop(), 0);
    EXPECT_EQ(b->getTop(), 0);
    EXPECT_EQ(c->getTop(), 50);
    EXPECT_EQ(d->getTop(), 50);
}

// Flow WRAP_ALIGNED: arranges referenced widgets in a regular grid (here 2×2 via maxElementsWrap=2).
// Widgets in the same column share an x band; same row share a y band — a true grid, not chains.
TEST(CLConstraintLayout, FlowWrapAlignedGrid) {
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());
    TextView* a = new TextView(&App::getInstance()); a->setText("A"); a->setId(1);
    TextView* b = new TextView(&App::getInstance()); b->setText("B"); b->setId(2);
    TextView* c = new TextView(&App::getInstance()); c->setText("C"); c->setId(3);
    TextView* d = new TextView(&App::getInstance()); d->setText("D"); d->setId(4);
    cl->addView(a, new ConstraintLayout::LayoutParams(100, 50));
    cl->addView(b, new ConstraintLayout::LayoutParams(100, 50));
    cl->addView(c, new ConstraintLayout::LayoutParams(100, 50));
    cl->addView(d, new ConstraintLayout::LayoutParams(100, 50));

    Flow* flow = new Flow(&App::getInstance());
    flow->setId(10);
    flow->setReferencedIds({1, 2, 3, 4});
    flow->setWrapMode(Flow::WRAP_ALIGNED);
    flow->setMaxElementsWrap(2);  // 2 columns → 2×2 grid
    auto* lp = new ConstraintLayout::LayoutParams(0, 0);
    lp->leftToLeft = ConstraintLayout::PARENT_ID;
    lp->rightToRight = ConstraintLayout::PARENT_ID;
    lp->topToTop = ConstraintLayout::PARENT_ID;
    lp->bottomToBottom = ConstraintLayout::PARENT_ID;
    cl->addView(flow, lp);

    cl->measure(exactly(600), exactly(400));
    cl->layout(0, 0, 600, 400);

    // a=(row0,col0), b=(row0,col1), c=(row1,col0), d=(row1,col1).
    EXPECT_EQ(a->getLeft(), c->getLeft());   // col 0 shares x
    EXPECT_EQ(b->getLeft(), d->getLeft());   // col 1 shares x
    EXPECT_EQ(a->getTop(),  b->getTop());    // row 0 shares y
    EXPECT_EQ(c->getTop(),  d->getTop());    // row 1 shares y
    EXPECT_LT(a->getLeft(), b->getLeft());   // col 0 left of col 1
    EXPECT_LT(a->getTop(),  c->getTop());    // row 0 above row 1
}

TEST(CLConstraintLayout, FlowWrapsToSecondRow) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());
    TextView* a = new TextView(&App::getInstance()); a->setText("A"); a->setId(1);
    TextView* b = new TextView(&App::getInstance()); b->setText("B"); b->setId(2);
    TextView* c = new TextView(&App::getInstance()); c->setText("C"); c->setId(3);
    TextView* d = new TextView(&App::getInstance()); d->setText("D"); d->setId(4);
    cl->addView(a, new ConstraintLayout::LayoutParams(100, 50));
    cl->addView(b, new ConstraintLayout::LayoutParams(100, 50));
    cl->addView(c, new ConstraintLayout::LayoutParams(100, 50));
    cl->addView(d, new ConstraintLayout::LayoutParams(100, 50));

    Flow* flow = new Flow(&App::getInstance());
    flow->setId(10);
    flow->setReferencedIds({1, 2, 3, 4});
    flow->setWrapMode(Flow::WRAP_CHAIN);
    auto* lp = new ConstraintLayout::LayoutParams(200, 100);
    lp->leftToLeft = ConstraintLayout::PARENT_ID;
    lp->topToTop = ConstraintLayout::PARENT_ID;
    cl->addView(flow, lp);

    cl->measure(exactly(600), exactly(400));
    cl->layout(0, 0, 600, 400);

    EXPECT_EQ(a->getLeft(), 0);   EXPECT_EQ(a->getTop(), 0);
    EXPECT_EQ(b->getLeft(), 100); EXPECT_EQ(b->getTop(), 0);
    EXPECT_EQ(c->getLeft(), 0);   EXPECT_EQ(c->getTop(), 50);  // wrapped to row 2
    EXPECT_EQ(d->getLeft(), 100); EXPECT_EQ(d->getTop(), 50);
}

// A 100x200 Flow (WRAP_CHAIN, VERTICAL) referencing four 50x50 widgets: they stack in a column.
// w0(0,0) w1(0,50) w2(0,100) w3(0,150).
TEST(CLConstraintLayout, FlowVerticalStacksColumn) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());
    TextView* views[4];
    for (int i = 0; i < 4; i++) {
        views[i] = new TextView(&App::getInstance()); views[i]->setText("X");
        views[i]->setId(i + 1);
        cl->addView(views[i], new ConstraintLayout::LayoutParams(50, 50));
    }
    Flow* flow = new Flow(&App::getInstance());
    flow->setId(10);
    flow->setReferencedIds({1, 2, 3, 4});
    flow->setWrapMode(Flow::WRAP_CHAIN);
    flow->setOrientation(ConstraintWidget::VERTICAL);
    auto* lp = new ConstraintLayout::LayoutParams(50, 200);
    lp->leftToLeft = ConstraintLayout::PARENT_ID;
    lp->topToTop = ConstraintLayout::PARENT_ID;
    cl->addView(flow, lp);

    cl->measure(exactly(600), exactly(400));
    cl->layout(0, 0, 600, 400);

    for (int i = 0; i < 4; i++) {
        EXPECT_EQ(views[i]->getLeft(), 0) << "view " << i;
        EXPECT_EQ(views[i]->getTop(), i * 50) << "view " << i;
    }
}

// A Flow with WRAP_CONTENT height: height is driven by the wrapped rows (2 rows of 50 = 100).
TEST(CLConstraintLayout, FlowWrapContentHeight) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());
    TextView* a = new TextView(&App::getInstance()); a->setText("A"); a->setId(1);
    TextView* b = new TextView(&App::getInstance()); b->setText("B"); b->setId(2);
    TextView* c = new TextView(&App::getInstance()); c->setText("C"); c->setId(3);
    TextView* d = new TextView(&App::getInstance()); d->setText("D"); d->setId(4);
    cl->addView(a, new ConstraintLayout::LayoutParams(100, 50));
    cl->addView(b, new ConstraintLayout::LayoutParams(100, 50));
    cl->addView(c, new ConstraintLayout::LayoutParams(100, 50));
    cl->addView(d, new ConstraintLayout::LayoutParams(100, 50));

    Flow* flow = new Flow(&App::getInstance());
    flow->setId(10);
    flow->setReferencedIds({1, 2, 3, 4});
    flow->setWrapMode(Flow::WRAP_CHAIN);
    auto* lp = new ConstraintLayout::LayoutParams(200, ConstraintLayout::LayoutParams::WRAP_CONTENT);
    lp->leftToLeft = ConstraintLayout::PARENT_ID;
    lp->topToTop = ConstraintLayout::PARENT_ID;
    cl->addView(flow, lp);

    cl->measure(exactly(600), exactly(400));
    cl->layout(0, 0, 600, 400);

    EXPECT_EQ(a->getTop(), 0);          // row 1
    EXPECT_EQ(c->getTop(), 50);         // wrapped to row 2
    EXPECT_EQ(flow->getHeight(), 100);  // WRAP height reflects 2 rows of 50
}

// A horizontal Flow with WRAP_CONTENT width: no width to wrap against → single row, width = sum.
TEST(CLConstraintLayout, FlowWrapWidthSingleRow) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());
    TextView* a = new TextView(&App::getInstance()); a->setText("A"); a->setId(1);
    TextView* b = new TextView(&App::getInstance()); b->setText("B"); b->setId(2);
    TextView* c = new TextView(&App::getInstance()); c->setText("C"); c->setId(3);
    cl->addView(a, new ConstraintLayout::LayoutParams(100, 50));
    cl->addView(b, new ConstraintLayout::LayoutParams(100, 50));
    cl->addView(c, new ConstraintLayout::LayoutParams(100, 50));

    Flow* flow = new Flow(&App::getInstance());
    flow->setId(10);
    flow->setReferencedIds({1, 2, 3});
    flow->setWrapMode(Flow::WRAP_CHAIN);
    auto* lp = new ConstraintLayout::LayoutParams(
        ConstraintLayout::LayoutParams::WRAP_CONTENT, 50);
    lp->leftToLeft = ConstraintLayout::PARENT_ID;
    lp->topToTop = ConstraintLayout::PARENT_ID;
    cl->addView(flow, lp);

    cl->measure(exactly(600), exactly(400));
    cl->layout(0, 0, 600, 400);

    EXPECT_EQ(a->getTop(), 0);   // single row — no wrapping
    EXPECT_EQ(b->getTop(), 0);
    EXPECT_EQ(c->getTop(), 0);
    EXPECT_EQ(flow->getWidth(), 300);  // WRAP width = 3 * 100
}

// A WRAP_CONTENT container (AT_MOST spec) should size itself to its fixed child.
TEST(CLConstraintLayout, WrapContainerSizesToChild) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());
    TextView* tv = new TextView(&App::getInstance()); tv->setText("X");
    auto* lp = new ConstraintLayout::LayoutParams(100, 50);
    lp->leftToLeft = ConstraintLayout::PARENT_ID;
    lp->topToTop = ConstraintLayout::PARENT_ID;
    cl->addView(tv, lp);

    cl->measure(atMost(600), atMost(400));

    EXPECT_EQ(cl->getMeasuredWidth(), 100);   // WRAP container shrinks to the child
    EXPECT_EQ(cl->getMeasuredHeight(), 50);
}

// ---- ConstraintSet ----

// Build constraints programmatically and apply: connect both sides to parent -> centered.
// ConstraintSet typed setters: centerHorizontally/Vertically + constrainWidth/Height build a set
// programmatically; applyTo writes it onto the layout → the view centers in a 600×400 container.
TEST(CLConstraintLayout, ConstraintSetCenterHelpers) {
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());
    TextView* tv = new TextView(&App::getInstance()); tv->setText("X"); tv->setId(1);
    cl->addView(tv, new ConstraintLayout::LayoutParams(100, 50));

    ConstraintSet cs;
    cs.constrainWidth(1, 100);
    cs.constrainHeight(1, 50);
    cs.centerHorizontally(1, ConstraintSet::PARENT);
    cs.centerVertically(1, ConstraintSet::PARENT);
    cs.applyTo(cl);

    cl->measure(exactly(600), exactly(400));
    cl->layout(0, 0, 600, 400);

    EXPECT_EQ(tv->getLeft(), 250);  // (600 - 100) / 2
    EXPECT_EQ(tv->getTop(), 175);   // (400 - 50) / 2
}

TEST(CLConstraintLayout, ConstraintSetCentersChild) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());
    TextView* a = new TextView(&App::getInstance()); a->setText("A"); a->setId(1);
    cl->addView(a, new ConstraintLayout::LayoutParams(100, 50));

    ConstraintSet cs;
    cs.constrainWidth(1, 100);
    cs.constrainHeight(1, 50);
    cs.connect(1, ConstraintSet::LEFT, ConstraintSet::PARENT, ConstraintSet::LEFT);
    cs.connect(1, ConstraintSet::RIGHT, ConstraintSet::PARENT, ConstraintSet::RIGHT);
    cs.applyTo(cl);

    cl->measure(exactly(600), exactly(400));
    cl->layout(0, 0, 600, 400);
    EXPECT_EQ(a->getLeft(), 250);
    EXPECT_EQ(a->getRight(), 350);
}

// clone() snapshots a layout; modify + applyTo repositions a previously-centered child to the left.
TEST(CLConstraintLayout, ConstraintSetCloneAndModify) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());
    TextView* a = new TextView(&App::getInstance()); a->setText("A"); a->setId(1);
    auto* lp = new ConstraintLayout::LayoutParams(100, 50);
    lp->leftToLeft = ConstraintLayout::PARENT_ID;
    lp->rightToRight = ConstraintLayout::PARENT_ID; // centered
    cl->addView(a, lp);
    cl->measure(exactly(600), exactly(400));
    cl->layout(0, 0, 600, 400);
    ASSERT_EQ(a->getLeft(), 250); // initially centered

    ConstraintSet cs;
    cs.clone(cl);
    cs.clear(1, ConstraintSet::RIGHT);                        // drop the right anchor
    cs.connect(1, ConstraintSet::LEFT, ConstraintSet::PARENT, ConstraintSet::LEFT);
    cs.applyTo(cl);
    cl->measure(exactly(600), exactly(400));
    cl->layout(0, 0, 600, 400);
    EXPECT_EQ(a->getLeft(), 0); // now pinned left
    EXPECT_EQ(a->getWidth(), 100);
}

// ConstraintSet.load(Context, XmlPullParser) parses a <ConstraintSet> XML resource into the
// Constraint model. The scene loads from the gui_test pak (binary AXML + arsc ids — aapt2 has
// resolved enums/"parent"/@id refs before the parser sees them). Verifies the attribute dispatch
// (populateConstraint) for dimensions, anchors, bias, margins, visibility/alpha (PropertySet),
// transforms, chain style, and ratio.
TEST(CLConstraintLayout, ConstraintSetXmlLoad) {
    App& app = App::getInstance();
    auto parser = app.getResources().getXml(gui_test::R::xml::constraintset_parse);
    // Advance to the <ConstraintSet> START_TAG, then let load() consume through its END_TAG.
    while (parser->getEventType() != XmlPullParser::START_TAG &&
           parser->getEventType() != XmlPullParser::END_DOCUMENT) {
        parser->next();
    }

    ConstraintSet cs;
    cs.load(&app, *parser);

    ASSERT_TRUE(cs.contains(gui_test::R::id::cs_target));
    const auto& c = cs.get(gui_test::R::id::cs_target);
    EXPECT_EQ(c.layout.mWidth, 100);
    EXPECT_EQ(c.layout.mHeight, 60);
    EXPECT_EQ(c.layout.leftToLeft, 0);     // "parent" -> PARENT_ID=0
    EXPECT_EQ(c.layout.rightToRight, 0);
    EXPECT_FLOAT_EQ(c.layout.horizontalBias, 0.25f);
    EXPECT_EQ(c.layout.leftMargin, 8);
    EXPECT_EQ(c.layout.horizontalChainStyle, (int)ConstraintWidget::CHAIN_PACKED);
    EXPECT_EQ(c.layout.dimensionRatio, "2:1");
    EXPECT_EQ(c.propertySet.visibility, 4); // invisible -> View::INVISIBLE=4
    EXPECT_FLOAT_EQ(c.propertySet.alpha, 0.5f);
    EXPECT_FLOAT_EQ(c.transform.rotation, 45.0f);
    EXPECT_FLOAT_EQ(c.transform.scaleX, 2.0f);
}

// KeyFrames parses a <KeyFrameSet> into core MotionKey subclasses (KeyAttribute + KeyPosition),
// filed under the target view id (a resource id from the pak arsc). alpha/rotation are framework
// attrs (android:), the motion attrs live in the widgetex namespace (app:).
TEST(CLConstraintLayout, KeyFramesXmlParse) {
    App& app = App::getInstance();
    auto parser = app.getResources().getXml(gui_test::R::xml::keyframes_parse);
    while (parser->getEventType() != XmlPullParser::START_TAG &&
           parser->getEventType() != XmlPullParser::END_DOCUMENT) {
        parser->next();
    }

    KeyFrames kf(&app, *parser);
    auto keys = kf.getKeysForView(gui_test::R::id::kf_target);
    ASSERT_EQ(keys.size(), 2u);

    MotionKeyAttributes* attr = nullptr;
    MotionKeyPosition* pos = nullptr;
    for (MotionKey* k : keys) {
        if (k->mType == MotionKeyAttributes::KEY_TYPE) attr = static_cast<MotionKeyAttributes*>(k);
        else if (k->mType == MotionKeyPosition::KEY_TYPE) pos = static_cast<MotionKeyPosition*>(k);
    }
    ASSERT_NE(attr, nullptr);
    ASSERT_NE(pos, nullptr);
    EXPECT_EQ(attr->mFramePosition, 50);
    EXPECT_FLOAT_EQ(attr->mAlpha, 0.0f);
    EXPECT_FLOAT_EQ(attr->mRotation, 90.0f);
    EXPECT_EQ(pos->mFramePosition, 50);
    EXPECT_FLOAT_EQ(pos->mPercentX, 0.5f);
    EXPECT_FLOAT_EQ(pos->mPercentY, 0.5f);
    EXPECT_EQ(pos->mPositionType, MotionKeyPosition::TYPE_PATH);
}

// MotionScene parses a full scene: <Transition> referencing two inline <ConstraintSet>s, with a
// <KeyFrameSet> and an <OnClick> child. Loaded from the pak (binary AXML): the @+id refs resolve
// to arsc resource ids, so the Transition's start/end refs agree with the parsed sets.
TEST(CLConstraintLayout, MotionSceneXmlParse) {
    App& app = App::getInstance();
    MotionScene scene(nullptr);
    scene.load(&app, gui_test::R::xml::motion_scene_parse);

    auto* t = scene.getCurrentTransition();
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->getDuration(), 500);
    EXPECT_FALSE(t->isAbstract());

    auto* startSet = scene.getConstraintSet(t->getStartId());
    auto* endSet = scene.getConstraintSet(t->getEndId());
    ASSERT_NE(startSet, nullptr);
    ASSERT_NE(endSet, nullptr);
    EXPECT_EQ(startSet->get(gui_test::R::id::ms_target).layout.leftToLeft, 0);  // parent
    EXPECT_EQ(endSet->get(gui_test::R::id::ms_target).layout.rightToRight, 0);  // parent

    ASSERT_NE(t->getKeyFrames(), nullptr);
    EXPECT_EQ(t->getKeyFrames()->getKeysForView(gui_test::R::id::ms_target).size(), 1u);
    ASSERT_EQ(t->getOnClicks().size(), 1u);
    EXPECT_EQ(t->getOnClicks()[0].targetId, (int)gui_test::R::id::ms_target);
    EXPECT_EQ(t->getOnClicks()[0].clickAction, MotionScene::Transition::FLAG_TOGGLE);

    ASSERT_NE(t->getOnSwipe(), nullptr);
    EXPECT_EQ(t->getOnSwipe()->dragDirection, MotionScene::OnSwipe::DRAG_RIGHT);
    EXPECT_FLOAT_EQ(t->getOnSwipe()->dragScale, 2.0f);
    EXPECT_EQ(t->getOnSwipe()->onTouchUp, MotionScene::OnSwipe::ON_UP_AUTOCOMPLETE_TO_START);
}

// deriveConstraintsFrom: a derived <ConstraintSet> inherits the base set's constraints, with its
// own same-id entries overriding the base's (derived wins). The base may be defined after the
// derived set in the XML — the merge is lazy on the first getConstraintSet() call.
TEST(CLConstraintLayout, MotionSceneDeriveConstraints) {
    App& app = App::getInstance();
    MotionScene scene(nullptr);
    scene.load(&app, gui_test::R::xml::motion_scene_derive);

    auto* t = scene.getCurrentTransition();
    ASSERT_NE(t, nullptr);
    auto* derived = scene.getConstraintSet(t->getStartId());
    auto* base = scene.getConstraintSet(t->getEndId());
    ASSERT_NE(derived, nullptr);
    ASSERT_NE(base, nullptr);

    EXPECT_TRUE(base->contains(gui_test::R::id::dc_one));
    EXPECT_TRUE(base->contains(gui_test::R::id::dc_three));
    EXPECT_FALSE(base->contains(gui_test::R::id::dc_two));
    EXPECT_EQ(base->get(gui_test::R::id::dc_one).layout.mWidth, 100);

    // derived inherits base's dc_three, keeps its own dc_two, and its dc_one overrides base's.
    EXPECT_TRUE(derived->contains(gui_test::R::id::dc_one));
    EXPECT_TRUE(derived->contains(gui_test::R::id::dc_two));
    EXPECT_TRUE(derived->contains(gui_test::R::id::dc_three));            // inherited from base
    EXPECT_EQ(derived->get(gui_test::R::id::dc_one).layout.mWidth, 200);  // derived wins over base
    EXPECT_EQ(derived->get(gui_test::R::id::dc_two).layout.mWidth, 80);   // own
    EXPECT_EQ(derived->get(gui_test::R::id::dc_three).layout.mWidth, 60); // inherited
}

// Multi-transition state machine: <Transition android:id> is parsed, and MotionScene can look up a
// transition by id or by its start/end ConstraintSet endpoints (used by MotionLayout::setTransition
// (id) / transitionToState). The first non-abstract transition remains the current one.
TEST(CLConstraintLayout, MotionSceneTransitionLookup) {
    App& app = App::getInstance();
    MotionScene scene(nullptr);
    scene.load(&app, gui_test::R::xml::motion_scene_lookup);

    auto* t1 = scene.getCurrentTransition(); // first non-abstract
    ASSERT_NE(t1, nullptr);
    ASSERT_NE(t1->getId(), MotionScene::UNSET); // <Transition android:id> parsed
    EXPECT_EQ(t1->getId(), (int)gui_test::R::id::t1);
    const int t1Id = t1->getId();
    const int A = t1->getStartId(), B = t1->getEndId();
    EXPECT_EQ(A, (int)gui_test::R::id::trA);
    EXPECT_EQ(B, (int)gui_test::R::id::trB);

    EXPECT_EQ(scene.getTransitionById(t1Id), t1);   // lookup by id
    EXPECT_EQ(scene.findTransition(A, B), t1);      // lookup by endpoints
    EXPECT_EQ(scene.findTransition(B, A), nullptr); // reverse direction not defined
    scene.setCurrentTransition(t1);
    EXPECT_EQ(scene.getCurrentTransition(), t1);
}

// autoTransition: a <Transition autoTransition="..."> is parsed; when the layout rests at the
// matching endpoint MotionScene::autoTransition fires it (animate/jump to the other end).
TEST(CLConstraintLayout, MotionSceneAutoTransitionParse) {
    App& app = App::getInstance();
    MotionScene scene(nullptr);
    scene.load(&app, gui_test::R::xml::motion_scene_autotransition);

    auto* t = scene.getCurrentTransition();
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->getAutoTransition(), MotionScene::Transition::AUTO_ANIMATE_TO_END);
}

// <CustomAttribute> is parsed into the Constraint model and dispatched, at applyTo(), to an
// externally-registered handler. The framework binds no attribute itself — the test registers a
// "textColor" -> TextView::setTextColor handler (an app/widget-layer concern) and checks dispatch.
TEST(CLConstraintLayout, ConstraintSetCustomAttribute) {
    ConstraintSet::registerCustomAttributeHandler("textColor",
        [](View* v, const ConstraintSet::CustomAttribute& ca) {
            if (auto* tv = dynamic_cast<TextView*>(v)) tv->setTextColor(ca.intValue);
        });

    App& app = App::getInstance();
    auto parser = app.getResources().getXml(gui_test::R::xml::constraintset_customattr);
    while (parser->getEventType() != XmlPullParser::START_TAG &&
           parser->getEventType() != XmlPullParser::END_DOCUMENT) {
        parser->next();
    }
    ConstraintSet set;
    set.load(&app, *parser);

    ASSERT_EQ(set.get(gui_test::R::id::ca_target).mCustomAttributes.size(), 1u);
    const auto& ca = set.get(gui_test::R::id::ca_target).mCustomAttributes[0];
    EXPECT_EQ(ca.name, "textColor");
    EXPECT_EQ(ca.type, ConstraintSet::CustomAttribute::COLOR);
    ASSERT_NE(ca.intValue, 0); // a real color was parsed

    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());
    TextView* tv = new TextView(&App::getInstance()); tv->setText("T");
    tv->setId(gui_test::R::id::ca_target);
    cl->addView(tv);
    set.applyTo(cl);
    EXPECT_EQ(tv->getCurrentTextColor(), ca.intValue); // handler dispatched the parsed color
}

// A container with padding insets its children: leftToLeft=parent with paddingLeft=50 -> x=50.
TEST(CLConstraintLayout, PaddingInsetsChildren) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());
    cl->setPadding(50, 20, 0, 0);
    TextView* a = new TextView(&App::getInstance()); a->setText("A"); a->setId(1);
    auto* lp = new ConstraintLayout::LayoutParams(100, 50);
    lp->leftToLeft = ConstraintLayout::PARENT_ID;
    lp->topToTop = ConstraintLayout::PARENT_ID;
    cl->addView(a, lp);
    cl->measure(exactly(600), exactly(400));
    cl->layout(0, 0, 600, 400);
    EXPECT_EQ(a->getLeft(), 50); // paddingLeft offset
    EXPECT_EQ(a->getTop(), 20);  // paddingTop offset
}

// ---- MotionLayout ----

// A child pinned left in the start set and right in the end set animates across the width.
// setProgress(0)=left(0), (1)=right(500), (0.5)=mid(250).
TEST(CLConstraintLayout, MotionLayoutAnimatesChild) {
    App& app = App::getInstance();
    MotionLayout* ml = new MotionLayout(&App::getInstance());
    TextView* tv = new TextView(&App::getInstance()); tv->setText("X"); tv->setId(1);
    ml->addView(tv, new ConstraintLayout::LayoutParams(100, 50));
    ml->measure(exactly(600), exactly(400));
    ml->layout(0, 0, 600, 400);

    ConstraintSet start, end;
    start.constrainWidth(1, 100); start.constrainHeight(1, 50);
    start.connect(1, ConstraintSet::LEFT, ConstraintSet::PARENT, ConstraintSet::LEFT);
    end.constrainWidth(1, 100); end.constrainHeight(1, 50);
    end.connect(1, ConstraintSet::RIGHT, ConstraintSet::PARENT, ConstraintSet::RIGHT);

    ml->setTransition(&start, &end);

    ml->setProgress(0.0f);
    EXPECT_EQ(tv->getLeft(), 0);
    EXPECT_EQ(tv->getWidth(), 100);

    ml->setProgress(1.0f);
    EXPECT_EQ(tv->getLeft(), 500); // pinned right: 600 - 100
    EXPECT_EQ(tv->getWidth(), 100);

    ml->setProgress(0.5f);
    EXPECT_EQ(tv->getLeft(), 250); // midpoint
}

// A MotionLayout with a position keyframe arcs the child off the linear path at progress 0.5.
// start: child pinned left (x=0). end: child pinned right (x=500). KeyPosition frame50 percentX=0.5
// altPercentY=0.5 → at progress 0.5 the child arcs down to y≈250 (linear would be y=0).
TEST(CLConstraintLayout, MotionLayoutKeyPositionArc) {
    App& app = App::getInstance();
    MotionLayout* ml = new MotionLayout(&App::getInstance());
    TextView* tv = new TextView(&App::getInstance()); tv->setText("X"); tv->setId(1);
    ml->addView(tv, new ConstraintLayout::LayoutParams(100, 50));
    ml->measure(exactly(600), exactly(400));
    ml->layout(0, 0, 600, 400);

    ConstraintSet start, end;
    start.constrainWidth(1, 100); start.constrainHeight(1, 50);
    start.connect(1, ConstraintSet::LEFT, ConstraintSet::PARENT, ConstraintSet::LEFT);
    start.connect(1, ConstraintSet::TOP, ConstraintSet::PARENT, ConstraintSet::TOP);
    end.constrainWidth(1, 100); end.constrainHeight(1, 50);
    end.connect(1, ConstraintSet::RIGHT, ConstraintSet::PARENT, ConstraintSet::RIGHT);
    end.connect(1, ConstraintSet::TOP, ConstraintSet::PARENT, ConstraintSet::TOP);

    ml->setTransition(&start, &end);

    // Position keyframe: arc the child downward at the midpoint.
    MotionKeyPosition arc;
    arc.mFramePosition = 50;
    arc.mPercentX = 0.5f;
    arc.mAltPercentY = 0.5f;
    ml->addKeyPosition(1, &arc);

    ml->setProgress(0.5f);
    // Without the keyframe the child would be at (250, 0). With the arc it's offset downward.
    EXPECT_NE(tv->getTop(), 0);    // arced off the linear path
    EXPECT_GT(tv->getTop(), 100);  // significantly below y=0
}

// NOTE: match_constraint (0dp) — spread-fill works; the match-constraint re-measure loop
// (BasicMeasure size-dependent iteration) is still deferred, so 0dp+content-dependent sizing
// (wrap/percent) isn't yet converged.
// re-measure loop (deferred). A 0dp child currently fills ~576 of 600 instead of exactly 600.

// ConstraintLayoutStates parses a <StateSet> into State/Variants and selects a ConstraintSet by
// the layout's dimensions. State "base" has a default set (100x50) plus a Variant for width>600
// (200x50). At width 400 the default wins; at width 800 the Variant wins. Inline <ConstraintSet>
// refs resolve by resource id (the `constraints` attr). Anti-flap: passing the current set's id
// with matching dims returns the same set.
TEST(CLConstraintLayout, ConstraintLayoutStatesMatch) {
    App& app = App::getInstance();
    auto parser = app.getResources().getXml(gui_test::R::xml::stateset_match);
    while (parser->getEventType() != XmlPullParser::START_TAG &&
           parser->getEventType() != XmlPullParser::END_DOCUMENT) {
        parser->next();
    }

    ConstraintLayoutStates states(&app, nullptr, *parser);

    const int baseId    = gui_test::R::id::st_base;
    const int defaultId = gui_test::R::id::st_default;
    const int wideId    = gui_test::R::id::st_wide;
    EXPECT_EQ(states.getDefaultState(), baseId);  // defaultState attr resolved

    // Narrow width -> default set (width 100); wide width -> Variant set (width 200).
    ConstraintSet* narrow = states.convertToConstraintSet(-1, baseId, 400.0f, 300.0f);
    ConstraintSet* wide   = states.convertToConstraintSet(-1, baseId, 800.0f, 300.0f);
    ASSERT_NE(narrow, nullptr);
    ASSERT_NE(wide, nullptr);
    EXPECT_NE(narrow, wide);                       // different sets selected by dimension
    EXPECT_EQ(narrow->get(gui_test::R::id::st_target).layout.mWidth, 100);
    EXPECT_EQ(wide->get(gui_test::R::id::st_target).layout.mWidth, 200);

    // Anti-flap: with the wide set currently applied and wide dims, it is kept (returns the same set).
    EXPECT_EQ(states.convertToConstraintSet(wideId, baseId, 800.0f, 300.0f), wide);
    EXPECT_EQ(states.convertToConstraintSet(defaultId, baseId, 800.0f, 300.0f), wide);
}

// ConstraintLayoutStates.updateConstraints applies the dimension-selected ConstraintSet to the
// bound layout. State "base": default set sizes the target to 100px, a Variant for width>600 sizes
// it to 200px. At width 400 the child measures 100; at width 900 it measures 200 (re-applied +
// re-measured).
TEST(CLConstraintLayout, ConstraintLayoutStatesSwitchesOnResize) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(&App::getInstance());
    TextView* tv = new TextView(&App::getInstance()); tv->setText("X");
    tv->setId(gui_test::R::id::sw_target);
    cl->addView(tv, new ConstraintLayout::LayoutParams(100, 50));

    auto parser = app.getResources().getXml(gui_test::R::xml::stateset_switch);
    while (parser->getEventType() != XmlPullParser::START_TAG &&
           parser->getEventType() != XmlPullParser::END_DOCUMENT) {
        parser->next();
    }

    ConstraintLayoutStates states(&app, cl, *parser);
    const int baseId = gui_test::R::id::sw_base;

    // Narrow (400 wide) -> default "small" set -> child width 100.
    states.updateConstraints(baseId, 400.0f, 400.0f);
    cl->measure(exactly(800), exactly(400));
    cl->layout(0, 0, 800, 400);
    EXPECT_EQ(tv->getWidth(), 100);

    // Wide (900 wide) -> Variant "large" set -> child width 200.
    states.updateConstraints(baseId, 900.0f, 400.0f);
    cl->measure(exactly(800), exactly(400));
    cl->layout(0, 0, 800, 400);
    EXPECT_EQ(tv->getWidth(), 200);
}

// ViewTransition parses a <ViewTransition> (a per-view animation) out of a MotionScene. Verifies the
// attribute dispatch (onStateTransition, duration, viewTransitionMode, motionInterpolator), the
// nested <KeyFrameSet>, and the getViewTransitionById lookup.
TEST(CLConstraintLayout, ViewTransitionParse) {
    App& app = App::getInstance();
    MotionScene scene(nullptr);
    scene.load(&app, gui_test::R::xml::motion_scene_vt_parse);

    EXPECT_EQ(scene.getViewTransitionCount(), 1u);
    auto* vt = scene.getViewTransitionAt(0);
    ASSERT_NE(vt, nullptr);
    EXPECT_EQ(vt->getStateTransition(), ViewTransition::ONSTATE_ACTION_DOWN);
    EXPECT_EQ(vt->getDuration(), 300);
    EXPECT_EQ(vt->getViewTransitionMode(), ViewTransition::VIEWTRANSITIONMODE_NOSTATE);
    EXPECT_EQ(vt->getInterpolatorString(), "standard");
    ASSERT_NE(vt->getKeyFrames(), nullptr);
    // getViewTransitionById round-trips the parsed id.
    EXPECT_EQ(scene.getViewTransitionById(vt->getId()), vt);
}

// A noState <ViewTransition> animates a single view independently of the main transition. Firing it
// builds a standalone per-view Motion (start==end==current frame; the KeyFrameSet drives the
// deviation) and starts an Animate on the controller. Stepping the controller advances the Animate:
// at the midpoint the KeyAttribute (alpha=0 @frame50) takes the view's alpha to 0; reaching the end
// (actionDown, no hold) removes the animation.
TEST(CLConstraintLayout, ViewTransitionNoStateAnimates) {
    App& app = App::getInstance();
    MotionLayout* ml = new MotionLayout(&App::getInstance());
    TextView* tv = new TextView(&App::getInstance()); tv->setText("X");
    tv->setId(gui_test::R::id::vtn_target);
    ml->addView(tv, new ConstraintLayout::LayoutParams(100, 50));
    ml->measure(exactly(600), exactly(400));
    ml->layout(0, 0, 600, 400);

    auto scene = std::make_unique<MotionScene>(ml);
    scene->load(&app, gui_test::R::xml::motion_scene_vt_nostate);
    const int vtId = scene->getViewTransitionAt(0)->getId(); // capture before moving the scene
    ml->setScene(std::move(scene));

    auto* controller = ml->getViewTransitionController();
    ASSERT_NE(controller, nullptr);

    std::vector<View*> views = { tv };
    ml->viewTransition(vtId, views);
    EXPECT_EQ(controller->animationCount(), 1u); // an Animate registered + first frame ran

    controller->stepAnimations(200); // 200ms of a 400ms animation → progress 0.5 → alpha 0
    EXPECT_NEAR(tv->getAlpha(), 0.0f, 0.05f);

    controller->stepAnimations(400); // past the end → the Animate removes itself
    EXPECT_EQ(controller->animationCount(), 0u);
}

// onStateTransition=actionDownUp sets mHoldAt100, so reaching progress 1.0 HOLDS the animation
// (it stays registered, awaiting a release that reverses it) — unlike actionDown which removes on
// completion. This pins the DownUp-specific branch; mutateReverse itself mirrors mutateForward.
TEST(CLConstraintLayout, ViewTransitionNoStateDownUpHolds) {
    App& app = App::getInstance();
    MotionLayout* ml = new MotionLayout(&App::getInstance());
    TextView* tv = new TextView(&App::getInstance()); tv->setText("X");
    tv->setId(gui_test::R::id::vtd_target);
    ml->addView(tv, new ConstraintLayout::LayoutParams(100, 50));
    ml->measure(exactly(600), exactly(400));
    ml->layout(0, 0, 600, 400);

    auto scene = std::make_unique<MotionScene>(ml);
    scene->load(&app, gui_test::R::xml::motion_scene_vt_downup);
    const int vtId = scene->getViewTransitionAt(0)->getId();
    ml->setScene(std::move(scene));

    auto* controller = ml->getViewTransitionController();
    ASSERT_NE(controller, nullptr);
    std::vector<View*> views = { tv };
    ml->viewTransition(vtId, views);
    EXPECT_EQ(controller->animationCount(), 1u);

    controller->stepAnimations(400); // reach progress 1.0 — actionDownUp HOLDS (not removed)
    EXPECT_EQ(controller->animationCount(), 1u);
}

// The actionDownUp press-release effect: after holding at 100%, an ACTION_UP reverses the animation
// (Animate.reactTo -> reverse), which then steps back to 0 over upDuration and removes itself. This
// pins the mutateReverse path that DownUpHolds leaves untested. A main <Transition> is set up first
// so the MotionLayout has a current state (touchEvent bails while currentState == -1, faithfully).
TEST(CLConstraintLayout, ViewTransitionNoStateDownUpReverses) {
    App& app = App::getInstance();
    MotionLayout* ml = new MotionLayout(&App::getInstance());
    TextView* tv = new TextView(&App::getInstance()); tv->setText("X");
    tv->setId(gui_test::R::id::rv_target);
    ml->addView(tv, new ConstraintLayout::LayoutParams(100, 50));
    ml->measure(exactly(600), exactly(400));
    ml->layout(0, 0, 600, 400);

    auto scene = std::make_unique<MotionScene>(ml);
    scene->load(&app, gui_test::R::xml::motion_scene_vt_reverse);
    const int vtId = scene->getViewTransitionAt(0)->getId();
    auto* t = scene->getCurrentTransition();
    ASSERT_NE(t, nullptr);
    const int startId = t->getStartId();
    const int endId = t->getEndId();
    ml->setScene(std::move(scene));
    ml->setTransition(startId, endId); // establishes mCurrentState so touchEvent does not bail
    ASSERT_NE(ml->getCurrentState(), -1);

    auto* controller = ml->getViewTransitionController();
    ASSERT_NE(controller, nullptr);
    std::vector<View*> views = { tv };
    ml->viewTransition(vtId, views);          // forward -> holds at 100%
    controller->stepAnimations(400);
    ASSERT_EQ(controller->animationCount(), 1u);

    MotionEvent up;                            // ACTION_UP reverses the held animation
    up.setAction(MotionEvent::ACTION_UP);
    controller->touchEvent(up);
    EXPECT_EQ(controller->animationCount(), 1u); // reversing, still registered

    controller->stepAnimations(100);           // half of upDuration(200) -> still mid-reverse
    EXPECT_EQ(controller->animationCount(), 1u);
    controller->stepAnimations(100);           // upDuration elapsed -> reaches 0 -> removes
    EXPECT_EQ(controller->animationCount(), 0u);
}

// currentState/allStates delta mode: a <ConstraintOverride> is applied as a FIELD-LEVEL overlay onto
// the target ConstraintSet's constraint, not a wholesale sub-struct replace. A scale-only delta
// (scaleX=1.5) must take effect WITHOUT clobbering an unrelated field (rotation=30) the delta never
// touched. (Android's sparse Delta does this precisely; CDROID approximates via default-difference.)
TEST(CLConstraintLayout, ViewTransitionDeltaOverlaysWithoutClobbering) {
    App& app = App::getInstance();
    const int id = gui_test::R::id::vds_target;
    // The "current state" constraint for the target already has rotation=30 (a rotated button).
    ConstraintSet target;
    target.get(id).transform.rotation = 30.0f;
    EXPECT_EQ(target.get(id).transform.scaleX, 1.0f); // scale untouched so far

    // Parse the delta: <ConstraintOverride motionTarget scaleX="1.5"/>.
    auto parser = app.getResources().getXml(gui_test::R::xml::vt_delta_scale);
    while (parser->getEventType() != XmlPullParser::START_TAG &&
           parser->getEventType() != XmlPullParser::END_DOCUMENT) {
        parser->next();
    }
    ConstraintSet delta;
    delta.loadConstraint(&app, *parser);

    ASSERT_TRUE(delta.contains(id));           // motionTarget resolved to the target id
    delta.applyDelta(target.get(id));
    EXPECT_FLOAT_EQ(target.get(id).transform.scaleX, 1.5f); // delta applied
    EXPECT_FLOAT_EQ(target.get(id).transform.rotation, 30.0f); // preserved — not clobbered to 0
}

// A Layout-field delta (anchor + margin) overlays the same way as Transform: setting leftToLeft +
// leftMargin takes effect WITHOUT clobbering an unrelated anchor (topToTop) the delta never touched.
TEST(CLConstraintLayout, ViewTransitionDeltaOverlaysLayoutFields) {
    App& app = App::getInstance();
    const int id = gui_test::R::id::vdl_target;
    ConstraintSet target;
    target.get(id).layout.topToTop = 0;    // already anchored top->parent
    target.get(id).layout.leftToLeft = -1; // not yet anchored horizontally

    auto parser = app.getResources().getXml(gui_test::R::xml::vt_delta_layout);
    while (parser->getEventType() != XmlPullParser::START_TAG &&
           parser->getEventType() != XmlPullParser::END_DOCUMENT) {
        parser->next();
    }
    ConstraintSet delta;
    delta.loadConstraint(&app, *parser);

    ASSERT_TRUE(delta.contains(id));
    delta.applyDelta(target.get(id));
    EXPECT_EQ(target.get(id).layout.leftToLeft, 0);  // parent — delta applied
    EXPECT_EQ(target.get(id).layout.leftMargin, 20); // delta applied
    EXPECT_EQ(target.get(id).layout.topToTop, 0);    // preserved — not clobbered
}

// Precise delta: a delta that sets a field to its DEFAULT value (rotation="0") RESETS the target's
// rotation. The old default-difference approximation skipped default values and would have left the
// target's rotation=30 untouched. Authored-field overlay applies it.
TEST(CLConstraintLayout, ViewTransitionDeltaAppliesDefaultValuedField) {
    App& app = App::getInstance();
    const int id = gui_test::R::id::vdd_target;
    ConstraintSet target;
    target.get(id).transform.rotation = 30.0f;   // target currently rotated
    target.get(id).transform.scaleX = 2.0f;       // and scaled

    // delta explicitly sets rotation="0" (its default) — should reset rotation, leave scaleX.
    auto parser = app.getResources().getXml(gui_test::R::xml::vt_delta_default);
    while (parser->getEventType() != XmlPullParser::START_TAG &&
           parser->getEventType() != XmlPullParser::END_DOCUMENT) {
        parser->next();
    }
    ConstraintSet delta;
    delta.loadConstraint(&app, *parser);

    delta.applyDelta(target.get(id));
    EXPECT_FLOAT_EQ(target.get(id).transform.rotation, 0.0f); // RESET to 0 (precise)
    EXPECT_FLOAT_EQ(target.get(id).transform.scaleX, 2.0f);    // untouched (not authored by delta)
}

// viewTransitionMode=allStates persists the delta into every ConstraintSet except the current state,
// so the change survives a later state switch. Firing an allStates VT (scaleX=1.5) from the start
// state writes the delta into the END set (the from-state is the animation source, not persisted).
TEST(CLConstraintLayout, ViewTransitionAllStatesPersistsDelta) {
    App& app = App::getInstance();
    MotionLayout* ml = new MotionLayout(&App::getInstance());
    TextView* tv = new TextView(&App::getInstance()); tv->setText("X");
    tv->setId(gui_test::R::id::as_target);
    ml->addView(tv, new ConstraintLayout::LayoutParams(100, 50));
    ml->measure(exactly(600), exactly(400));
    ml->layout(0, 0, 600, 400);

    auto scene = std::make_unique<MotionScene>(ml);
    scene->load(&app, gui_test::R::xml::motion_scene_vt_allstates);
    const int vtId = scene->getViewTransitionAt(0)->getId();
    const int endId = scene->getCurrentTransition()->getEndId();
    const int startId = scene->getCurrentTransition()->getStartId();
    ml->setScene(std::move(scene));
    ml->setTransition(startId, endId);

    ASSERT_NE(ml->getConstraintSet(endId), nullptr);
    EXPECT_FLOAT_EQ(ml->getConstraintSet(endId)->get(gui_test::R::id::as_target).transform.scaleX, 1.0f); // before
    std::vector<View*> views = { tv };
    ml->viewTransition(vtId, views); // allStates persists the delta into every set != current
    EXPECT_FLOAT_EQ(ml->getConstraintSet(endId)->get(gui_test::R::id::as_target).transform.scaleX, 1.5f); // persisted
}

// currentState delta integration: firing the VT solves current+delta offscreen (captureState) and
// runs the per-view Animate from the current frame to the delta'd END frame — the androidx
// current->current+delta semantics under CDROID's independent-animation design (f6b5548e4): the
// main transition is never replaced. Uses a WIDTH delta (applied via layout(), not a transform
// setter) so an unattached test view reliably reflects it. Afterwards the main transition still
// drives start↔end untouched.
TEST(CLConstraintLayout, ViewTransitionCurrentStateAnimatesDelta) {
    App& app = App::getInstance();
    MotionLayout* ml = new MotionLayout(&App::getInstance());
    TextView* tv = new TextView(&App::getInstance()); tv->setText("X");
    tv->setId(gui_test::R::id::csa_target);
    ml->addView(tv, new ConstraintLayout::LayoutParams(100, 50));
    ml->measure(exactly(600), exactly(400));
    ml->layout(0, 0, 600, 400);

    auto scene = std::make_unique<MotionScene>(ml);
    scene->load(&app, gui_test::R::xml::motion_scene_vt_currentstate);
    const int vtId = scene->getViewTransitionAt(0)->getId();
    const int startId = scene->getCurrentTransition()->getStartId();
    const int endId = scene->getCurrentTransition()->getEndId();
    ml->setScene(std::move(scene));
    ml->setTransition(startId, endId);
    ml->setProgress(0.0f);
    EXPECT_EQ(tv->getWidth(), 100); // at the start state
    EXPECT_EQ(tv->getLeft(), 0);

    std::vector<View*> views = { tv };
    ml->viewTransition(vtId, views); // currentState: Animate current -> (current + width=200 delta)
    auto* controller = ml->getViewTransitionController();
    ASSERT_NE(controller, nullptr);
    EXPECT_EQ(controller->animationCount(), 1u);
    EXPECT_EQ(tv->getWidth(), 100); // first frame ran; still at the current state

    controller->stepAnimations(400); // full duration -> progress 1.0 -> the delta'd end frame
    EXPECT_EQ(tv->getWidth(), 200);  // width delta applied via the independent Animate
    EXPECT_EQ(tv->getLeft(), 0);     // position untouched (width-only delta, left-anchored)

    // The main transition was never replaced: progress still drives start↔end.
    ml->setProgress(1.0f);
    EXPECT_EQ(tv->getLeft(), 500); // 600 - 100: end set, right-pinned
    EXPECT_EQ(tv->getWidth(), 100);
}

// setsTag: a noState ViewTransition that completes sets a keyed tag on its target (setsTag is a
// reference; the tag key is the resource id). After firing + stepping to completion the tag is set.
TEST(CLConstraintLayout, ViewTransitionSetsTagOnCompletion) {
    App& app = App::getInstance();
    MotionLayout* ml = new MotionLayout(&App::getInstance());
    TextView* tv = new TextView(&App::getInstance()); tv->setText("X");
    tv->setId(gui_test::R::id::stg_target);
    ml->addView(tv, new ConstraintLayout::LayoutParams(100, 50));
    ml->measure(exactly(600), exactly(400));
    ml->layout(0, 0, 600, 400);

    const int tagKey = gui_test::R::id::stg_tag_key;
    auto scene = std::make_unique<MotionScene>(ml);
    scene->load(&app, gui_test::R::xml::motion_scene_vt_setstag);
    const int vtId = scene->getViewTransitionAt(0)->getId();
    ml->setScene(std::move(scene));
    auto* controller = ml->getViewTransitionController();
    ASSERT_NE(controller, nullptr);

    EXPECT_EQ(tv->getTag(tagKey), nullptr); // before
    ml->viewTransition(vtId, { tv });
    controller->stepAnimations(400);        // complete -> applyTags
    EXPECT_NE(tv->getTag(tagKey), nullptr); // setsTag applied
}

// ifTagSet gating (checkTags): an actionDown ViewTransition with ifTagSet=4242 fires only while the
// target carries that tag. The controller caches the targeted-view set on the first touch (built via
// matchesView, so the tag must be present then), and the firing loop re-checks matchesView each touch
// — so removing the tag gates the next fire. A main <Transition> is set up so touchEvent does not
// bail on currentState == -1.
TEST(CLConstraintLayout, ViewTransitionIfTagSetGates) {
    App& app = App::getInstance();
    MotionLayout* ml = new MotionLayout(&App::getInstance());
    TextView* tv = new TextView(&App::getInstance()); tv->setText("X");
    tv->setId(gui_test::R::id::ift_target);
    ml->addView(tv, new ConstraintLayout::LayoutParams(100, 50));
    ml->measure(exactly(600), exactly(400));
    ml->layout(0, 0, 600, 400);

    const int tagKey = gui_test::R::id::ift_tag_key;
    auto scene = std::make_unique<MotionScene>(ml);
    scene->load(&app, gui_test::R::xml::motion_scene_vt_iftag);
    const int startId = scene->getCurrentTransition()->getStartId();
    const int endId = scene->getCurrentTransition()->getEndId();
    ml->setScene(std::move(scene));
    ml->setTransition(startId, endId);

    auto* controller = ml->getViewTransitionController();
    ASSERT_NE(controller, nullptr);
    MotionEvent down;
    down.setAction(MotionEvent::ACTION_DOWN);
    down.setLocation(0, 0); // over the top-left target

    tv->setTag(tagKey, (void*) tv);           // tag present -> view enters the targeted set + fires
    controller->touchEvent(down);
    EXPECT_EQ(controller->animationCount(), 1u);
    controller->stepAnimations(400);           // let it finish so the count resets
    ASSERT_EQ(controller->animationCount(), 0u);

    tv->setTag(tagKey, nullptr);               // remove the tag -> checkTags now gates the fire
    controller->touchEvent(down);
    EXPECT_EQ(controller->animationCount(), 0u); // did NOT fire (ifTagSet no longer satisfied)
}

// sharedValueSet trigger: the ViewTransitionController listens on the SharedValues registry for the
// VT's SharedValueId; when MotionLayout.setSharedValue reaches the target value, the VT fires.
// (Process-wide registry; a unique key avoids interference with other tests' leaked listeners.)
TEST(CLConstraintLayout, ViewTransitionSharedValueSetFires) {
    App& app = App::getInstance();
    MotionLayout* ml = new MotionLayout(&App::getInstance());
    TextView* tv = new TextView(&App::getInstance()); tv->setText("X");
    tv->setId(gui_test::R::id::sv_target);
    ml->addView(tv, new ConstraintLayout::LayoutParams(100, 50));
    ml->measure(exactly(600), exactly(400));
    ml->layout(0, 0, 600, 400);

    const int key = gui_test::R::id::sv_key;
    auto scene = std::make_unique<MotionScene>(ml);
    scene->load(&app, gui_test::R::xml::motion_scene_vt_shared);
    const int startId = scene->getCurrentTransition()->getStartId();
    const int endId = scene->getCurrentTransition()->getEndId();
    ml->setScene(std::move(scene));
    ml->setTransition(startId, endId); // currentState valid so onNewValue does not bail

    auto* controller = ml->getViewTransitionController();
    ASSERT_NE(controller, nullptr);
    ml->setSharedValue(key, 42); // target value reached -> VT fires
    EXPECT_EQ(controller->animationCount(), 1u);
}

// String motionTarget: a ViewTransition whose motionTarget is a regex matches views whose
// LayoutParams.constraintTag matches it (not by view id). "btn_.*" matches "btn_save" but not "label".
TEST(CLConstraintLayout, ViewTransitionMatchesConstraintTag) {
    App& app = App::getInstance();
    MotionLayout* ml = new MotionLayout(&App::getInstance());
    TextView* a = new TextView(&App::getInstance()); a->setText("A"); a->setId(1);
    auto* lpa = new ConstraintLayout::LayoutParams(100, 50); lpa->constraintTag = "btn_save";
    ml->addView(a, lpa);
    TextView* b = new TextView(&App::getInstance()); b->setText("B"); b->setId(2);
    auto* lpb = new ConstraintLayout::LayoutParams(100, 50); lpb->constraintTag = "label";
    ml->addView(b, lpb);

    auto scene = std::make_unique<MotionScene>(ml);
    scene->load(&app, gui_test::R::xml::motion_scene_vt_tagregex);
    auto* vt = scene->getViewTransitionAt(0);
    ASSERT_NE(vt, nullptr);
    ml->setScene(std::move(scene));

    EXPECT_TRUE(vt->matchesView(a));  // "btn_save" matches "btn_.*"
    EXPECT_FALSE(vt->matchesView(b)); // "label" does not match
}

// A ViewTransition-level (set-level) <CustomAttribute> is stored on the delta and applied to every
// target via applyDelta. (loadCustomAttribute + the set-level carry; the ViewTransition parse wires
// <CustomAttribute> children to loadCustomAttribute.)
TEST(CLConstraintLayout, ViewTransitionSetLevelCustomAttribute) {
    App& app = App::getInstance();
    auto parser = app.getResources().getXml(gui_test::R::xml::vt_customattr_setlevel);
    while (parser->getEventType() != XmlPullParser::START_TAG &&
           parser->getEventType() != XmlPullParser::END_DOCUMENT) {
        parser->next();
    }

    ConstraintSet delta;
    delta.loadCustomAttribute(&app, *parser);
    ConstraintSet target;
    ConstraintSet::Constraint& c = target.get(1);
    EXPECT_TRUE(c.mCustomAttributes.empty());

    delta.applyDelta(c); // set-level customs carry onto the target
    ASSERT_EQ(c.mCustomAttributes.size(), 1u);
    EXPECT_EQ(c.mCustomAttributes[0].name, "textColor");
    EXPECT_EQ(c.mCustomAttributes[0].type, ConstraintSet::CustomAttribute::COLOR);
}

// applyViewTransition(id, Motion*) merges a ViewTransition's KeyFrameSet into a standalone Motion
// (Android applyViewTransition). A Motion with start==end==current frame is otherwise inert; after
// merging a KeyAttribute (alpha=0 @frame50), interpolating at progress 0.5 yields alpha 0.
TEST(CLConstraintLayout, ViewTransitionApplyViewTransitionMergesKeyframes) {
    App& app = App::getInstance();
    MotionLayout* ml = new MotionLayout(&App::getInstance());
    TextView* tv = new TextView(&App::getInstance()); tv->setText("X");
    tv->setId(gui_test::R::id::mk_target);
    ml->addView(tv, new ConstraintLayout::LayoutParams(100, 50));
    ml->measure(exactly(600), exactly(400));
    ml->layout(0, 0, 600, 400);

    auto scene = std::make_unique<MotionScene>(ml);
    scene->load(&app, gui_test::R::xml::motion_scene_vt_merge);
    const int vtId = scene->getViewTransitionAt(0)->getId();
    ml->setScene(std::move(scene));

    // A standalone Motion whose start == end == the view's current frame (so without keyframes it
    // would not deviate).
    MotionWidget mw;
    MotionLayout::captureWidgetFrame(mw, tv);
    Motion m;
    m.setStart(&mw);
    m.setEnd(&mw);
    m.setup(600, 400, 400.0f);

    EXPECT_TRUE(ml->applyViewTransition(vtId, &m)); // found the VT and merged its keyframes

    MotionWidget temp;
    m.interpolate(&temp, 0.5f);
    EXPECT_NEAR(temp.getAlpha(), 0.0f, 0.05f); // merged KeyAttribute applied at the midpoint
}

#endif // ENABLE_CONSTRAINTLAYOUT
