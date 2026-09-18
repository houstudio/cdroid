#include <deskclockfragment.h>

#include <fabcontainer.h>

namespace cdroid {
namespace deskclock {

void DeskClockFragment::onResume() {
    Fragment::onResume();

    // Update the fab and buttons in case their state changed while the fragment was paused.
    if (isTabSelected()) {
        updateFab(FabContainer::FAB_AND_BUTTONS_IMMEDIATE);
    }
}

bool DeskClockFragment::onKeyDown(int /*keyCode*/, KeyEvent& /*event*/) {
    // By default return false so event continues to propagate
    return false;
}

} // namespace deskclock
} // namespace cdroid
