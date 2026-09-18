#include "lrclib.h"

#ifdef REMUSIC_ONLINE

#include <cmath>
#include <map>
#include <mutex>
#include <sstream>

#include <json/json.h>

#include "httputil.h"
#include <porting/cdlog.h>

namespace remusic {

static const char* const kHost = "https://lrclib.net";

// Memoized (title|artist|duration) -> lrc. Misses are cached too (a song
// without lyrics stays without). Bounded crudely: lyrics are a cache, not
// a store.
static std::mutex sCacheMutex;
static std::map<std::string, std::string> sCache;

static bool parseJson(const std::string& body, Json::Value* out) {
    Json::CharReaderBuilder rb;
    std::istringstream in(body);
    std::string errs;
    return !body.empty() && Json::parseFromStream(rb, in, out, &errs);
}

std::string LrcLib::fetchSyncedLyrics(const std::string& title,
        const std::string& artist, long durationMs) {
    if (title.empty()) return "";
    const std::string key = title + "|" + artist + "|" + std::to_string(durationMs);
    {
        std::lock_guard<std::mutex> lock(sCacheMutex);
        auto it = sCache.find(key);
        if (it != sCache.end()) return it->second;
    }

    std::string lrc;
    // 1) Exact query — one object with syncedLyrics, 404 (empty body) on miss.
    Json::Value root;
    if (parseJson(httpGet(std::string(kHost) + "/api/get?track_name=" + urlEncode(title)
                    + "&artist_name=" + urlEncode(artist)), &root)
            && root.isObject()) {
        lrc = root.get("syncedLyrics", "").asString();
    }

    // 2) Search fallback — candidate list; keep the closest-duration entry
    //    that actually carries synced lyrics. A duration far off ours is a
    //    different song, not a fuzzy match.
    if (lrc.empty()) {
        Json::Value arr;
        if (parseJson(httpGet(std::string(kHost) + "/api/search?track_name=" + urlEncode(title)
                        + "&artist_name=" + urlEncode(artist)), &arr)
                && arr.isArray()) {
            double best = -1.0;
            std::string bestLrc;
            for (const auto& c : arr) {
                if (!c.isObject()) continue;
                const std::string sync = c.get("syncedLyrics", "").asString();
                if (sync.empty()) continue;
                const double diff = std::fabs(c.get("duration", 0).asDouble() * 1000.0
                        - (double)durationMs);
                if (durationMs > 0 && diff > 15000.0) continue;
                if (best < 0 || diff < best) { best = diff; bestLrc = sync; }
            }
            lrc = bestLrc;
        }
    }

    LOGD("lrclib: %s - %s -> %s", title.c_str(), artist.c_str(),
         lrc.empty() ? "no match" : "synced lrc");
    std::lock_guard<std::mutex> lock(sCacheMutex);
    if (sCache.size() > 96) sCache.clear();
    sCache[key] = lrc;
    return lrc;
}

} // namespace remusic
#endif /* REMUSIC_ONLINE */
