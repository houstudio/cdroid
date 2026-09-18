#include "ccmixter.h"

#ifdef REMUSIC_ONLINE

#include <cstdlib>
#include <sstream>
#include <thread>

#include <json/json.h>

#include "httputil.h"
#include <core/handler.h>
#include <core/looper.h>
#include <porting/cdlog.h>

namespace remusic {

static const char* kBase = "https://ccmixter.org/api/query?f=json&req=search&limit=25";

static void postToMain(std::function<void()> fn) {
    static cdroid::Handler sMain(cdroid::Looper::getMainLooper());
    sMain.post(std::move(fn));
}

// "3:22" (file_format_info.ps) -> ms; 0 when absent.
static int playLengthMs(const Json::Value& file) {
    const std::string ps = file.get("file_format_info", Json::Value())
                               .get("ps", "").asString();
    const size_t c = ps.find(':');
    if (c == std::string::npos) return 0;
    return (int) ((atoi(ps.substr(0, c).c_str()) * 60
            + atoi(ps.c_str() + c + 1)) * 1000);
}

// Pick the mp3 file (uploads often carry flac+mp3; mp3 is what FFmpeg wants).
static const Json::Value* pickMp3(const Json::Value& files) {
    const Json::Value* mp3 = nullptr;
    for (const auto& f : files) {
        if (f.get("file_nicname", "").asString() == "mp3") return &f;
        if (mp3 == nullptr) mp3 = &f;
    }
    return mp3;
}

static std::vector<AudiusTrack> parseUploads(const std::string& body) {
    std::vector<AudiusTrack> out;
    Json::Value root;
    Json::CharReaderBuilder rb;
    std::string errs;
    std::istringstream in(body);
    if (body.empty() || !Json::parseFromStream(rb, in, &root, &errs) || !root.isArray()) {
        if (!errs.empty()) LOGE("ccmixter json: %s", errs.c_str());
        return out;
    }
    for (const auto& u : root) {
        if (!u.isObject()) continue;
        // u.get(key, Json::Value()) returns a reference that can dangle
        // into a temporary (the crash this loop once had); operator[]
        // instead yields a reference to a shared null for missing keys.
        const Json::Value& files = u["files"];
        const Json::Value* file = files.isArray() && files.size() > 0
                ? pickMp3(files) : nullptr;
        if (file == nullptr) continue;
        AudiusTrack t;
        t.id = std::to_string(u.get("upload_id", 0).asInt64());
        t.title = u.get("upload_name", "").asString();
        t.artist = u.get("user_name", "").asString();
        t.url = file->get("download_url", "").asString();
        t.durationMs = playLengthMs(*file);
        if (!t.url.empty() && !t.title.empty()) out.push_back(std::move(t));
    }
    return out;
}

static void fetch(const std::string& url, CcMixter::Cb onDone) {
    std::thread([url, onDone = std::move(onDone)]() mutable {
        const std::string body = httpGet(url);
        std::vector<AudiusTrack> tracks = body.empty()
                ? std::vector<AudiusTrack>() : parseUploads(body);
        const std::string error = tracks.empty()
                ? (body.empty() ? lastHttpError() : "empty chart") : std::string();
        postToMain([tracks = std::move(tracks), error, onDone = std::move(onDone)]() mutable {
            onDone(std::move(tracks), error);
        });
    }).detach();
}

void CcMixter::chart(const std::string& tag, Cb onDone, int offset) {
    std::string url = std::string(kBase) + "&sort=rank"
            + "&offset=" + std::to_string(offset);
    if (!tag.empty()) url += "&search_text=" + urlEncode(tag);
    fetch(url, std::move(onDone));
}

void CcMixter::search(const std::string& text, Cb onDone, int offset) {
    fetch(std::string(kBase) + "&search_text=" + urlEncode(text)
            + "&offset=" + std::to_string(offset), std::move(onDone));
}

} // namespace remusic

#endif // REMUSIC_ONLINE
