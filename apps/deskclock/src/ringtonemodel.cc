#include <ringtonemodel.h>

#include <porting/cdlog.h>

#include <R.h>
#include <content/resources.h>
#include <customringtonedao.h>

#include <algorithm>

using namespace ::deskclock;

namespace cdroid {
namespace deskclock {
namespace data {

RingtoneModel::RingtoneModel(Context& context, SharedPreferences& prefs)
    : mContext(context), mPrefs(prefs) {
}

RingtoneModel::~RingtoneModel() {
    delete mCustomRingtones;
}

const CustomRingtone* RingtoneModel::addCustomRingtone(const std::string& uri,
        const std::string& title) {
    // If the uri is already present in an existing ringtone, do nothing.
    const CustomRingtone* existing = getCustomRingtone(uri);
    if (existing != nullptr) {
        return existing;
    }

    std::vector<CustomRingtone>& ringtones = mutableCustomRingtones();
    ringtones.push_back(CustomRingtoneDAO::addCustomRingtone(mPrefs, uri, title));
    std::stable_sort(ringtones.begin(), ringtones.end(),
            [](const CustomRingtone& lhs, const CustomRingtone& rhs) {
        return lhs.compareTo(rhs) < 0;
    });
    return &ringtones.back();
}

void RingtoneModel::removeCustomRingtone(const std::string& uri) {
    std::vector<CustomRingtone>& ringtones = mutableCustomRingtones();
    for (auto it = ringtones.begin(); it != ringtones.end(); ++it) {
        if (it->uri == uri) {
            CustomRingtoneDAO::removeCustomRingtone(mPrefs, it->id);
            ringtones.erase(it);
            break;
        }
    }
}

const CustomRingtone* RingtoneModel::getCustomRingtone(const std::string& uri) {
    for (const CustomRingtone& ringtone : mutableCustomRingtones()) {
        if (ringtone.uri == uri) {
            return &ringtone;
        }
    }
    return nullptr;
}

const std::vector<CustomRingtone>& RingtoneModel::getCustomRingtones() {
    return mutableCustomRingtones();
}

const std::vector<std::pair<std::string, std::string>>& RingtoneModel::getSystemRingtones() {
    // RingtoneManager(STREAM_ALARM) stand-in (see header note): the app's
    // bundled raw alarm sounds as resource uris, titled by resource entry
    // name (the honest title; no media-store metadata on cdroid).
    static std::vector<std::pair<std::string, std::string>> systemRingtones;
    if (systemRingtones.empty()) {
        for (int resId : {R::raw::alarm_expire, R::raw::timer_expire}) {
            const std::string uri = "android.resource://cdroid.deskclock/"
                    + std::to_string(resId);
            std::string entryName;
            mContext.getResources().getResourceEntryName(resId, &entryName);
            systemRingtones.push_back({uri, entryName});
        }
    }
    return systemRingtones;
}

void RingtoneModel::loadRingtoneTitles() {
    // Early return if the cache is already primed.
    if (!mRingtoneTitles.empty()) {
        return;
    }

    // Cache a title for each system ringtone.
    try {
        for (const auto& ringtone : getSystemRingtones()) {
            mRingtoneTitles[ringtone.first] = ringtone.second;
        }
    } catch (const std::exception& ignored) {
        // best attempt only
        LOGE("Error loading ringtone title cache");
    }
}

std::string RingtoneModel::getRingtoneTitle(const std::string& uri) {
    // Special case: no ringtone has a title of "Silent".
    if (uri.empty()) {   // AlarmSettingColumns.NO_RINGTONE_URI == Utils.RINGTONE_SILENT
        return mContext.getString(R::string::silent_ringtone_title);
    }

    // If the ringtone is custom, it has its own title.
    const CustomRingtone* customRingtone = getCustomRingtone(uri);
    if (customRingtone != nullptr) {
        return customRingtone->title;
    }

    // Check the cache; a miss falls back to the resource entry name for
    // resource uris (upstream spins up a media player via Ringtone.getTitle).
    auto it = mRingtoneTitles.find(uri);
    if (it != mRingtoneTitles.end()) {
        return it->second;
    }
    LOGE("No ringtone for uri: %s", uri.c_str());
    return mContext.getString(R::string::unknown_ringtone_title);
}

std::vector<CustomRingtone>& RingtoneModel::mutableCustomRingtones() {
    if (mCustomRingtones == nullptr) {
        mCustomRingtones = new std::vector<CustomRingtone>(
                CustomRingtoneDAO::getCustomRingtones(mPrefs));
        std::stable_sort(mCustomRingtones->begin(), mCustomRingtones->end(),
                [](const CustomRingtone& lhs, const CustomRingtone& rhs) {
            return lhs.compareTo(rhs) < 0;
        });
    }
    return *mCustomRingtones;
}

} // namespace data
} // namespace deskclock
} // namespace cdroid
