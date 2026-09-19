#include <headerviewholder.h>

#include <R.h>

#include <headerholder.h>
#include <widget/textview.h>

using namespace ::deskclock;

namespace cdroid {
namespace deskclock {
namespace ringtone {

// The view type IS the layout resource (the factory inflates it directly).
const int HeaderViewHolder::VIEW_TYPE_ITEM_HEADER = R::layout::ringtone_item_header;

HeaderViewHolder::HeaderViewHolder(View* itemView) : ItemViewHolder(itemView) {
    mItemHeader = (TextView*) itemView->findViewById(R::id::ringtone_item_header);
}

void HeaderViewHolder::onBindItemView(ItemHolder& itemHolder) {
    HeaderHolder* holder = dynamic_cast<HeaderHolder*>(&itemHolder);
    mItemHeader->setText(holder->textResId);
}

} // namespace ringtone
} // namespace deskclock
} // namespace cdroid
