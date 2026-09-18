#ifndef __DESKCLOCK_RINGTONELOADER_H__
#define __DESKCLOCK_RINGTONELOADER_H__
/*********************************************************************************
 * Port of com.android.deskclock.ringtone.RingtoneLoader — assembles the list of
 * ItemHolders that back the RecyclerView used to choose a ringtone.
 *
 * CDROID note: no androidx LoaderManager/AsyncTaskLoader (loaders were
 * synchronized across the port); loadInBackground() runs synchronously on the
 * main thread. The system-ringtone cursor (RingtoneManager/MediaStore) is the
 * DataModel's bundled raw alarm set.
 *********************************************************************************/
#include <core/context.h>

#include <customringtone.h>
#include <itemadapter.h>

#include <string>
#include <vector>

namespace cdroid {
namespace deskclock {
namespace ringtone {

class RingtoneLoader {
private:
    Context& mContext;
    const std::string mDefaultRingtoneUri;
    const std::string mDefaultRingtoneTitle;
    std::vector<data::CustomRingtone> mCustomRingtones;

public:
    RingtoneLoader(Context& context, const std::string& defaultRingtoneUri,
            const std::string& defaultRingtoneTitle);

    /** @return the heap list of holders (the adapter takes ownership). */
    std::vector<ItemHolder*>* loadInBackground();
};

} // namespace ringtone
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_RINGTONELOADER_H__
