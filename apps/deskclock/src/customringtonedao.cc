#include <customringtonedao.h>

namespace cdroid {
namespace deskclock {
namespace data {

CustomRingtone CustomRingtoneDAO::addCustomRingtone(SharedPreferences& prefs,
        const std::string& uri, const std::string& title) {
    const long id = (long) prefs.getLong(NEXT_RINGTONE_ID, 0);
    std::set<std::string> ids = getRingtoneIds(prefs);
    ids.insert(std::to_string(id));

    prefs.edit()
            .putString(std::string(RINGTONE_URI) + std::to_string(id), uri)
            .putString(std::string(RINGTONE_TITLE) + std::to_string(id), title)
            .putLong(NEXT_RINGTONE_ID, id + 1)
            .putStringSet(RINGTONE_IDS, ids)
            .apply();

    return CustomRingtone(id, uri, title, true);
}

void CustomRingtoneDAO::removeCustomRingtone(SharedPreferences& prefs, long id) {
    std::set<std::string> ids = getRingtoneIds(prefs);
    ids.erase(std::to_string(id));

    // Editor is an abstract interface (edit() returns a reference) — chain.
    if (ids.empty()) {
        prefs.edit()
                .remove(std::string(RINGTONE_URI) + std::to_string(id))
                .remove(std::string(RINGTONE_TITLE) + std::to_string(id))
                .remove(RINGTONE_IDS)
                .remove(NEXT_RINGTONE_ID)
                .apply();
    } else {
        prefs.edit()
                .remove(std::string(RINGTONE_URI) + std::to_string(id))
                .remove(std::string(RINGTONE_TITLE) + std::to_string(id))
                .putStringSet(RINGTONE_IDS, ids)
                .apply();
    }
}

std::vector<CustomRingtone> CustomRingtoneDAO::getCustomRingtones(SharedPreferences& prefs) {
    const std::set<std::string> ids = prefs.getStringSet(RINGTONE_IDS, {});
    std::vector<CustomRingtone> ringtones;
    ringtones.reserve(ids.size());

    for (const std::string& id : ids) {
        const long idLong = atol(id.c_str());
        const std::string uri = prefs.getString(std::string(RINGTONE_URI) + id, "");
        const std::string title = prefs.getString(std::string(RINGTONE_TITLE) + id, "");
        ringtones.push_back(CustomRingtone(idLong, uri, title, true));
    }
    return ringtones;
}

std::set<std::string> CustomRingtoneDAO::getRingtoneIds(SharedPreferences& prefs) {
    return prefs.getStringSet(RINGTONE_IDS, {});
}

} // namespace data
} // namespace deskclock
} // namespace cdroid
