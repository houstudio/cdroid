#include <ringtoneholder.h>

#include <datamodel.h>

namespace cdroid {
namespace deskclock {
namespace ringtone {

RingtoneHolder::RingtoneHolder(const std::string& uri, const std::string& name,
        bool hasPermissions)
    : ItemHolder(RecyclerView::NO_ID), mName(name), mHasPermissions(hasPermissions), uri(uri) {
}

bool RingtoneHolder::isSilent() const {
    return uri.empty();   // Utils.RINGTONE_SILENT (Uri.EMPTY)
}

std::string RingtoneHolder::name() const {
    return !mName.empty() ? mName : data::DataModel::getDataModel().getRingtoneTitle(uri);
}

} // namespace ringtone
} // namespace deskclock
} // namespace cdroid
