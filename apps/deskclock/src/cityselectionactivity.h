#ifndef __DESKCLOCK_CITYSELECTIONACTIVITY_H__
#define __DESKCLOCK_CITYSELECTIONACTIVITY_H__
/*********************************************************************************
 * Port of com.android.deskclock.worldclock.CitySelectionActivity — allows the
 * user to alter the cities selected for display.
 *
 * CDROID notes: extends Window (the Activity); the androidx SearchView is not
 * ported, so the search controller uses a plain EditText action view; the
 * SectionIndexer fast-scroll face is not ported (section indexes render, the
 * thumb does not).
 *********************************************************************************/
#include <widget/cdwindow.h>
#include <widget/listview.h>

#include <actionbarmenu.h>
#include <dropshadowcontroller.h>
#include <searchmenuitemcontroller.h>

namespace cdroid {
namespace deskclock {
namespace worldclock {

class CitySelectionActivity : public Window {
private:
    class CityAdapter;

private:
    /** The list of all selected and unselected cities, indexed and possibly filtered. */
    ListView* mCitiesList = nullptr;

    /** The adapter that presents all of the selected and unselected cities. */
    CityAdapter* mCitiesAdapter = nullptr;

    /** Manages all action bar menu display and click handling. */
    actionbarmenu::OptionsMenuManager mOptionsMenuManager;

    /** Menu item controller for the search view. */
    actionbarmenu::SearchMenuItemController* mSearchMenuItemController = nullptr;

    /** The controller that shows the drop shadow when content is not scrolled to the top. */
    DropShadowController* mDropShadowController = nullptr;

public:
    CitySelectionActivity();
    ~CitySelectionActivity() override;

    void onCreate(Bundle* savedInstanceState) override;
    void onResume() override;
    void onPause() override;
    bool onCreateOptionsMenu(Menu& menu) override;
    bool onPrepareOptionsMenu(Menu& menu) override;
    bool onOptionsItemSelected(MenuItem& item) override;

    /** SortOrderMenuItemController hook: honor a new city sort order. */
    void onSortOrderChanged();

private:
    /** Fast scrolling is only enabled while no filtering is happening. */
    void updateFastScrolling();
};

} // namespace worldclock
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_CITYSELECTIONACTIVITY_H__
