#include "audius.h"

#ifdef REMUSIC_ONLINE

#include <sstream>
#include <thread>

#include <json/json.h>

#include "httputil.h"
#include <core/handler.h>
#include <core/looper.h>
#include <porting/cdlog.h>

namespace remusic {

// Discovery host selection: api.audius.co returns the candidate list; the
// first entry is the standard endpoint (kept simple — the list rarely
// changes and any host serves the same API).
static const char* kHost = "https://api.audius.co";
static const char* kApp = "remusic-cdroid";

static void postToMain(std::function<void()> fn) {
    static cdroid::Handler sMain(cdroid::Looper::getMainLooper());
    sMain.post(std::move(fn));
}

static std::vector<AudiusTrack> parseTracks(const std::string& body) {
    std::vector<AudiusTrack> out;
    Json::Value root;
    Json::CharReaderBuilder rb;
    std::string errs;
    std::istringstream in(body);
    if (body.empty() || !Json::parseFromStream(rb, in, &root, &errs) || !root.isArray()) {
        if (!errs.empty()) LOGE("audius json: %s", errs.c_str());
        return out;
    }
    for (const auto& t : root) {
        if (!t.isObject()) continue;
        AudiusTrack track;
        track.id = t.get("id", "").asString();
        track.title = t.get("title", "").asString();
        track.artist = t.get("user", Json::Value()).get("name", "").asString();
        // durations arrive in SECONDS (int or "3:25" mm:ss on older nodes)
        const Json::Value d = t.get("duration", 0);
        if (d.isNumeric()) track.durationMs = d.asInt() * 1000;
        else if (d.isString() && d.asString().find(':') != std::string::npos) {
            const std::string s = d.asString();
            const size_t c = s.find(':');
            track.durationMs = (int) ((atoi(s.substr(0, c).c_str()) * 60
                    + atoi(s.c_str() + c + 1)) * 1000);
        }
        const Json::Value art = t.get("artwork", Json::Value());
        for (const char* size : {"480", "1000", "150"}) {
            track.artwork = art.get(size, "").asString();
            if (!track.artwork.empty()) break;
        }
        if (!track.id.empty() && !track.title.empty()) out.push_back(std::move(track));
    }
    return out;
}

static void fetch(const std::string& url,
        std::function<void(std::vector<AudiusTrack>)> onDone) {
    std::thread([url, onDone = std::move(onDone)]() mutable {
        // The redirect chain lands on an audio/mpeg node; FFmpeg follows it.
        std::vector<AudiusTrack> tracks = parseTracks(httpGet(url));
        postToMain([tracks = std::move(tracks), onDone = std::move(onDone)]() mutable {
            onDone(std::move(tracks));
        });
    }).detach();
}

void Audius::trending(std::function<void(std::vector<AudiusTrack>)> onDone) {
    fetch(std::string(kHost) + "/v1/tracks/trending?app_name=" + kApp, std::move(onDone));
}

void Audius::trendingGenre(const std::string& genre,
        std::function<void(std::vector<AudiusTrack>)> onDone) {
    fetch(std::string(kHost) + "/v1/tracks/trending?genre=" + urlEncode(genre)
            + "&app_name=" + kApp, std::move(onDone));
}

void Audius::search(const std::string& query,
        std::function<void(std::vector<AudiusTrack>)> onDone) {
    fetch(std::string(kHost) + "/v1/tracks/search?query=" + urlEncode(query)
            + "&app_name=" + kApp, std::move(onDone));
}

std::string Audius::streamUrl(const std::string& trackId) {
    return std::string(kHost) + "/v1/tracks/" + trackId + "/stream?app_name=" + kApp;
}

} // namespace remusic

#endif // REMUSIC_ONLINE
