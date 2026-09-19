// radio-browser.info community API client (key-free): search stations by
// tag, delivering the parsed list on the MAIN looper. The fetch runs on a
// detached worker thread (libcurl); failures deliver an empty list.
#ifndef __REMUSIC_RADIOBROWSER_H__
#define __REMUSIC_RADIOBROWSER_H__

#include <functional>
#include <string>
#include <vector>

namespace remusic {

struct RadioStation {
    std::string name;     // station name
    std::string url;      // direct stream address (url_resolved)
    std::string tags;
    std::string codec;
    std::string country;
    std::string favicon;  // station logo URL
    int bitrate = 0;      // kbps
};

class RadioBrowser {
public:
    /** Fetch page size: one 200-row page covers most tags outright (radio
     *  directories are small — tag=chinese totals 21); the panel's infinite
     *  scroll remains the safety net for the few huge ones. */
    static constexpr int kPageSize = 200;

    static void searchByTag(const std::string& tag,
            std::function<void(std::vector<RadioStation>)> onDone, int offset = 0);
    /** NetSearchWordsActivity's stand-in: stations whose name contains term. */
    static void searchByName(const std::string& term,
            std::function<void(std::vector<RadioStation>)> onDone, int offset = 0);
};

} // namespace remusic
#endif
