#ifndef __DESKCLOCK_CUSTOMRINGTONE_H__
#define __DESKCLOCK_CUSTOMRINGTONE_H__
/*********************************************************************************
 * Port of com.android.deskclock.data.CustomRingtone — a read-only domain object
 * representing a custom ringtone chosen from the file system.
 *
 * CDROID note: the uri crosses the DAO/model seam as a string (the port-wide
 * Uri convention; Uri objects are parsed at the playback boundary).
 *********************************************************************************/
#include <string>

namespace cdroid {
namespace deskclock {
namespace data {

class CustomRingtone {
public:
    /** The unique identifier of the custom ringtone (non-const: the Timer
     *  domain-object convention — vectors of these must stay movable). */
    long id;

    /** The uri that allows playback of the ringtone. */
    std::string uri;

    /** The title describing the file at the given uri; typically the file name. */
    std::string title;

    /** @return true iff the application has permission to read the content of uri. */
    bool hasPermissions() const { return mHasPermissions; }

    /** @return this, or a copy with the new permissions flag (immutable upstream). */
    CustomRingtone setHasPermissions(bool hasPermissions) const {
        return hasPermissions == mHasPermissions ? *this
                : CustomRingtone(id, uri, title, hasPermissions);
    }

    /** Comparable contract: case-insensitive title order. */
    int compareTo(const CustomRingtone& other) const;

    CustomRingtone(long id, const std::string& uri, const std::string& title,
            bool hasPermissions)
        : id(id), uri(uri), title(title), mHasPermissions(hasPermissions) {}

private:
    bool mHasPermissions;
};

} // namespace data
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_CUSTOMRINGTONE_H__
