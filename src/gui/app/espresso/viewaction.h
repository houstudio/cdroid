#ifndef CDROID_ESPRESSO_VIEWACTION_H
#define CDROID_ESPRESSO_VIEWACTION_H

/*
 * android.support.test.espresso.ViewAction — responsible for performing an
 * interaction on the given View.
 */

#include <memory>
#include <string>

#include <app/espresso/hamcrest.h>

namespace cdroid {
class View;

namespace espresso {

class UiController;

class ViewAction {
public:
    virtual ~ViewAction() = default;

    /** A view must satisfy these constraints before the action is performed. */
    virtual MatcherPtr<View> getConstraints() = 0;

    /** Performs this action on the given view. */
    virtual void perform(UiController& uiController, View& view) = 0;

    /** A description of what the action did. */
    virtual std::string getDescription() = 0;
};

using ViewActionPtr = std::shared_ptr<ViewAction>;

} /*endof namespace espresso*/
} /*endof namespace cdroid*/

#endif /*CDROID_ESPRESSO_VIEWACTION_H*/
