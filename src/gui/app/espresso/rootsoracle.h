#ifndef CDROID_ESPRESSO_ROOTSORACLE_H
#define CDROID_ESPRESSO_ROOTSORACLE_H

/*
 * android.support.test.espresso.base.RootsOracle (+ ActiveRootLister) —
 * lists the active Roots. AOSP pulls root views from
 * WindowManagerGlobal.getRootViews(activity); CDROID's WindowManager window
 * stack plays that role (a CDROID Window IS the root view of its tree, and
 * Window::getAttributes() carries the mirrored window type).
 */

#include <vector>

#include <app/espresso/root.h>

namespace cdroid {
namespace espresso {

class RootsOracle {
public:
    using ActiveRootLister = std::vector<Root> (*)();

    /** Lists all active roots (AOSP listActiveRoots). */
    std::vector<Root> listActiveRoots();
};

} /*endof namespace espresso*/
} /*endof namespace cdroid*/

#endif /*CDROID_ESPRESSO_ROOTSORACLE_H*/
