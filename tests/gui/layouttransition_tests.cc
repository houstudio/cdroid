/* LayoutTransition runtime regressions. Both cases lock crashes found by the
 * never-before-built apps/samples/layouttransition.cc smoke run (2026-09-17):
 *
 * 1. TopOfParentHierarchySurvivesAdd — the parent-hierarchy walk fed the top
 *    container into setupChangeAnimation with a null parent (AOSP guards with
 *    `parentParent instanceof ViewGroup`, java:786-792); the layout listener
 *    captured that null and the next layout pass dereferenced it in
 *    requestTransitionStart.
 *
 * 2. PendingRemoverDetachesListener — the pending-anim remover deletes the
 *    pending animator after duration+100ms but left the layout listener (which
 *    captures it) registered on the child; the next size-changing layout call
 *    ran anim->setupEndValues() on freed memory.
 */
#include <gtest/gtest.h>
#include <core/app.h>
#include <view/view.h>
#include <view/viewgroup.h>
#include <widget/linearlayout.h>
#include <widget/button.h>
#include <animation/layouttransition.h>
#include "guienvironment.h"
using namespace cdroid;

class LAYOUTTRANSITION : public testing::Test {
public:
    virtual void SetUp() {}
    virtual void TearDown() {}
};

static void layoutAt(ViewGroup* vg, int w, int h) {
    vg->measure(View::MeasureSpec::makeMeasureSpec(w, View::MeasureSpec::EXACTLY),
                View::MeasureSpec::makeMeasureSpec(h, View::MeasureSpec::EXACTLY));
    vg->layout(0, 0, w, h);
}

// Three-level hierarchy with the transition on the middle level: adding a view
// walks up to the top container whose getParent() is null — the listener must
// not capture that null (crash before the instanceof-guard fix).
TEST_F(LAYOUTTRANSITION, TopOfParentHierarchySurvivesAdd) {
    App& app = App::getInstance();
    LinearLayout* top = new LinearLayout(&app);
    top->setOrientation(LinearLayout::VERTICAL);
    LinearLayout* middle = new LinearLayout(&app);
    middle->setOrientation(LinearLayout::VERTICAL);
    top->addView(middle);
    LinearLayout* existing = new LinearLayout(&app);
    middle->addView(existing);
    middle->setLayoutTransition(new LayoutTransition());

    layoutAt(top, 320, 240);
    // Add -> runChangeTransition -> parent-hierarchy walk past the top.
    Button* btn = new Button(&app); btn->setText("Add");
    middle->addView(btn, new ViewGroup::LayoutParams(
            ViewGroup::LayoutParams::MATCH_PARENT, 40));
    // Size-changing relayout of the whole tree fires the change listeners.
    layoutAt(top, 320, 280);
    layoutAt(top, 320, 300);
    pumpUntilIdle(600);

    EXPECT_EQ(top->getHeight(), 300);
    EXPECT_EQ(btn->getHeight(), 40);
    top->removeAllViews();
    delete top;
}

// The pending animation (never started: no layout change during the pending
// window) is deleted by the remover after duration+100ms; the layout listener
// it captured must be detached so this later size-changing layout is a no-op,
// not a use-after-free.
TEST_F(LAYOUTTRANSITION, PendingRemoverDetachesListener) {
    App& app = App::getInstance();
    LinearLayout* vg = new LinearLayout(&app);
    vg->setOrientation(LinearLayout::VERTICAL);
    LinearLayout* existing = new LinearLayout(&app);
    vg->addView(existing);
    LayoutTransition* lt = new LayoutTransition();
    lt->setDuration(100);
    vg->setLayoutTransition(lt);

    layoutAt(vg, 320, 240);
    Button* btn = new Button(&app); btn->setText("Add");
    vg->addView(btn, new ViewGroup::LayoutParams(
            ViewGroup::LayoutParams::MATCH_PARENT, 40));
    // Drain past duration+100ms with the animator still pending (no relayout
    // so the listener never promotes it into currentChangingAnimations).
    pumpFor(500);
    // Now relayout: if the stale listener is still registered, this runs the
    // freed animator's setupEndValues() inside the OnLayoutChange callback.
    layoutAt(vg, 320, 320);
    layoutAt(vg, 320, 360);
    pumpUntilIdle(600);

    EXPECT_EQ(vg->getHeight(), 360);
    EXPECT_EQ(btn->getHeight(), 40);
    vg->removeAllViews();
    delete vg;
}
