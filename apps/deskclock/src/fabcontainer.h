#ifndef __DESKCLOCK_FABCONTAINER_H__
#define __DESKCLOCK_FABCONTAINER_H__
/*********************************************************************************
 * Ports of com.android.deskclock.FabContainer and FabController — the fab
 * update-flag vocabulary and the configure/click interfaces.
 *********************************************************************************/
#include <widget/button.h>
#include <widget/imageview.h>

namespace cdroid {
namespace deskclock {

class FabContainer {
public:
    // Bit fields for updates
    static constexpr int FAB_ANIMATION_MASK = 3;               // Bit 0-1
    /** Signals that the fab should be updated in place with no animation. */
    static constexpr int FAB_IMMEDIATE = 1;
    /** Signals the fab should be "animated away", updated, and "animated back". */
    static constexpr int FAB_SHRINK_AND_EXPAND = 2;
    /** Signals that the fab should morph into a new state in place. */
    static constexpr int FAB_MORPH = 3;
    static constexpr int FAB_REQUEST_FOCUS_MASK = 4;           // Bit 2
    /** Signals that the fab should request focus. */
    static constexpr int FAB_REQUEST_FOCUS = 4;
    static constexpr int BUTTONS_ANIMATION_MASK = 24;          // Bit 3-4
    /** Signals that the buttons should be updated in place with no animation. */
    static constexpr int BUTTONS_IMMEDIATE = 8;
    /** Signals that the buttons should be "animated away", updated, and "animated back". */
    static constexpr int BUTTONS_SHRINK_AND_EXPAND = 16;
    static constexpr int BUTTONS_DISABLE_MASK = 32;            // Bit 5
    /** Disable the buttons of the fab so they do not respond to clicks. */
    static constexpr int BUTTONS_DISABLE = 32;
    static constexpr int FAB_AND_BUTTONS_SHRINK_EXPAND_MASK = 192; // Bit 6-7
    /** Signals that the fab and buttons should be "animated away". */
    static constexpr int FAB_AND_BUTTONS_SHRINK = 128;
    /** Signals that the fab and buttons should be "animated back". */
    static constexpr int FAB_AND_BUTTONS_EXPAND = 64;

    // Convenience flags
    static constexpr int FAB_AND_BUTTONS_IMMEDIATE = FAB_IMMEDIATE | BUTTONS_IMMEDIATE;
    static constexpr int FAB_AND_BUTTONS_SHRINK_AND_EXPAND =
            FAB_SHRINK_AND_EXPAND | BUTTONS_SHRINK_AND_EXPAND;

    virtual ~FabContainer() = default;

    /** Requests that this container update the fab and/or its buttons. */
    virtual void updateFab(int updateTypes) = 0;
};

/** Implementers configure the fab and associated left/right buttons. */
class FabController {
public:
    virtual ~FabController() = default;

    /** Configures the display of the fab to match the current state. */
    virtual void onUpdateFab(ImageView& fab) = 0;

    /** Attaches click handler logic to the fab. */
    virtual void onFabClick(ImageView& fab) = 0;

    /** Configures the display of the left and right buttons. */
    virtual void onUpdateFabButtons(Button& left, Button& right) = 0;

    /** Attaches click handler logic to the left button. */
    virtual void onLeftButtonClick(Button& left) = 0;

    /** Attaches click handler logic to the right button. */
    virtual void onRightButtonClick(Button& right) = 0;

    /** Morphs the fab to a new state in place. */
    virtual void onMorphFab(ImageView& fab) {}
};

} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_FABCONTAINER_H__
