#ifndef CDROID_ESPRESSO_ADAPTERDATALOADERACTION_H
#define CDROID_ESPRESSO_ADAPTERDATALOADERACTION_H

/*
 * android.support.test.espresso.action.AdapterDataLoaderAction —
 * forces an AdapterView to ensure that the data matching a provided data
 * matcher is loaded into the current view hierarchy.
 */

#include <memory>

#include <app/espresso/adapterviewprotocol.h>
#include <app/espresso/hamcrest.h>
#include <app/espresso/viewaction.h>

namespace cdroid {
class View;

namespace espresso {

class AdapterDataLoaderAction : public ViewAction {
public:
    /**
     * AOSP's @Nullable Integer atPosition maps to a (value, has) pair —
     * Matcher<? extends Object> maps to MatcherPtr<void*> (Adapter::getItem
     * hands out void*).
     */
    AdapterDataLoaderAction(MatcherPtr<void*> dataToLoadMatcher,
            bool hasAtPosition, int atPosition,
            std::shared_ptr<AdapterViewProtocol> adapterViewProtocol);

    /** The data this action loaded; only valid after perform(). */
    const AdaptedData& getAdaptedData() const;

    MatcherPtr<View> getConstraints() override;
    void perform(UiController& uiController, View& view) override;
    std::string getDescription() override;

private:
    MatcherPtr<void*> mDataToLoadMatcher;
    bool mHasAtPosition;
    int mAtPosition;
    std::shared_ptr<AdapterViewProtocol> mAdapterViewProtocol;

    // AdaptedData has no default state (AOSP builds it only via Builder);
    // null until perform() selects the matched item.
    std::shared_ptr<AdaptedData> mAdaptedData;
    bool mPerformed = false;
};

} /*endof namespace espresso*/
} /*endof namespace cdroid*/

#endif /*CDROID_ESPRESSO_ADAPTERDATALOADERACTION_H*/
