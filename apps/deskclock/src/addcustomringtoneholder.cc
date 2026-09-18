#include <addcustomringtoneholder.h>

#include <addcustomringtoneviewholder.h>

namespace cdroid {
namespace deskclock {
namespace ringtone {

int AddCustomRingtoneHolder::getItemViewType() const {
    return AddCustomRingtoneViewHolder::VIEW_TYPE_ADD_NEW;
}

} // namespace ringtone
} // namespace deskclock
} // namespace cdroid
