#ifndef CDROID_ESPRESSO_HUMANREADABLES_H
#define CDROID_ESPRESSO_HUMANREADABLES_H

/*
 * android.support.test.espresso.util.HumanReadables — text converters for
 * various Android objects.
 *
 * CDROID omissions (APIs not ported; see the .cc):
 *  - describe(View): res-name uses Context::getResourceName; no
 *    InputConnection surface, so has-input-connection/editor-info and the
 *    Cursor/WebView describe() overloads are absent;
 *  - describe(Cursor): no Cursor in CDROID.
 */

#include <string>
#include <vector>

#include <view/view.h>

namespace cdroid {
namespace espresso {

class HumanReadables {
public:
    /**
     * Prints out an error message featuring the view hierarchy starting at
     * the rootView.
     *
     * @param rootView the root of the hierarchy tree to print out.
     * @param problemViews list of the views that you would like to point out
     *        are causing the error message or null to skip this feature.
     * @param errorHeader the header of the error message.
     * @param problemViewSuffix the message to append to the view description
     *        in the tree printout. Required if problemViews is supplied.
     */
    static std::string getViewHierarchyErrorMessage(View* rootView,
            const std::vector<View*>& problemViews, const std::string& errorHeader,
            const std::string& problemViewSuffix);

    /** Transforms an arbitrary view into a string with debug info. */
    static std::string describe(View* v);

private:
    HumanReadables() = default;
};

} /*endof namespace espresso*/
} /*endof namespace cdroid*/

#endif /*CDROID_ESPRESSO_HUMANREADABLES_H*/
