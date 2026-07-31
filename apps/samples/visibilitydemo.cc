/*********************************************************************************
 * Visibility-transition runtime verification sample (Slide & Explode).
 *
 * Single window. Each click toggles a centered box's visibility AND cycles the
 * transition: Slide  -> Explode -> Slide ...
 *   TransitionManager::beginDelayedTransition(sceneRoot, slide|explode);
 *   box->setVisibility(VISIBLE / GONE);
 * Slide (default edge = BOTTOM) slides the box up on appear / down on disappear.
 * Explode expands the box out from / collapses toward its center.
 *
 * This exercises the disappear-render path (PFLAG_INVALIDATED redraw + fixed-size
 * sceneRoot) for the Visibility subclasses beyond Fade.
 *
 * Build:  make visibilitydemo -j44   (in outX64-Debug)
 *********************************************************************************/
#include <cdroid.h>
#include <cdlog.h>
#include <transition/transitionmanager.h>
#include <transition/slide.h>
#include <transition/explode.h>

int main(int argc, const char* argv[]){
    App app(argc, argv);
    Window* w = new Window(0, 0, -1, -1);
    w->setBackgroundColor(0xFF223344);

    LinearLayout* root = new LinearLayout(-1, -1);
    root->setOrientation(LinearLayout::VERTICAL);
    w->addView(root);

    Button* toggle = new Button("Toggle (Slide / Explode)", 700, 140);
    toggle->setTextSize(32);
    root->addView(toggle);

    // FIXED-size sceneRoot (weight=1) — a WRAP parent collapses when its only child goes
    // GONE, hiding the disappearing view (see disappear-rotation lesson). Keep it fixed.
    FrameLayout* sceneRoot = new FrameLayout(-1, 800);
    sceneRoot->setLayoutParams(new LinearLayout::LayoutParams(LayoutParams::MATCH_PARENT, 0, 1.0f));
    sceneRoot->setBackgroundColor(0xFF112233);
    root->addView(sceneRoot);

    View* box = new View(0, 0);
    box->setBackgroundColor(0xFFFF4081);
    box->setId(2001);
    sceneRoot->addView(box, new FrameLayout::LayoutParams(360, 360, Gravity::CENTER));

    // Reused — beginDelayedTransition clones them (originals borrowed, not freed).
    Slide*   slide   = new Slide();    // default edge = BOTTOM
    Explode* explode = new Explode();
    slide->setDuration(600);
    explode->setDuration(600);

    bool visible = true;
    int  which   = 0; // even = Slide, odd = Explode
    toggle->setOnClickListener([sceneRoot, slide, explode, box, &visible, &which](View&){
        Transition* t = ((which % 2) == 0) ? (Transition*) slide : (Transition*) explode;
        TransitionManager::beginDelayedTransition(sceneRoot, t);
        visible = !visible;
        which++;
        box->setVisibility(visible ? View::VISIBLE : View::GONE);
        LOGD("visibilitydemo: vis=%d next=%s", visible ? 0 : 8, ((which % 2) == 0) ? "Slide" : "Explode");
    });

    return app.exec();
}
