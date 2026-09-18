#include <ringtoneviewholder.h>

#include <R.h>

#include <core/porterduff.h>
#include <menu/menu.h>
#include <widget/imageview.h>
#include <widget/textview.h>

#include <animatorutils.h>
#include <ringtoneholder.h>

using namespace ::deskclock;

namespace cdroid {
namespace deskclock {
namespace ringtone {

const int RingtoneViewHolder::VIEW_TYPE_SYSTEM_SOUND = R::layout::ringtone_item_sound;
const int RingtoneViewHolder::VIEW_TYPE_CUSTOM_SOUND = -R::layout::ringtone_item_sound;

RingtoneViewHolder::RingtoneViewHolder(View* itemView) : ItemViewHolder(itemView) {
    mSelectedView = itemView->findViewById(R::id::sound_image_selected);
    mNameView = (TextView*) itemView->findViewById(R::id::ringtone_name);
    mImageView = (ImageView*) itemView->findViewById(R::id::ringtone_image);

    itemView->setOnClickListener([this](View&) {
        RingtoneHolder* holder = dynamic_cast<RingtoneHolder*>(itemHolder);
        if (holder == nullptr) return;
        // View.OnClickListener.onClick
        if (holder->hasPermissions()) {
            notifyItemClicked(CLICK_NORMAL);
        } else {
            notifyItemClicked(CLICK_NO_PERMISSIONS);
        }
    });
}

void RingtoneViewHolder::onBindItemView(ItemHolder& itemHolder) {
    RingtoneHolder* holder = dynamic_cast<RingtoneHolder*>(&itemHolder);
    mNameView->setText(holder->name());

    const bool opaque = holder->isSelected || !holder->hasPermissions();
    mNameView->setAlpha(opaque ? 1.0f : .63f);
    mImageView->setAlpha(opaque ? 1.0f : .63f);
    mImageView->clearColorFilter();

    const int itemViewType = getItemViewType();
    if (itemViewType == VIEW_TYPE_CUSTOM_SOUND) {
        if (!holder->hasPermissions()) {
            mImageView->setImageResource(R::drawable::ic_ringtone_not_found);
            // ThemeUtils.resolveColor(context, android.R.attr.colorAccent)
            TypedValue value;
            const int colorAccent = itemView->getContext()->getTheme().resolveAttribute(
                    0x01010435 /* android:attr/colorAccent */, &value, true)
                            && value.type >= TypedValue::TYPE_FIRST_COLOR_INT
                    ? (int) value.data : 0xffff8080;
            mImageView->setColorFilter(colorAccent, PorterDuff::Mode::SRC_ATOP);
        } else {
            mImageView->setImageResource(R::drawable::placeholder_album_artwork);
        }
    } else if (holder->isSilent()) {   // Utils.RINGTONE_SILENT
        mImageView->setImageResource(R::drawable::ic_ringtone_silent);
    } else if (holder->isPlaying) {
        mImageView->setImageResource(R::drawable::ic_ringtone_active);
    } else {
        mImageView->setImageResource(R::drawable::ic_ringtone);
    }
    AnimatorUtils::startDrawableAnimation(*mImageView);

    mSelectedView->setVisibility(holder->isSelected ? View::VISIBLE : View::GONE);

    const int bgColorId = holder->isSelected ? R::color::white_08p : R::color::transparent;
    itemView->setBackgroundColor(itemView->getContext()->getColor(bgColorId));

    if (itemViewType == VIEW_TYPE_CUSTOM_SOUND) {
        // OnCreateContextMenuListener: long-press offers "Remove sound".
        itemView->setOnCreateContextMenuListener([this](ContextMenu& contextMenu, View&,
                ContextMenuInfo*) {
            notifyItemClicked(CLICK_LONG_PRESS);
            contextMenu.add(Menu::NONE, 0, Menu::NONE,
                    itemView->getContext()->getString(R::string::remove_sound));
        });
    }
}

} // namespace ringtone
} // namespace deskclock
} // namespace cdroid
