#ifndef __DESKCLOCK_HEADERVIEWHOLDER_H__
#define __DESKCLOCK_HEADERVIEWHOLDER_H__
/*********************************************************************************
 * Port of com.android.deskclock.ringtone.HeaderViewHolder.
 *********************************************************************************/
#include <itemadapter.h>

namespace cdroid {
class TextView;

namespace deskclock {
namespace ringtone {

class HeaderHolder;

class HeaderViewHolder : public ItemViewHolder {
private:
    TextView* mItemHeader;

public:
    explicit HeaderViewHolder(View* itemView);

    static const int VIEW_TYPE_ITEM_HEADER;   // = R::layout::ringtone_item_header

protected:
    void onBindItemView(ItemHolder& itemHolder) override;
};

} // namespace ringtone
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_HEADERVIEWHOLDER_H__
