#ifndef __DESKCLOCK_RINGTONEPICKERACTIVITY_H__
#define __DESKCLOCK_RINGTONEPICKERACTIVITY_H__
/*********************************************************************************
 * Port of com.android.deskclock.ringtone.RingtonePickerActivity — presents a
 * set of ringtones (bundled device sounds, silence, the default ringtone and
 * user-added custom ringtones) from which the user may select one.
 *
 * CDROID notes (documented deviations):
 *  - LoaderManager/AsyncTaskLoader are not ported: RingtoneLoader runs
 *    synchronously and feeds onLoadFinished directly.
 *  - "Add new" launches ACTION_OPEN_DOCUMENT (SAF) upstream; no file picker on
 *    cdroid — the click is recorded and consumed (DEFERRED).
 *  - onActivityResult / onSaveInstanceState are not ported (no SAF round-trip;
 *    Window has no state-save hook).
 *  - Uri extras ride as strings (the port-wide Uri convention).
 *********************************************************************************/
#include <core/intent.h>
#include <fragment/fragmentactivity.h>

#include <widgetEx/recyclerview/recyclerview.h>

#include <actionbarmenu.h>
#include <dropshadowcontroller.h>
#include <itemadapter.h>

namespace cdroid {
namespace deskclock {
namespace data {
class Alarm;
} // namespace data

namespace ringtone {

class RingtoneHolder;

class RingtonePickerActivity : public FragmentActivity {
private:
    /** The controller that shows the drop shadow when content is not scrolled to the top. */
    DropShadowController* mDropShadowController = nullptr;

    /** Generates the items in the activity context menu. */
    actionbarmenu::OptionsMenuManager mOptionsMenuManager;

    /** Displays a set of selectable ringtones. */
    RecyclerView* mRecyclerView = nullptr;

    /** Stores the set of ItemHolders that wrap the selectable ringtones. */
    ItemAdapter<ItemHolder>* mRingtoneAdapter = nullptr;

    /** The title of the default ringtone. */
    std::string mDefaultRingtoneTitle;

    /** The uri of the default ringtone. */
    std::string mDefaultRingtoneUri;

    /** The uri of the ringtone to select after data is loaded (empty + flag = none). */
    std::string mSelectedRingtoneUri;
    bool mHasSelectedRingtoneUri = false;

    /** true indicates the mSelectedRingtoneUri must be played after data load. */
    bool mIsPlaying = false;

    /** Identifies the alarm to receive the selected ringtone; -1 indicates there is no alarm. */
    int64_t mAlarmId = -1;

    /** The location of the custom ringtone to be removed. */
    int mIndexOfRingtoneToRemove = RecyclerView::NO_POSITION;

    /** Closes the context menu when the list scrolls (upstream OnScrollListener). */
    RecyclerView::OnScrollListener mCloseContextMenuOnScroll;

public:
    RingtonePickerActivity();
    ~RingtonePickerActivity() override;

    void onCreate(Bundle* savedInstanceState) override;
    void onResume() override;
    void onPause() override;
    void onStop() override;
    bool onCreateOptionsMenu(Menu& menu) override;
    bool onPrepareOptionsMenu(Menu& menu) override;
    bool onOptionsItemSelected(MenuItem& item) override;
    bool onContextItemSelected(MenuItem& item) override;

    // Upstream private; the confirm dialog reaches it through the Kotlin
    // inner-class privilege, which an anonymous-namespace C++ class lacks.
    void removeCustomRingtone(const std::string& toRemove);

    /** Key to an extra that defines resource id to the title of this activity. */
    static constexpr const char* EXTRA_TITLE = "extra_title";

    /** Key to an extra that identifies the alarm to which the selected ringtone is attached. */
    static constexpr const char* EXTRA_ALARM_ID = "extra_alarm_id";

    /** Key to an extra that identifies the selected ringtone. */
    static constexpr const char* EXTRA_RINGTONE_URI = "extra_ringtone_uri";

    /** Key to an extra that defines the uri representing the default ringtone. */
    static constexpr const char* EXTRA_DEFAULT_RINGTONE_URI = "extra_default_ringtone_uri";

    /** Key to an extra that defines the name of the default ringtone. */
    static constexpr const char* EXTRA_DEFAULT_RINGTONE_NAME = "extra_default_ringtone_name";

    /** @return an intent that launches the ringtone picker to edit the alarm's ringtone */
    static Intent createAlarmRingtonePickerIntent(Context& context, const data::Alarm& alarm);

    /** @return an intent that launches the ringtone picker to edit the ringtone of all timers */
    static Intent createTimerRingtonePickerIntent(Context& context);

private:
    /** LoaderManager stand-in: build the holder list and apply it. */
    void loadRingtoneData();

    /** The LoaderCallbacks.onLoadFinished body. */
    void onLoadFinished(std::vector<ItemHolder*>* itemHolders);

    RingtoneHolder* getRingtoneHolder(const std::string& uri) const;
    RingtoneHolder* getSelectedRingtoneHolder() const;

    /** The given ringtone will be selected as a side-effect of playing the ringtone. */
    void startPlayingRingtone(RingtoneHolder& ringtone);

    /**
     * @param deselect true indicates the ringtone should also be deselected;
     * false indicates its selection state should remain unchanged
     */
    void stopPlayingRingtone(RingtoneHolder* ringtone, bool deselect);

};

} // namespace ringtone
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_RINGTONEPICKERACTIVITY_H__
