/*********************************************************************************
 * TransitionUtils::copyViewImage verification sample.
 *
 * Two fixed-size containers side by side. Clicking Toggle reparents the pink box
 * between them under beginDelayedTransition(sceneRoot, Fade).
 *
 * Moving the box OUT of sceneRoot leaves it attached (in sidePanel) with no end
 * values inside sceneRoot -- exactly the Visibility::onDisappear branch that
 * consumes TransitionUtils::copyViewImage: a snapshot of the box fades out over
 * sceneRoot while the real box is already sitting in sidePanel. Moving it back
 * exercises the plain appear path (real view fades in).
 *
 * Build:  make viewcopydemo -j44   (in outX64-Debug)
 *********************************************************************************/
#include <cdroid.h>
#include <cdlog.h>
#include <transition/transitionmanager.h>
#include <transition/fade.h>

int main(int argc, const char* argv[]){
    App app(argc, argv);
    Window* w = new Window(0, 0, -1, -1);
    w->setBackgroundColor(0xFF223344);

    LinearLayout* root = new LinearLayout(&app);
    root->setOrientation(LinearLayout::VERTICAL);
    w->addView(root);

    Button* toggle = new Button(&app); toggle->setText("Toggle (Move Box)");
    toggle->setTextSize(32);
    root->addView(toggle);

    LinearLayout* row = new LinearLayout(&app);
    row->setOrientation(LinearLayout::HORIZONTAL);
    row->setLayoutParams(new LinearLayout::LayoutParams(LayoutParams::MATCH_PARENT, 0, 1.0f));
    root->addView(row);

    // sceneRoot: the container the transition runs on. Reparenting out of it
    // leaves the view attached elsewhere -> copyViewImage snapshot path.
    FrameLayout* sceneRoot = new FrameLayout(&app);
    sceneRoot->setBackgroundColor(0xFF112233);
    sceneRoot->setLayoutParams(new LinearLayout::LayoutParams(0, LayoutParams::MATCH_PARENT, 1.0f));
    row->addView(sceneRoot);

    FrameLayout* sidePanel = new FrameLayout(&app);
    sidePanel->setBackgroundColor(0xFF333311);
    sidePanel->setLayoutParams(new LinearLayout::LayoutParams(0, LayoutParams::MATCH_PARENT, 1.0f));
    row->addView(sidePanel);

    View* box = new View(&app);
    box->setBackgroundColor(0xFFFF4081);
    box->setId(3001);
    sceneRoot->addView(box, new FrameLayout::LayoutParams(240, 240, Gravity::CENTER));

    Fade* fade = new Fade();
    fade->setDuration(1500); // long enough to eyeball / capture mid-fade frames
    bool inScene = true;
    toggle->setOnClickListener([sceneRoot, sidePanel, fade, box, &inScene](View&){
        // NOTE: scoping the transition to sceneRoot animates the leaving side only
        // (snapshot fade-out via copyViewImage); the box appears instantly in
        // sidePanel -- a transition animates just its own subtree. A second
        // beginDelayedTransition(sidePanel, ...) to fade the arrival in is a
        // separate cross-root scenario: the second transition's running-animator
        // cleanup currently cancels the first's snapshot animator mid-flight
        // (under investigation), so the demo stays on the single-root form.
        TransitionManager::beginDelayedTransition(sceneRoot, fade);
        ViewGroup* from = inScene ? static_cast<ViewGroup*>(sceneRoot) : static_cast<ViewGroup*>(sidePanel);
        ViewGroup* to   = inScene ? static_cast<ViewGroup*>(sidePanel)  : static_cast<ViewGroup*>(sceneRoot);
        from->removeView(box);
        to->addView(box, new FrameLayout::LayoutParams(240, 240, Gravity::CENTER));
        LOGD("viewcopydemo: box moved %s", inScene ? "sceneRoot -> sidePanel" : "sidePanel -> sceneRoot");
        inScene = !inScene;
    });

    return app.exec();
}
