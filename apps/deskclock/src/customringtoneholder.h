#ifndef __DESKCLOCK_CUSTOMRINGTONEHOLDER_H__
#define __DESKCLOCK_CUSTOMRINGTONEHOLDER_H__
/*********************************************************************************
 * Port of com.android.deskclock.ringtone.CustomRingtoneHolder.
 *********************************************************************************/
#include <ringtoneholder.h>

#include <customringtone.h>

namespace cdroid {
namespace deskclock {
namespace ringtone {

class CustomRingtoneHolder : public RingtoneHolder {
public:
    explicit CustomRingtoneHolder(const data::CustomRingtone& ringtone);

    int getItemViewType() const override;
};

} // namespace ringtone
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_CUSTOMRINGTONEHOLDER_H__
