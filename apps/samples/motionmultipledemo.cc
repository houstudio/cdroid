/*
 * MotionLayout multi-child demo — 3 boxes animate simultaneously with different paths + alpha.
 * Box 1: left→right + fade out mid (alpha keyframe at 50).
 * Box 2: top→bottom.
 * Box 3: top-left→bottom-right (diagonal).
 * Tap any box to toggle.
 * Build: make motionmultipledemo -j44   Run: ./motionmultipledemo
 */
#include <cdroid.h>
#include <widgetEx/constraintlayout/constraintset.h>
#include <widgetEx/constraintlayout/motion/motionlayout.h>
#include <widgetEx/constraintlayout/core/motion/motionkeyattributes.h>

using namespace cdroid;

static TextView* makeBox(const char* label, int id, uint32_t color) {
    auto* tv = new TextView(label, 120, 80);
    tv->setId(id);
    tv->setBackgroundColor(color);
    tv->setGravity(Gravity::CENTER);
    tv->setTextColor(0xFFFFFFFF);
    tv->setTextSize(18);
    return tv;
}

int main(int argc, const char* argv[]) {
    App app(argc, argv);
    Window* win = new Window(0, 0, -1, -1);

    MotionLayout* ml = new MotionLayout(-1, -1);
    ml->setBackgroundColor(0xFF1B1B2F);
    win->addView(ml);

    TextView* b1 = makeBox("Fade", 1, 0xFFEF5350);
    TextView* b2 = makeBox("Down", 2, 0xFF66BB6A);
    TextView* b3 = makeBox("Diag", 3, 0xFF42A5F5);
    ml->addView(b1, new ConstraintLayout::LayoutParams(120, 80));
    ml->addView(b2, new ConstraintLayout::LayoutParams(120, 80));
    ml->addView(b3, new ConstraintLayout::LayoutParams(120, 80));

    ConstraintSet start, end;
    // Box 1: left→right
    start.connect(1, ConstraintSet::LEFT, ConstraintSet::PARENT, ConstraintSet::LEFT);
    start.connect(1, ConstraintSet::TOP,  ConstraintSet::PARENT, ConstraintSet::TOP);
    start.constrainWidth(1, 120); start.constrainHeight(1, 80);
    end.connect(1, ConstraintSet::RIGHT, ConstraintSet::PARENT, ConstraintSet::RIGHT);
    end.connect(1, ConstraintSet::TOP,   ConstraintSet::PARENT, ConstraintSet::TOP);
    end.constrainWidth(1, 120); end.constrainHeight(1, 80);

    // Box 2: top→bottom (centered)
    start.connect(2, ConstraintSet::LEFT,  ConstraintSet::PARENT, ConstraintSet::LEFT);
    start.connect(2, ConstraintSet::RIGHT, ConstraintSet::PARENT, ConstraintSet::RIGHT);
    start.connect(2, ConstraintSet::TOP,   ConstraintSet::PARENT, ConstraintSet::TOP);
    start.constrainWidth(2, 120); start.constrainHeight(2, 80);
    end.connect(2, ConstraintSet::LEFT,   ConstraintSet::PARENT, ConstraintSet::LEFT);
    end.connect(2, ConstraintSet::RIGHT,  ConstraintSet::PARENT, ConstraintSet::RIGHT);
    end.connect(2, ConstraintSet::BOTTOM, ConstraintSet::PARENT, ConstraintSet::BOTTOM);
    end.constrainWidth(2, 120); end.constrainHeight(2, 80);

    // Box 3: top-left→bottom-right
    start.connect(3, ConstraintSet::LEFT, ConstraintSet::PARENT, ConstraintSet::LEFT);
    start.connect(3, ConstraintSet::TOP,  ConstraintSet::PARENT, ConstraintSet::TOP);
    start.constrainWidth(3, 120); start.constrainHeight(3, 80);
    end.connect(3, ConstraintSet::RIGHT,  ConstraintSet::PARENT, ConstraintSet::RIGHT);
    end.connect(3, ConstraintSet::BOTTOM, ConstraintSet::PARENT, ConstraintSet::BOTTOM);
    end.constrainWidth(3, 120); end.constrainHeight(3, 80);

    ml->setTransition(&start, &end);
    ml->setTransitionDuration(800);
    ml->setTransitionEasing("standard");

    // Alpha keyframe on box 1: fade to 0.3 at midpoint.
    auto* fade = new MotionKeyAttributes();
    fade->mFramePosition = 50;
    fade->mAlpha = 0.3f;
    ml->addKeyAttributes(1, fade);

    bool atEnd = false;
    auto toggle = [&]() {
        atEnd = !atEnd;
        if (atEnd) ml->transitionToEnd();
        else       ml->transitionToStart();
    };
    b1->setOnClickListener([toggle](View&) { toggle(); });
    b2->setOnClickListener([toggle](View&) { toggle(); });
    b3->setOnClickListener([toggle](View&) { toggle(); });

    return app.exec();
}
