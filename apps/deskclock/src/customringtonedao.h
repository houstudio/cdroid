#ifndef __DESKCLOCK_CUSTOMRINGTONEDAO_H__
#define __DESKCLOCK_CUSTOMRINGTONEDAO_H__
/*********************************************************************************
 * Port of com.android.deskclock.data.CustomRingtoneDAO — transfer of data
 * between CustomRingtone domain objects and their SharedPreferences storage.
 *********************************************************************************/
#include <content/sharedpreferences.h>

#include <customringtone.h>

#include <set>
#include <string>
#include <vector>

namespace cdroid {
namespace deskclock {
namespace data {

class CustomRingtoneDAO {
private:
    /** Key to a preference that stores the set of all custom ringtone ids. */
    static constexpr const char* RINGTONE_IDS = "ringtone_ids";

    /** Key to a preference that stores the next unused ringtone id. */
    static constexpr const char* NEXT_RINGTONE_ID = "next_ringtone_id";

    /** Prefix for a key to a preference that stores the URI of a ringtone id. */
    static constexpr const char* RINGTONE_URI = "ringtone_uri_";

    /** Prefix for a key to a preference that stores the title of a ringtone id. */
    static constexpr const char* RINGTONE_TITLE = "ringtone_title_";

public:
    /**
     * @param uri points to an audio file located on the file system
     * @param title the title of the audio content at the given uri
     * @return the newly added custom ringtone
     */
    static CustomRingtone addCustomRingtone(SharedPreferences& prefs, const std::string& uri,
            const std::string& title);

    /** @param id identifies the ringtone to be removed */
    static void removeCustomRingtone(SharedPreferences& prefs, long id);

    /** @return a list of all known custom ringtones */
    static std::vector<CustomRingtone> getCustomRingtones(SharedPreferences& prefs);

private:
    static std::set<std::string> getRingtoneIds(SharedPreferences& prefs);
};

} // namespace data
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_CUSTOMRINGTONEDAO_H__
