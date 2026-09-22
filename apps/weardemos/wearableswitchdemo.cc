// Port of SupportWearDemos WearableSwitchDemo: just the centered Switch
// (see switch_demo.xml for the dropped wear style).
#include <core/app.h>
#include <cdroid.h>
#include <core/activityfactory.h>
#include "R.h"

using namespace cdroid;

class WearableSwitchDemo : public Window {
public:
    WearableSwitchDemo() : Window(&App::getInstance(), 0, 0, -1, -1) {
        addView(LayoutInflater::from(getContext())
                ->inflate(weardemos::R::layout::switch_demo, this, false));
    }
};
REGISTER_ACTIVITY(WearableSwitchDemo);
