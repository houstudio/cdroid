#include <customringtoneholder.h>

#include <ringtoneviewholder.h>

namespace cdroid {
namespace deskclock {
namespace ringtone {

CustomRingtoneHolder::CustomRingtoneHolder(const data::CustomRingtone& ringtone)
    : RingtoneHolder(ringtone.uri, ringtone.title, ringtone.hasPermissions()) {
}

int CustomRingtoneHolder::getItemViewType() const {
    return RingtoneViewHolder::VIEW_TYPE_CUSTOM_SOUND;
}

} // namespace ringtone
} // namespace deskclock
} // namespace cdroid
