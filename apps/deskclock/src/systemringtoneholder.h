#ifndef __DESKCLOCK_SYSTEMRINGTONEHOLDER_H__
#define __DESKCLOCK_SYSTEMRINGTONEHOLDER_H__
/*********************************************************************************
 * Port of com.android.deskclock.ringtone.SystemRingtoneHolder.
 *********************************************************************************/
#include <ringtoneholder.h>

namespace cdroid {
namespace deskclock {
namespace ringtone {

class SystemRingtoneHolder : public RingtoneHolder {
public:
    SystemRingtoneHolder(const std::string& uri, const std::string& name);

    int getItemViewType() const override;
};

} // namespace ringtone
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_SYSTEMRINGTONEHOLDER_H__
