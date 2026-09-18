#ifndef __DESKCLOCK_HEADERHOLDER_H__
#define __DESKCLOCK_HEADERHOLDER_H__
/*********************************************************************************
 * Port of com.android.deskclock.ringtone.HeaderHolder.
 *********************************************************************************/
#include <itemadapter.h>

namespace cdroid {
namespace deskclock {
namespace ringtone {

class HeaderViewHolder;

class HeaderHolder : public ItemHolder {
public:
    const int textResId;

    explicit HeaderHolder(int textResId)
        : ItemHolder(RecyclerView::NO_ID), textResId(textResId) {}

    int getItemViewType() const override;
};

} // namespace ringtone
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_HEADERHOLDER_H__
