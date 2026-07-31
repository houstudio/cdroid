/*
 * ConstraintSet demo — tap the box to instantly switch between two layouts.
 * Start: box centered. End: box pinned bottom-right. No animation, instant switch.
 * Build: make constraintsetdemo -j44   Run: ./constraintsetdemo
 */
#include <cdroid.h>
#include <widgetEx/constraintlayout/constraintlayout.h>
#include <widgetEx/constraintlayout/constraintset.h>

using namespace cdroid;

int main(int argc, const char* argv[]) {
    App app(argc, argv);
    Window* win = new Window(0, 0, -1, -1);

    ConstraintLayout* cl = new ConstraintLayout(-1, -1);
    cl->setBackgroundColor(0xFF1B1B2F);
    win->addView(cl);

    TextView* box = new TextView("Tap", 150, 100);
    box->setId(1);
    box->setBackgroundColor(0xFFEF5350);
    box->setGravity(Gravity::CENTER);
    box->setTextColor(0xFFFFFFFF);
    box->setTextSize(20);
    // Start layout: centered.
    auto* lp = new ConstraintLayout::LayoutParams(150, 100);
    lp->leftToLeft = ConstraintLayout::PARENT_ID;
    lp->rightToRight = ConstraintLayout::PARENT_ID;
    lp->topToTop = ConstraintLayout::PARENT_ID;
    lp->bottomToBottom = ConstraintLayout::PARENT_ID;
    cl->addView(box, lp);

    // Pre-build two ConstraintSets.
    ConstraintSet centered, corner;
    centered.constrainWidth(1, 150); centered.constrainHeight(1, 100);
    centered.connect(1, ConstraintSet::LEFT,  ConstraintSet::PARENT, ConstraintSet::LEFT);
    centered.connect(1, ConstraintSet::RIGHT, ConstraintSet::PARENT, ConstraintSet::RIGHT);
    centered.connect(1, ConstraintSet::TOP,   ConstraintSet::PARENT, ConstraintSet::TOP);
    centered.connect(1, ConstraintSet::BOTTOM,ConstraintSet::PARENT, ConstraintSet::BOTTOM);

    corner.constrainWidth(1, 150); corner.constrainHeight(1, 100);
    corner.connect(1, ConstraintSet::RIGHT,  ConstraintSet::PARENT, ConstraintSet::RIGHT);
    corner.connect(1, ConstraintSet::BOTTOM, ConstraintSet::PARENT, ConstraintSet::BOTTOM);
    corner.setMargin(1, ConstraintSet::RIGHT, 30);
    corner.setMargin(1, ConstraintSet::BOTTOM, 30);

    bool atCorner = false;
    box->setOnClickListener([&](View&) {
        atCorner = !atCorner;
        if (atCorner) corner.applyTo(cl);
        else          centered.applyTo(cl);
    });

    return app.exec();
}
