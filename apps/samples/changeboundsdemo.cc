/*********************************************************************************
 * ChangeBounds runtime verification sample.
 *
 * Single window. A "Toggle" button flips a box's size AND position
 * (top-left 300x300  <->  bottom-right 500x500) inside a FIXED-size sceneRoot,
 * wrapped in:
 *   TransitionManager::beginDelayedTransition(sceneRoot, new ChangeBounds());
 * The box should smoothly animate its bounds (move + resize) between the two states.
 *
 * Build:  make changeboundsdemo -j44   (in outX64-Debug)
 *********************************************************************************/
#include <cdroid.h>
#include <cdlog.h>
#include <transition/transitionmanager.h>
#include <transition/changebounds.h>

int main(int argc, const char* argv[]){
    App app(argc, argv);
    Window* w = new Window(0, 0, -1, -1);
    w->setBackgroundColor(0xFF223344);

    LinearLayout* root = new LinearLayout(-1, -1);
    root->setOrientation(LinearLayout::VERTICAL);
    w->addView(root);

    Button* toggle = new Button("Toggle ChangeBounds", 700, 140);
    toggle->setTextSize(32);
    root->addView(toggle);

    // FIXED-size sceneRoot (weight=1). A WRAP parent is unstable to animate bounds
    // against, and (per the disappear-rotation lesson) a WRAP parent can collapse and
    // hide children. Keep it fixed.
    FrameLayout* sceneRoot = new FrameLayout(-1, 800);
    sceneRoot->setLayoutParams(new LinearLayout::LayoutParams(LayoutParams::MATCH_PARENT, 0, 1.0f));
    sceneRoot->setBackgroundColor(0xFF112233);
    root->addView(sceneRoot);

    View* box = new View(0, 0);
    box->setBackgroundColor(0xFFFF4081);
    box->setId(2001);
    // Start state: top-left, 300x300.
    sceneRoot->addView(box, new FrameLayout::LayoutParams(300, 300, Gravity::TOP | Gravity::LEFT));

    // Reused each click — beginDelayedTransition clones it (original is borrowed).
    ChangeBounds* changeBounds = new ChangeBounds();
    changeBounds->setDuration(600);

    bool big = false;
    toggle->setOnClickListener([sceneRoot, changeBounds, box, &big](View&){
        TransitionManager::beginDelayedTransition(sceneRoot, changeBounds);
        big = !big;
        FrameLayout::LayoutParams* lp = (FrameLayout::LayoutParams*) box->getLayoutParams();
        if (big){
            lp->width  = 500;
            lp->height = 500;
            lp->gravity = Gravity::BOTTOM | Gravity::RIGHT;
        } else {
            lp->width  = 300;
            lp->height = 300;
            lp->gravity = Gravity::TOP | Gravity::LEFT;
        }
        box->setLayoutParams(lp);
        LOGD("changeboundsdemo: big=%d w=%d h=%d grav=%d", big ? 1 : 0, lp->width, lp->height, lp->gravity);
    });

    return app.exec();
}
