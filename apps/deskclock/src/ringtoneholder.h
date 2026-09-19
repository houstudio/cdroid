#ifndef __DESKCLOCK_RINGTONEHOLDER_H__
#define __DESKCLOCK_RINGTONEHOLDER_H__
/*********************************************************************************
 * Port of com.android.deskclock.ringtone.RingtoneHolder.
 *
 * CDROID note: the uri is the ItemHolder item (upstream ItemHolder<Uri?>);
 * it is held as a string per the port-wide Uri convention.
 *********************************************************************************/
#include <itemadapter.h>

#include <string>

namespace cdroid {
namespace deskclock {
namespace ringtone {

class RingtoneHolder : public ItemHolder {
private:
    const std::string mName;
    const bool mHasPermissions;

public:
    /** The uri that allows playback of the ringtone (the ItemHolder item). */
    const std::string uri;

    bool isSelected = false;
    bool isPlaying = false;

    RingtoneHolder(const std::string& uri, const std::string& name,
            bool hasPermissions = true);

    long id() const { return itemId; }

    bool hasPermissions() const { return mHasPermissions; }

    bool isSilent() const;   // Utils.RINGTONE_SILENT == uri (the empty uri)

    /** @return the given name, or the DataModel's title for the uri. */
    std::string name() const;
};

} // namespace ringtone
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_RINGTONEHOLDER_H__
