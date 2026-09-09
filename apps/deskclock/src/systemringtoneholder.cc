#include <systemringtoneholder.h>

#include <ringtoneviewholder.h>

namespace cdroid {
namespace deskclock {
namespace ringtone {

SystemRingtoneHolder::SystemRingtoneHolder(const std::string& uri, const std::string& name)
    : RingtoneHolder(uri, name) {
}

int SystemRingtoneHolder::getItemViewType() const {
    return RingtoneViewHolder::VIEW_TYPE_SYSTEM_SOUND;
}

} // namespace ringtone
} // namespace deskclock
} // namespace cdroid
