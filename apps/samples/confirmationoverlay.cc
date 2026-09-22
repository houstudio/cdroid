// ConfirmationOverlay E2E probe (CDROID-added): exercises showOn/showAbove
// with a message (text-inset margin path) and both listener lifetimes.
#include <cdroid.h>
#include <porting/cdlog.h>
#include <widget/button.h>
#include <widgetEx/wear/confirmationoverlay.h>

using namespace cdroid;

int main(int argc, const char* argv[]) {
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);
    App app(argc, argv);
    Window* w = new Window(0, 0, 454, 454);
    w->setBackgroundColor(0xFF303030);

    ConfirmationOverlay* overlay = new ConfirmationOverlay();
    overlay->setMessage("Saved on phone");
    overlay->setDuration(1000);
    overlay->setOnAnimationFinishedListener([]() {
        LOGI("E2E: onAnimationFinished fired");
    });

    Button* b = new Button(w->getContext(), nullptr);
    b->setText("SHOW");
    // The overlay is a Z=0 sibling (upstream sets no elevation), so a Material
    // button (elevation 2dp) draws ABOVE the scrim — Android's documented
    // Z-ordering (ViewGroup::buildOrderedChildList), not a compositing bug.
    // Upstream wear demos don't hit this because wear themes are flat. Flatten
    // the trigger the Android way — the button's state-list animator re-asserts
    // elevation=2dp on every state evaluation (base state carries a 0-duration
    // elevation animator in AOSP's own button_state_list_anim_material), so a
    // bare setElevation(0) gets clobbered back; the animator must go too.
    b->setStateListAnimator(nullptr);
    b->setElevation(0);
    b->setOnClickListener([w, overlay](View&) {
        LOGI("E2E: click -> showOn");
        overlay->showOn(w);
    });
    // plain top-left placement; the click target is its own 200x60 box
    w->addView(b, 200, 60);

    return app.exec();
}
