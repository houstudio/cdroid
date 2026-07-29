/*
 * MotionLayout demo — a box that animates between two constraint states on tap.
 *   start: top-left (leftToLeft + topToTop = parent)
 *   end:   bottom-right (rightToRight + bottomToBottom = parent)
 * Easing: "standard" (ease-in-out). Position keyframe: arc upward at midpoint.
 * Tapping the box toggles transitionToEnd / transitionToStart.
 *
 * Build: make motionlayoutdemo -j44 (auto-registered by apps/samples/CMakeLists.txt)
 * Run:   ./motionlayoutdemo
 */
#include <cdroid.h>
#include <widgetEx/constraintlayout/constraintset.h>
#include <widgetEx/constraintlayout/motion/motionlayout.h>
#include <widgetEx/constraintlayout/core/motion/motionkeyposition.h>

using namespace cdroid;

int main(int argc, const char* argv[]) {
    App app(argc, argv);
    Window* win = new Window(0, 0, -1, -1);

    MotionLayout* ml = new MotionLayout(-1, -1); // fills the window
    ml->setBackgroundColor(0xFF1B1B2F);
    win->addView(ml);

    // The animated box.
    TextView* box = new TextView("Tap me", 200, 120);
    box->setId(1);
    box->setBackgroundColor(0xFFEF5350);
    box->setGravity(Gravity::CENTER);
    box->setTextColor(0xFFFFFFFF);
    box->setTextSize(22);
    ml->addView(box, new ConstraintLayout::LayoutParams(200, 120));

    // Start state: pinned top-left. End state: pinned bottom-right.
    ConstraintSet start, end;
    start.constrainWidth(1, 200);  start.constrainHeight(1, 120);
    start.connect(1, ConstraintSet::LEFT, ConstraintSet::PARENT, ConstraintSet::LEFT);
    start.connect(1, ConstraintSet::TOP,  ConstraintSet::PARENT, ConstraintSet::TOP);
    end.constrainWidth(1, 200);    end.constrainHeight(1, 120);
    end.connect(1, ConstraintSet::RIGHT,  ConstraintSet::PARENT, ConstraintSet::RIGHT);
    end.connect(1, ConstraintSet::BOTTOM, ConstraintSet::PARENT, ConstraintSet::BOTTOM);

    ml->setTransition(&start, &end); // capture is deferred until first layout (we have a size)
    ml->setTransitionDuration(700);
    ml->setTransitionEasing("standard"); // ease-in-out

    // Position keyframe: arc the box upward at the midpoint (perpendicular offset).
    auto* arc = new MotionKeyPosition();
    arc->mFramePosition = 50;
    arc->mPercentX = 0.5f;       // halfway along the path
    arc->mAltPercentY = -0.3f;   // arc upward (negative = up)
    ml->addKeyPosition(1, arc);

    bool atEnd = false;
    box->setOnClickListener([ml, &atEnd](View&) {
        atEnd = !atEnd;
        if (atEnd) ml->transitionToEnd();
        else       ml->transitionToStart();
    });

    return app.exec();
}
