/*********************************************************************************
 * TransitionSet composition verification sample.
 *
 * Single window. A "Toggle" button flips a box's visibility AND rotation inside a
 * sceneRoot, wrapped in:
 *   TransitionSet set; set.addTransition(new Fade()); set.addTransition(new Rotate());
 *   TransitionManager::beginDelayedTransition(sceneRoot, set);
 * Played ORDERING_TOGETHER, the box should fade in/out WHILE rotating.
 *
 * Build:  make transitionsetdemo -j44   (in outX64-Debug)
 *********************************************************************************/
#include <cdroid.h>
#include <cdlog.h>
#include <transition/transitionmanager.h>
#include <transition/transitionset.h>
#include <transition/fade.h>
#include <transition/rotate.h>

int main(int argc, const char* argv[]){
    App app(argc, argv);
    Window* w = new Window(0, 0, -1, -1);
    w->setBackgroundColor(0xFF223344);

    LinearLayout* root = new LinearLayout(-1, -1);
    root->setOrientation(LinearLayout::VERTICAL);
    w->addView(root);

    Button* toggle = new Button("Toggle Fade + Rotate", 700, 140);
    toggle->setTextSize(32);
    root->addView(toggle);

    // Bounded-height sceneRoot so the button stays visible above the box.
    FrameLayout* sceneRoot = new FrameLayout(-1, 800);
    root->addView(sceneRoot);

    View* box = new View(500, 500);
    box->setBackgroundColor(0xFFFF4081);
    box->setId(2001);
    sceneRoot->addView(box);

    // Compose Fade + Rotate, played together. Reused each click (beginDelayedTransition
    // clones it; the original is borrowed, not freed — java GC equivalent).
    TransitionSet* set = new TransitionSet();
    set->addTransition(new Fade());
    set->addTransition(new Rotate());
    set->setDuration(600); // propagated to both children

    bool visible = true;
    float rotation = 0.f;
    toggle->setOnClickListener([sceneRoot, set, box, &visible, &rotation](View&){
        TransitionManager::beginDelayedTransition(sceneRoot, set);
        visible = !visible;
        rotation += 90.f;
        box->setVisibility(visible ? View::VISIBLE : View::GONE);
        box->setRotation(rotation);
        LOGD("transitionsetdemo: vis=%d rot=%f", visible ? 0 : 8, rotation);
    });

    return app.exec();
}
