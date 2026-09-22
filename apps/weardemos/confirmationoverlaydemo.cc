// Port of SupportWearDemos ConfirmationOverlayDemo: one overlay shown over
// the whole activity (showOn), one over a specific view (showAbove).
#include <core/app.h>
#include <cdroid.h>
#include <core/activityfactory.h>
#include <widget/button.h>
#include <widgetEx/wear/confirmationoverlay.h>
#include "R.h"

using namespace cdroid;

class ConfirmationOverlayDemo : public Window {
public:
    ConfirmationOverlayDemo() : Window(&App::getInstance(), 0, 0, -1, -1) {
        ViewGroup* root = (ViewGroup*)LayoutInflater::from(getContext())
                ->inflate(weardemos::R::layout::confirmation_overlay_demo, this, false);
        addView(root);

        ViewGroup* content = (ViewGroup*)root->findViewById(weardemos::R::id::content);
        ConfirmationOverlay* overlay = new ConfirmationOverlay();

        Button* activityTrigger = (Button*)root->findViewById(
                weardemos::R::id::activity_overlay_button);
        Button* viewTrigger = (Button*)root->findViewById(
                weardemos::R::id::view_overlay_button);
        // Upstream runs under a flat wear theme; CDROID's Material buttons carry
        // elevation 2dp, which draws them ABOVE the overlay's Z=0 scrim (AOSP
        // Z-ordering, by design). Flatten both triggers so the page shows the
        // upstream look — the animator must go too, it re-asserts elevation on
        // every state evaluation.
        for (Button* b : {activityTrigger, viewTrigger}) {
            b->setStateListAnimator(nullptr);
            b->setElevation(0);
        }
        activityTrigger->setOnClickListener([this, overlay](View&) {
            overlay->showOn(this);
        });

        viewTrigger->setOnClickListener([overlay, content](View&) {
            overlay->showAbove(content);
        });
    }
};
REGISTER_ACTIVITY(ConfirmationOverlayDemo);
