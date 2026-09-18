#ifndef __DESKCLOCK_RINGTONEVIEWHOLDER_H__
#define __DESKCLOCK_RINGTONEVIEWHOLDER_H__
/*********************************************************************************
 * Port of com.android.deskclock.ringtone.RingtoneViewHolder.
 *********************************************************************************/
#include <itemadapter.h>

namespace cdroid {
class ImageView;
class TextView;

namespace deskclock {
namespace ringtone {

class RingtoneHolder;

class RingtoneViewHolder : public ItemViewHolder {
private:
    View* mSelectedView;
    TextView* mNameView;
    ImageView* mImageView;

public:
    explicit RingtoneViewHolder(View* itemView);

    static const int VIEW_TYPE_SYSTEM_SOUND;   // = R::layout::ringtone_item_sound
    static const int VIEW_TYPE_CUSTOM_SOUND;   // = -R::layout::ringtone_item_sound
    static constexpr int CLICK_NORMAL = 0;
    static constexpr int CLICK_LONG_PRESS = -1;
    static constexpr int CLICK_NO_PERMISSIONS = -2;

protected:
    void onBindItemView(ItemHolder& itemHolder) override;
};

} // namespace ringtone
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_RINGTONEVIEWHOLDER_H__
