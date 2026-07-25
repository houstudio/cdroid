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

#include <core/app.h>
#include <core/attributeset.h>
#include <view/view.h>
#include <widget/textview.h>
#include <widgetEx/constraintlayout/constraintlayout.h>

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

#endif // ENABLE_CONSTRAINTLAYOUT
