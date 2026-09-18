#include <addcustomringtoneviewholder.h>

#include <R.h>

#include <widget/imageview.h>
#include <widget/textview.h>

using namespace ::deskclock;

namespace cdroid {
namespace deskclock {
namespace ringtone {

AddCustomRingtoneViewHolder::AddCustomRingtoneViewHolder(View* itemView)
    : ItemViewHolder(itemView) {
    itemView->setOnClickListener([this](View&) {
        // View.OnClickListener.onClick
        notifyItemClicked(CLICK_ADD_NEW);
    });

    View* selectedView = itemView->findViewById(R::id::sound_image_selected);
    selectedView->setVisibility(View::GONE);
    TextView* nameView = (TextView*) itemView->findViewById(R::id::ringtone_name);
    nameView->setText(itemView->getContext()->getString(R::string::add_new_sound));
    nameView->setAlpha(0.63f);
    ImageView* imageView = (ImageView*) itemView->findViewById(R::id::ringtone_image);
    imageView->setImageResource(R::drawable::ic_add_white_24dp);
    imageView->setAlpha(0.63f);
}

} // namespace ringtone
} // namespace deskclock
} // namespace cdroid
