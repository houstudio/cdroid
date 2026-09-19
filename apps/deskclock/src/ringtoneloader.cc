#include <ringtoneloader.h>

#include <R.h>

#include <addcustomringtoneholder.h>
#include <customringtoneholder.h>
#include <datamodel.h>
#include <headerholder.h>
#include <systemringtoneholder.h>

using namespace ::deskclock;

namespace cdroid {
namespace deskclock {
namespace ringtone {

RingtoneLoader::RingtoneLoader(Context& context, const std::string& defaultRingtoneUri,
        const std::string& defaultRingtoneTitle)
    : mContext(context), mDefaultRingtoneUri(defaultRingtoneUri),
      mDefaultRingtoneTitle(defaultRingtoneTitle) {
}

std::vector<ItemHolder*>* RingtoneLoader::loadInBackground() {
    mCustomRingtones = data::DataModel::getDataModel().getCustomRingtones();

    // Prime the ringtone title cache for later access.
    data::DataModel::getDataModel().loadRingtoneTitles();
    data::DataModel::getDataModel().loadRingtonePermissions();

    // Fetch the standard system ringtones (the bundled raw alarm set).
    const std::vector<std::pair<std::string, std::string>>& systemRingtones =
            data::DataModel::getDataModel().getSystemRingtones();

    // item count = # system ringtones + # custom ringtones + 2 headers + Add new music item
    auto* itemHolders = new std::vector<ItemHolder*>();
    itemHolders->reserve(systemRingtones.size() + mCustomRingtones.size() + 3);

    // Add the item holder for the Music heading.
    itemHolders->push_back(new HeaderHolder(R::string::your_sounds));

    // Add an item holder for each custom ringtone and also cache a pretty name.
    for (const data::CustomRingtone& ringtone : mCustomRingtones) {
        itemHolders->push_back(new CustomRingtoneHolder(ringtone));
    }

    // Add an item holder for the "Add new" music ringtone.
    itemHolders->push_back(new AddCustomRingtoneHolder());

    // Add an item holder for the Ringtones heading.
    itemHolders->push_back(new HeaderHolder(R::string::device_sounds));

    // Add an item holder for the silent ringtone.
    itemHolders->push_back(new SystemRingtoneHolder(std::string() /* Utils.RINGTONE_SILENT */,
            std::string()));

    // Add an item holder for the system default alarm sound.
    itemHolders->push_back(new SystemRingtoneHolder(mDefaultRingtoneUri, mDefaultRingtoneTitle));

    // Add an item holder for each system ringtone.
    for (const auto& ringtone : systemRingtones) {
        itemHolders->push_back(new SystemRingtoneHolder(ringtone.first, std::string()));
    }

    return itemHolders;
}

} // namespace ringtone
} // namespace deskclock
} // namespace cdroid
