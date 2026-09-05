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
    int bitrate = 0;      // kbps
};

class RadioBrowser {
public:
    static void searchByTag(const std::string& tag,
            std::function<void(std::vector<RadioStation>)> onDone);
};

} // namespace remusic
#endif
