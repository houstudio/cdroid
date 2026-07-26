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

#include <sstream>

#include <core/app.h>
#include <core/attributeset.h>
#include <core/xmlpullparser.h>
#include <view/view.h>
#include <widget/textview.h>
#include <widgetEx/constraintlayout/constraintlayout.h>
#include <widgetEx/constraintlayout/core/widgets/constraintwidget.h>
#include <widgetEx/constraintlayout/constraintset.h>
#include <widgetEx/constraintlayout/keyframes.h>
#include <widgetEx/constraintlayout/motionscene.h>
#include <widgetEx/constraintlayout/core/motion/motionkeyattributes.h>
#include <widgetEx/constraintlayout/core/motion/motionkeyposition.h>
#include <widgetEx/constraintlayout/motionlayout.h>
#include <widgetEx/constraintlayout/core/motion/motionkeyposition.h>
#include <widgetEx/constraintlayout/barrier.h>
#include <widgetEx/constraintlayout/group.h>
#include <widgetEx/constraintlayout/placeholder.h>
#include <widgetEx/constraintlayout/flow.h>

using namespace cdroid;

static int exactly(int size) {
    return View::MeasureSpec::makeMeasureSpec(size, View::MeasureSpec::EXACTLY);
}

// A fixed 100x50 child whose left+right both connect to the parent → horizontally centered
// in a 600-wide container: x = (600 - 100) / 2 = 250.
TEST(ConstraintLayout, CentersChildHorizontally) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(600, 400);
    TextView* tv = new TextView("X", 100, 50);
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
TEST(ConstraintLayout, PinsChildLeft) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(600, 400);
    TextView* tv = new TextView("X", 100, 50);
    auto* lp = new ConstraintLayout::LayoutParams(100, 50);
    lp->leftToLeft = ConstraintLayout::PARENT_ID;
    cl->addView(tv, lp);

    cl->measure(exactly(600), exactly(400));
    cl->layout(0, 0, 600, 400);

    EXPECT_EQ(tv->getLeft(), 0);
    EXPECT_EQ(tv->getWidth(), 100);
}

// A centered child with horizontal bias 0.3 → x = 0.3 * (600 - 100) = 150.
TEST(ConstraintLayout, BiasChild) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(600, 400);
    TextView* tv = new TextView("X", 100, 50);
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
TEST(ConstraintLayout, ChainSpreadTwo) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(600, 400);
    TextView* a = new TextView("A", 100, 50);
    TextView* b = new TextView("B", 100, 50);
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

    // CHAIN_SPREAD: equal gaps before/inside/after → (600-200)/3=133
    EXPECT_EQ(a->getLeft(), 133);
    EXPECT_EQ(a->getRight(), 233);
    EXPECT_EQ(b->getLeft(), 367);
    EXPECT_EQ(b->getRight(), 467);
}

// A 0dp (MATCH_CONSTRAINT) child with left+right to parent → spread-fills width 600.
TEST(ConstraintLayout, MatchConstraintFills) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(600, 400);
    TextView* tv = new TextView("X", 0, 50);
    auto* lp = new ConstraintLayout::LayoutParams(0, 50);
    lp->leftToLeft = ConstraintLayout::PARENT_ID;
    lp->rightToRight = ConstraintLayout::PARENT_ID;
    cl->addView(tv, lp);
    cl->measure(exactly(600), exactly(400));
    cl->layout(0, 0, 600, 400);
    EXPECT_EQ(tv->getLeft(), 0);
    EXPECT_EQ(tv->getWidth(), 600);
}

// A vertical Guideline at 50% (x=300) + a 0dp child constrained left=guideline, right=parent.
// The child should fill from 300 to 600 → x=300, width=300.
TEST(ConstraintLayout, GuidelinePositionsChild) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(600, 400);

    // Guideline child
    View* gl = new View(0, 0);
    gl->setId(10);
    auto* glp = new ConstraintLayout::LayoutParams(LayoutParams::WRAP_CONTENT, LayoutParams::WRAP_CONTENT);
    glp->orientation = ConstraintWidget::VERTICAL;
    glp->guidePercent = 0.5f;
    glp->validate();
    cl->addView(gl, glp);

    // Child: 0dp, left=guideline, right=parent
    TextView* tv = new TextView("X", 0, 50);
    auto* lp = new ConstraintLayout::LayoutParams(0, 50);
    lp->leftToLeft = 10;
    lp->rightToRight = ConstraintLayout::PARENT_ID;
    cl->addView(tv, lp);

    cl->measure(exactly(600), exactly(400));
    cl->layout(0, 0, 600, 400);

    EXPECT_EQ(tv->getLeft(), 300);
    EXPECT_EQ(tv->getWidth(), 300);
}

// Ratio: width=200 FIXED, height=0dp MATCH_CONSTRAINT, ratio "2:1" → height=100.
TEST(ConstraintLayout, RatioChild) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(600, 400);
    TextView* tv = new TextView("X", 200, 0);
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
TEST(ConstraintLayout, ChainPackedTwo) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(600, 400);
    TextView* a = new TextView("A", 100, 50);
    TextView* b = new TextView("B", 100, 50);
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
TEST(ConstraintLayout, BarrierRightAtMaxEdge) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(600, 400);
    TextView* a = new TextView("A", 100, 50); a->setId(1);
    TextView* b = new TextView("B", 100, 50); b->setId(2);
    TextView* c = new TextView("C", 100, 50); c->setId(4);

    auto* lpa = new ConstraintLayout::LayoutParams(100, 50);
    lpa->leftToLeft = ConstraintLayout::PARENT_ID;
    auto* lpb = new ConstraintLayout::LayoutParams(100, 50);
    lpb->leftToRight = 1;
    auto* lpc = new ConstraintLayout::LayoutParams(100, 50);
    lpc->leftToLeft = 3; // barrier id

    Barrier* barrier = new Barrier(LayoutParams::WRAP_CONTENT, LayoutParams::WRAP_CONTENT);
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
TEST(ConstraintLayout, BarrierLeftAtMinEdge) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(600, 400);
    TextView* a = new TextView("A", 100, 50); a->setId(1);
    TextView* b = new TextView("B", 200, 50); b->setId(2);
    TextView* c = new TextView("C", 80, 50);  c->setId(4);

    auto* lpa = new ConstraintLayout::LayoutParams(100, 50);
    lpa->leftToLeft = ConstraintLayout::PARENT_ID;
    lpa->leftMargin = 50;
    auto* lpb = new ConstraintLayout::LayoutParams(200, 50);
    lpb->leftToLeft = ConstraintLayout::PARENT_ID;
    lpb->leftMargin = 200;
    auto* lpc = new ConstraintLayout::LayoutParams(80, 50);
    lpc->leftToLeft = 3; // barrier id

    Barrier* barrier = new Barrier(LayoutParams::WRAP_CONTENT, LayoutParams::WRAP_CONTENT);
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
TEST(ConstraintLayout, GroupHidesReferenced) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(600, 400);
    TextView* a = new TextView("A", 100, 50); a->setId(1);
    TextView* b = new TextView("B", 100, 50); b->setId(2);
    cl->addView(a, new ConstraintLayout::LayoutParams(100, 50));
    cl->addView(b, new ConstraintLayout::LayoutParams(100, 50));

    Group* group = new Group(LayoutParams::WRAP_CONTENT, LayoutParams::WRAP_CONTENT);
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
TEST(ConstraintLayout, PlaceholderPositionsContent) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(600, 400);

    TextView* x = new TextView("X", 120, 60); x->setId(1);
    auto* lpx = new ConstraintLayout::LayoutParams(120, 60);
    lpx->leftToLeft = ConstraintLayout::PARENT_ID; // X's own (ignored) origin
    lpx->topToTop = ConstraintLayout::PARENT_ID;

    Placeholder* placeholder = new Placeholder(120, 60); placeholder->setId(2);
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
TEST(ConstraintLayout, MatchConstraintPercent) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(600, 400);
    TextView* tv = new TextView("X", 0, 50);
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
TEST(ConstraintLayout, MatchConstraintMaxCaps) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(600, 400);
    TextView* tv = new TextView("X", 0, 50);
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
TEST(ConstraintLayout, MatchConstraintWrap) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(600, 400);
    TextView* tv = new TextView("X", 0, 50);
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
TEST(ConstraintLayout, ChainWeightsDistribute) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(600, 400);
    TextView* a = new TextView("A", 0, 50); a->setId(1);
    TextView* b = new TextView("B", 0, 50); b->setId(2);
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
TEST(ConstraintLayout, FlowWrapsToSecondRow) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(600, 400);
    TextView* a = new TextView("A", 100, 50); a->setId(1);
    TextView* b = new TextView("B", 100, 50); b->setId(2);
    TextView* c = new TextView("C", 100, 50); c->setId(3);
    TextView* d = new TextView("D", 100, 50); d->setId(4);
    cl->addView(a, new ConstraintLayout::LayoutParams(100, 50));
    cl->addView(b, new ConstraintLayout::LayoutParams(100, 50));
    cl->addView(c, new ConstraintLayout::LayoutParams(100, 50));
    cl->addView(d, new ConstraintLayout::LayoutParams(100, 50));

    Flow* flow = new Flow(200, 100);
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
TEST(ConstraintLayout, FlowVerticalStacksColumn) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(600, 400);
    TextView* views[4];
    for (int i = 0; i < 4; i++) {
        views[i] = new TextView("X", 50, 50);
        views[i]->setId(i + 1);
        cl->addView(views[i], new ConstraintLayout::LayoutParams(50, 50));
    }
    Flow* flow = new Flow(50, 200);
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

// ---- ConstraintSet ----

// Build constraints programmatically and apply: connect both sides to parent -> centered.
TEST(ConstraintLayout, ConstraintSetCentersChild) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(600, 400);
    TextView* a = new TextView("A", 100, 50); a->setId(1);
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
TEST(ConstraintLayout, ConstraintSetCloneAndModify) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(600, 400);
    TextView* a = new TextView("A", 100, 50); a->setId(1);
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
// Constraint model. "parent"/literal ids resolve via Context::getId (strtol fallback). Verifies the
// name-keyed attribute dispatch (populateConstraint) for dimensions, anchors, bias, margins,
// visibility/alpha (PropertySet), transforms, chain style, and ratio.
TEST(ConstraintLayout, ConstraintSetXmlLoad) {
    App& app = App::getInstance();
    const std::string xml =
        "<ConstraintSet xmlns:android=\"http://schemas.android.com/apk/res/android\">"
        "  <Constraint android:id=\"42\""
        "              android:layout_width=\"100dp\""
        "              android:layout_height=\"60dp\""
        "              layout_constraintLeft_toLeftOf=\"parent\""
        "              layout_constraintRight_toRightOf=\"parent\""
        "              layout_constraintHorizontal_bias=\"0.25\""
        "              android:layout_marginLeft=\"8dp\""
        "              layout_constraintHorizontal_chainStyle=\"packed\""
        "              layout_constraintDimensionRatio=\"2:1\""
        "              android:visibility=\"invisible\""
        "              android:alpha=\"0.5\""
        "              android:rotation=\"45\""
        "              android:scaleX=\"2\" />"
        "</ConstraintSet>";
    auto stream = std::make_unique<std::stringstream>(xml);
    XmlPullParser parser(&app, std::move(stream));
    // Advance to the <ConstraintSet> START_TAG, then let load() consume through its END_TAG.
    while (parser.getEventType() != XmlPullParser::START_TAG &&
           parser.getEventType() != XmlPullParser::END_DOCUMENT) {
        parser.next();
    }

    ConstraintSet cs;
    cs.load(&app, parser);

    ASSERT_TRUE(cs.contains(42));
    const auto& c = cs.get(42);
    EXPECT_EQ(c.layout.mWidth, 100);
    EXPECT_EQ(c.layout.mHeight, 60);
    EXPECT_EQ(c.layout.leftToLeft, 0);     // "parent" -> PARENT_ID=0
    EXPECT_EQ(c.layout.rightToRight, 0);
    EXPECT_FLOAT_EQ(c.layout.horizontalBias, 0.25f);
    EXPECT_EQ(c.layout.leftMargin, 8);
    EXPECT_EQ(c.layout.horizontalChainStyle, ConstraintWidget::CHAIN_PACKED);
    EXPECT_EQ(c.layout.dimensionRatio, "2:1");
    EXPECT_EQ(c.propertySet.visibility, 4); // invisible -> View::INVISIBLE=4
    EXPECT_FLOAT_EQ(c.propertySet.alpha, 0.5f);
    EXPECT_FLOAT_EQ(c.transform.rotation, 45.0f);
    EXPECT_FLOAT_EQ(c.transform.scaleX, 2.0f);
}

// KeyFrames parses a <KeyFrameSet> into core MotionKey subclasses (KeyAttribute + KeyPosition),
// filed under the target view id. Attribute names are bare localnames (XmlPullParser strips the
// namespace); motionTarget resolves to a view id via Context::getId (strtol fallback: "42" -> 42).
TEST(ConstraintLayout, KeyFramesXmlParse) {
    App& app = App::getInstance();
    const std::string xml =
        "<KeyFrameSet xmlns:android=\"http://schemas.android.com/apk/res/android\">"
        "  <KeyAttribute motionTarget=\"42\" framePosition=\"50\" alpha=\"0\" rotation=\"90\" />"
        "  <KeyPosition motionTarget=\"42\" framePosition=\"50\" percentX=\"0.5\" percentY=\"0.5\""
        "              keyPositionType=\"pathRelative\" />"
        "</KeyFrameSet>";
    auto stream = std::make_unique<std::stringstream>(xml);
    XmlPullParser parser(&app, std::move(stream));
    while (parser.getEventType() != XmlPullParser::START_TAG &&
           parser.getEventType() != XmlPullParser::END_DOCUMENT) {
        parser.next();
    }

    KeyFrames kf(&app, parser);
    auto keys = kf.getKeysForView(42);
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
// <KeyFrameSet> and an <OnClick> child. The ConstraintSet ids ("@+id/start") resolve scene-locally
// by name, so the Transition's start/end refs agree with the parsed sets.
TEST(ConstraintLayout, MotionSceneXmlParse) {
    App& app = App::getInstance();
    const std::string xml =
        "<MotionScene xmlns:android=\"http://schemas.android.com/apk/res/android\" defaultDuration=\"300\">"
        "  <Transition constraintSetStart=\"@+id/start\" constraintSetEnd=\"@+id/end\" duration=\"500\">"
        "    <OnClick targetId=\"1\" clickAction=\"toggle\" />"
        "    <KeyFrameSet>"
        "      <KeyAttribute motionTarget=\"1\" framePosition=\"50\" alpha=\"0\" />"
        "    </KeyFrameSet>"
        "  </Transition>"
        "  <ConstraintSet android:id=\"@+id/start\">"
        "    <Constraint android:id=\"1\" layout_width=\"100\" layout_height=\"50\""
        "                layout_constraintLeft_toLeftOf=\"parent\" layout_constraintTop_toTopOf=\"parent\" />"
        "  </ConstraintSet>"
        "  <ConstraintSet android:id=\"@+id/end\">"
        "    <Constraint android:id=\"1\" layout_width=\"100\" layout_height=\"50\""
        "                layout_constraintRight_toRightOf=\"parent\" layout_constraintBottom_toBottomOf=\"parent\" />"
        "  </ConstraintSet>"
        "</MotionScene>";
    auto stream = std::make_unique<std::stringstream>(xml);
    XmlPullParser parser(&app, std::move(stream));

    MotionScene scene(nullptr);
    scene.load(&app, parser);

    auto* t = scene.getCurrentTransition();
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->getDuration(), 500);
    EXPECT_FALSE(t->isAbstract());

    auto* startSet = scene.getConstraintSet(t->getStartId());
    auto* endSet = scene.getConstraintSet(t->getEndId());
    ASSERT_NE(startSet, nullptr);
    ASSERT_NE(endSet, nullptr);
    EXPECT_EQ(startSet->get(1).layout.leftToLeft, 0);  // parent
    EXPECT_EQ(endSet->get(1).layout.rightToRight, 0);  // parent

    ASSERT_NE(t->getKeyFrames(), nullptr);
    EXPECT_EQ(t->getKeyFrames()->getKeysForView(1).size(), 1u);
    ASSERT_EQ(t->getOnClicks().size(), 1u);
    EXPECT_EQ(t->getOnClicks()[0].targetId, 1);
    EXPECT_EQ(t->getOnClicks()[0].clickAction, MotionScene::Transition::FLAG_TOGGLE);
}

// A container with padding insets its children: leftToLeft=parent with paddingLeft=50 -> x=50.
TEST(ConstraintLayout, PaddingInsetsChildren) {
    App& app = App::getInstance();
    ConstraintLayout* cl = new ConstraintLayout(600, 400);
    cl->setPadding(50, 20, 0, 0);
    TextView* a = new TextView("A", 100, 50); a->setId(1);
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
TEST(ConstraintLayout, MotionLayoutAnimatesChild) {
    App& app = App::getInstance();
    MotionLayout* ml = new MotionLayout(600, 400);
    TextView* tv = new TextView("X", 100, 50); tv->setId(1);
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
TEST(ConstraintLayout, MotionLayoutKeyPositionArc) {
    App& app = App::getInstance();
    MotionLayout* ml = new MotionLayout(600, 400);
    TextView* tv = new TextView("X", 100, 50); tv->setId(1);
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

#endif // ENABLE_CONSTRAINTLAYOUT
