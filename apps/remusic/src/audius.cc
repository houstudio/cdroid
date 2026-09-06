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

// Discovery hosts, all serving the same API. Different mainland routes
// reach different ones, so every request walks the list (cached winner
// first) until a host answers — a single blocked host must not blank the
// on-demand tab.
static const char* const kHosts[] = {
        "https://api.audius.co",
        "https://discoveryprovider.audius.co",
        "https://discoveryprovider2.audius.co",
        "https://discoveryprovider3.audius.co",
};
static std::string sWorkingHost;   // first host that answered (empty = none yet)
static const char* kApp = "remusic-cdroid";


// Host for a track's redirect chain: prefer the host this network already
// reached (empty = none yet, use the primary).
std::string Audius::streamUrl(const std::string& trackId) {
    const std::string host = sWorkingHost.empty() ? kHosts[0] : sWorkingHost;
    return host + "/v1/tracks/" + trackId + "/stream?app_name=" + kApp;
}

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
    if (body.empty() || !Json::parseFromStream(rb, in, &root, &errs)) {
        if (!errs.empty()) LOGE("audius json: %s", errs.c_str());
        return out;
    }
    // The API wraps arrays in {"data": [...]} (gateway and discovery hosts
    // alike); bare arrays were the old shape — accept both.
    const Json::Value arr = root.isArray() ? root : root.get("data", Json::Value());
    if (!arr.isArray()) return out;
    for (const auto& t : arr) {
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
        track.url = Audius::streamUrl(track.id);
        if (!track.id.empty() && !track.title.empty()) out.push_back(std::move(track));
    }
    return out;
}

// `path` carries "/v1/...&app_name="; hosts are walked outside.
static void fetch(const std::string& path, Audius::TracksCb onDone) {
    std::thread([path, onDone = std::move(onDone)]() mutable {
        std::vector<AudiusTrack> tracks;
        std::string error = "no host attempted";
        // Cached winner first, then the rest — first non-empty body wins.
        for (int round = 0; round < 2 && tracks.empty(); round++) {
            for (const char* host : kHosts) {
                const std::string h = host;
                if (round == 0 && !sWorkingHost.empty() && h != sWorkingHost) continue;
                if (round == 1 && h == sWorkingHost) continue;
                const std::string body = httpGet(h + path);
                error = lastHttpError();
                if (body.empty()) continue;
                tracks = parseTracks(body);
                if (!tracks.empty()) {
                    sWorkingHost = h;
                    error.clear();
                    break;
                }
                error = "empty chart from " + h;
            }
        }
        postToMain([tracks = std::move(tracks), error, onDone = std::move(onDone)]() mutable {
            onDone(std::move(tracks), error);
        });
    }).detach();
}

// Page size for the trending/search pagination (the panel's infinite list).
static const int kAudiusPage = 50;

void Audius::trending(TracksCb onDone, int offset) {
    fetch("/v1/tracks/trending?app_name=" + std::string(kApp)
            + "&limit=" + std::to_string(kAudiusPage)
            + "&offset=" + std::to_string(offset), std::move(onDone));
}

void Audius::trendingGenre(const std::string& genre, TracksCb onDone, int offset) {
    fetch("/v1/tracks/trending?genre=" + urlEncode(genre)
            + "&app_name=" + std::string(kApp)
            + "&limit=" + std::to_string(kAudiusPage)
            + "&offset=" + std::to_string(offset), std::move(onDone));
}

void Audius::search(const std::string& query, TracksCb onDone, int offset) {
    fetch("/v1/tracks/search?query=" + urlEncode(query)
            + "&app_name=" + std::string(kApp)
            + "&limit=" + std::to_string(kAudiusPage)
            + "&offset=" + std::to_string(offset), std::move(onDone));
}


} // namespace remusic

#endif // REMUSIC_ONLINE
