#include <headerholder.h>

#include <R.h>

#include <headerviewholder.h>

namespace cdroid {
namespace deskclock {
namespace ringtone {

int HeaderHolder::getItemViewType() const {
    return HeaderViewHolder::VIEW_TYPE_ITEM_HEADER;
}

} // namespace ringtone
} // namespace deskclock
} // namespace cdroid
