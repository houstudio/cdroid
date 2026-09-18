#ifndef __DESKCLOCK_RINGTONEMODEL_H__
#define __DESKCLOCK_RINGTONEMODEL_H__
/*********************************************************************************
 * Port of com.android.deskclock.data.RingtoneModel — all ringtone data is
 * accessed via this model.
 *
 * CDROID notes (documented deviations):
 *  - There is no system RingtoneManager/MediaStore: the "system" alarm sounds
 *    are the app's bundled raw audio (the AOSP DeskClock set), exposed as
 *    android.resource:// uris the klaxon stack already plays.
 *  - No ContentObserver for Settings.System.DEFAULT_ALARM_ALERT and no
 *    ACTION_LOCALE_CHANGED receiver (no system settings provider/broadcasts);
 *    the title cache lives for the process lifetime instead.
 *  - loadRingtonePermissions() is a no-op: persisted-uri permissions are a SAF
 *    concept; custom ringtones keep the hasPermissions flag they were stored
 *    with.
 *********************************************************************************/
#include <core/context.h>
#include <content/sharedpreferences.h>

#include <customringtone.h>

#include <map>
#include <string>
#include <vector>

namespace cdroid {
namespace deskclock {
namespace data {

class RingtoneModel {
private:
    Context& mContext;
    SharedPreferences& mPrefs;

    /** Maps ringtone uri to ringtone title; looking up a title from scratch is expensive. */
    std::map<std::string, std::string> mRingtoneTitles;

    /** A mutable copy of the custom ringtones. */
    std::vector<CustomRingtone>* mCustomRingtones = nullptr;

public:
    RingtoneModel(Context& context, SharedPreferences& prefs);
    ~RingtoneModel();

    /** @return the newly added custom ringtone, or the existing one for the uri */
    const CustomRingtone* addCustomRingtone(const std::string& uri, const std::string& title);

    void removeCustomRingtone(const std::string& uri);

    /** @return a list of all custom ringtones */
    const std::vector<CustomRingtone>& getCustomRingtones();

    /**
     * The RingtoneManager(context).setType(STREAM_ALARM) stand-in: the
     * ringtone uris/titles of the device's alarm sounds.
     */
    const std::vector<std::pair<std::string, std::string>>& getSystemRingtones();

    /** No persisted-uri permissions on cdroid — DEFERRED (see header note). */
    void loadRingtonePermissions() {}

    /** Prime the ringtone title cache for later access. */
    void loadRingtoneTitles();

    /** @return the title of the ringtone with the given uri */
    std::string getRingtoneTitle(const std::string& uri);

private:
    const CustomRingtone* getCustomRingtone(const std::string& uri);
    std::vector<CustomRingtone>& mutableCustomRingtones();
};

} // namespace data
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_RINGTONEMODEL_H__
