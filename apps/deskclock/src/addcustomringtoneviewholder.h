#ifndef __DESKCLOCK_ADDCUSTOMRINGTONEVIEWHOLDER_H__
#define __DESKCLOCK_ADDCUSTOMRINGTONEVIEWHOLDER_H__
/*********************************************************************************
 * Port of com.android.deskclock.ringtone.AddCustomRingtoneViewHolder.
 *********************************************************************************/
#include <itemadapter.h>

#include <climits>

namespace cdroid {
namespace deskclock {
namespace ringtone {

class AddCustomRingtoneViewHolder : public ItemViewHolder {
public:
    explicit AddCustomRingtoneViewHolder(View* itemView);

    static constexpr int VIEW_TYPE_ADD_NEW = INT_MIN;
    static constexpr int CLICK_ADD_NEW = INT_MIN;
};

} // namespace ringtone
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_ADDCUSTOMRINGTONEVIEWHOLDER_H__
