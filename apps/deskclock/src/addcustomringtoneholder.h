#ifndef __DESKCLOCK_ADDCUSTOMRINGTONEHOLDER_H__
#define __DESKCLOCK_ADDCUSTOMRINGTONEHOLDER_H__
/*********************************************************************************
 * Port of com.android.deskclock.ringtone.AddCustomRingtoneHolder.
 *********************************************************************************/
#include <itemadapter.h>

namespace cdroid {
namespace deskclock {
namespace ringtone {

class AddCustomRingtoneHolder : public ItemHolder {
public:
    AddCustomRingtoneHolder() : ItemHolder(RecyclerView::NO_ID) {}

    int getItemViewType() const override;
};

} // namespace ringtone
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_ADDCUSTOMRINGTONEHOLDER_H__
