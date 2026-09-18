#include "radiobrowser.h"

#ifdef REMUSIC_ONLINE

#include <sstream>
#include <cstring>
#include <thread>

#include <json/json.h>

#include "httputil.h"
#include <core/handler.h>
#include <core/looper.h>
#include <porting/cdlog.h>

namespace remusic {

static void searchStations(const std::string& query,
        std::function<void(std::vector<RadioStation>)> onDone, int offset) {
    std::thread([query, offset, onDone = std::move(onDone)]() mutable {
        const std::string url =
                "https://de1.api.radio-browser.info/json/stations/search"
                "?limit=" + std::to_string(RadioBrowser::kPageSize)
                + "&order=clickcount&reverse=true&hidebroken=true&offset="
                + std::to_string(offset) + "&" + query;
        std::vector<RadioStation> stations;
        const std::string body = httpGet(url);
        Json::Value root;
        Json::CharReaderBuilder rb;
        std::string errs;
        std::istringstream in(body);
        if (body.empty() || !Json::parseFromStream(rb, in, &root, &errs) || !root.isArray()) {
            if (!errs.empty()) LOGE("radio-browser json: %s", errs.c_str());
        } else {
            for (const auto& s : root) {
                RadioStation st;
                st.name = s.get("name", "").asString();
                st.url = s.get("url_resolved", "").asString();
                if (st.url.empty()) st.url = s.get("url", "").asString();
                st.tags = s.get("tags", "").asString();
                st.codec = s.get("codec", "").asString();
                st.country = s.get("country", "").asString();
                st.favicon = s.get("favicon", "").asString();
                st.bitrate = s.get("bitrate", 0).asInt();
                if (st.name.empty()) continue;
                // Playlist containers (.pls/.m3u/...) are not openable
                // streams for FFmpeg — drop them instead of failing on tap.
                static const char* kPlaylistSuffix[] = {
                        ".pls", ".m3u", ".m3u8", ".asx", ".qtl", ".ram"};
                bool playlist = false;
                for (const char* suf : kPlaylistSuffix) {
                    const size_t n = strlen(suf);
                    if (st.url.size() >= n
                            && strcasecmp(st.url.c_str() + st.url.size() - n, suf) == 0) {
                        playlist = true;
                        break;
                    }
                }
                if (playlist) continue;
                const bool http = st.url.compare(0, 7, "http://") == 0;
                const bool https = st.url.compare(0, 8, "https://") == 0;
                if (!http && !https) continue;
                // The bundled FFmpeg carries the https protocol name but no
                // TLS backend — plain-http streams lead so taps play, and
                // https-only stations are marked in the row.
                if (https && !http) st.name += " [https]";
                if (http) stations.insert(stations.begin(), std::move(st));
                else stations.push_back(std::move(st));
            }
        }
        // Deliver on the UI thread (static-local: first use is post-App).
        static cdroid::Handler sMain(cdroid::Looper::getMainLooper());
        sMain.post([stations = std::move(stations), onDone = std::move(onDone)]() mutable {
            onDone(std::move(stations));
        });
    }).detach();
}

void RadioBrowser::searchByTag(const std::string& tag,
        std::function<void(std::vector<RadioStation>)> onDone, int offset) {
    searchStations("tagList=" + tag, std::move(onDone), offset);
}

void RadioBrowser::searchByName(const std::string& term,
        std::function<void(std::vector<RadioStation>)> onDone, int offset) {
    searchStations("name=" + urlEncode(term), std::move(onDone), offset);
}

} // namespace remusic

#endif // REMUSIC_ONLINE
