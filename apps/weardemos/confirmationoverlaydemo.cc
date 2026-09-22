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
        activityTrigger->setOnClickListener([this, overlay](View&) {
            overlay->showOn(this);
        });

        Button* viewTrigger = (Button*)root->findViewById(
                weardemos::R::id::view_overlay_button);
        viewTrigger->setOnClickListener([overlay, content](View&) {
            overlay->showAbove(content);
        });
    }
};
REGISTER_ACTIVITY(ConfirmationOverlayDemo);
