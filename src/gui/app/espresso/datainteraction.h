#ifndef CDROID_ESPRESSO_DATAINTERACTION_H
#define CDROID_ESPRESSO_DATAINTERACTION_H

/*
 * android.support.test.espresso.DataInteraction — an interface to interact
 * with data displayed in AdapterViews.
 *
 * This interface builds on top of ViewInteraction and should be the
 * preferred way to interact with elements displayed inside AdapterViews.
 * This is necessary because an AdapterView may not load all the data held
 * by its Adapter into the view hierarchy until a user interaction makes it
 * necessary. Also it is more fluent / less brittle to match upon the data
 * object being rendered into the display then the rendering itself.
 *
 * By default, a DataInteraction takes place against any AdapterView found
 * within the current screen, if you have multiple AdapterView objects
 * displayed, you will need to narrow the selection by using the
 * inAdapterView method.
 *
 * The check and perform method operate on the top level child of the adapter
 * view, if you need to operate on a subview (eg: a Button within the list)
 * use the onChildView method before calling perform or check.
 *
 * CDROID adaptation: the data model is void* (Adapter::getItem), so the
 * data matcher is MatcherPtr<void*>; @Nullable Integer atPosition maps to a
 * (has, value) pair. The remote-serialization constructors and the
 * EspressoOptional deprecated surface are not ported.
 */

#include <memory>
#include <vector>

#include <app/espresso/adapterviewprotocol.h>
#include <app/espresso/hamcrest.h>
#include <app/espresso/rootmatchers.h>
#include <app/espresso/viewaction.h>

namespace cdroid {
class View;

namespace espresso {

class ViewInteraction;
class ViewAssertion;

class DataInteraction {
public:
    explicit DataInteraction(MatcherPtr<void*> dataMatcher);

    /** Causes perform and check methods to take place on a specific child
     *  view of the view returned by Adapter.getView(). */
    DataInteraction& onChildView(MatcherPtr<View> childMatcher);

    /** Causes this data interaction to work within the Root specified by
     *  the given root matcher. */
    DataInteraction& inRoot(RootMatcherPtr rootMatcher);

    /** Selects a particular adapter view to operate on, by default we
     *  operate on any adapter view on the screen. */
    DataInteraction& inAdapterView(MatcherPtr<View> adapterMatcher);

    /** Selects the view which matches the nth position on the adapter based
     *  on the data matcher. */
    DataInteraction& atPosition(int atPosition);

    /** Use a different AdapterViewProtocol if the Adapter implementation
     *  does not satisfy the AdapterView contract like ExpandableListView. */
    DataInteraction& usingAdapterViewProtocol(
            std::shared_ptr<AdapterViewProtocol> adapterViewProtocol);

    /** Performs an action on the view after we force the data to be loaded.
     *  @return a ViewInteraction for more assertions or actions. */
    ViewInteraction perform(const std::vector<ViewActionPtr>& actions);

    /** Performs an assertion on the state of the view after we force the
     *  data to be loaded.
     *  @return a ViewInteraction for more assertions or actions. */
    ViewInteraction check(std::shared_ptr<ViewAssertion> assertion);

    /**
     * Internal matcher that is required for Espresso.onData(). This matcher
     * is only visible to support proto serialization — do not use this
     * matcher in any Espresso test code!
     */
    class DisplayDataMatcher;

private:
    MatcherPtr<View> makeTargetMatcher();

    MatcherPtr<void*> mDataMatcher;
    MatcherPtr<View> mAdapterMatcher;
    MatcherPtr<View> mChildViewMatcher;  // @Nullable
    bool mHasAtPosition = false;
    int mAtPosition = 0;
    std::shared_ptr<AdapterViewProtocol> mAdapterViewProtocol;
    RootMatcherPtr mRootMatcher;
};

} /*endof namespace espresso*/
} /*endof namespace cdroid*/

#endif /*CDROID_ESPRESSO_DATAINTERACTION_H*/
