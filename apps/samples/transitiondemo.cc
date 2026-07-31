/*********************************************************************************
 * Minimal android.transition verification sample.
 *
 * Single window. A "Toggle" button flips a colored box's visibility
 * (VISIBLE <-> GONE) inside a sceneRoot, wrapped in
 *   TransitionManager::beginDelayedTransition(sceneRoot, new Fade());
 * The Fade transition should animate the box fading in/out.
 *
 * Build:  make transitiondemo -j44   (in outX64-Debug)
 *********************************************************************************/
#include <cdroid.h>
#include <cdlog.h>
#include <transition/transitionmanager.h>
#include <transition/fade.h>

int main(int argc, const char* argv[]){
    App app(argc, argv);
    Window* w = new Window(0, 0, -1, -1);
    w->setBackgroundColor(0xFF223344);

    // Vertical root holding the toggle button + the sceneRoot.
    LinearLayout* root = new LinearLayout(-1, -1);
    root->setOrientation(LinearLayout::VERTICAL);
    w->addView(root);

    Button* toggle = new Button("Toggle Fade", 480, 140);
    toggle->setTextSize(36);
    root->addView(toggle);

    // sceneRoot: the ViewGroup whose child visibility changes are transitioned.
    LinearLayout* sceneRoot = new LinearLayout(-1, -1);
    sceneRoot->setOrientation(LinearLayout::VERTICAL);
    root->addView(sceneRoot);

    // The box that fades in/out.
    View* box = new View(640, 480);
    box->setBackgroundColor(0xFFFF4081);
    box->setId(1001);
    sceneRoot->addView(box);

    // Reuse one Fade instance (beginDelayedTransition clones it each time; the original
    // is borrowed, never freed — equivalent to android's GC'd `new Fade()` per call but
    // without per-click allocation).
    Fade* fade = new Fade();

    toggle->setOnClickListener([sceneRoot, fade, box](View&){
        TransitionManager::beginDelayedTransition(sceneRoot, fade);
        bool visible = (box->getVisibility() == View::VISIBLE);
        box->setVisibility(visible ? View::GONE : View::VISIBLE);
        LOGD("transitiondemo: box visibility -> %d", visible ? View::GONE : View::VISIBLE);
    });

    return app.exec();
}
