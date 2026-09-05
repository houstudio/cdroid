#include "archiveorg.h"

#ifdef REMUSIC_ONLINE

#include <algorithm>
#include <cstdlib>
#include <sstream>
#include <thread>

#include <json/json.h>

#include "httputil.h"
#include <core/handler.h>
#include <core/looper.h>
#include <porting/cdlog.h>

namespace remusic {

static void postToMain(std::function<void()> fn) {
    // static-local: first use is post-App, so the main looper exists.
    static cdroid::Handler sMain(cdroid::Looper::getMainLooper());
    sMain.post(std::move(fn));
}

static Json::Value parseJson(const std::string& body) {
    Json::Value root;
    Json::CharReaderBuilder rb;
    std::string errs;
    std::istringstream in(body);
    if (body.empty() || !Json::parseFromStream(rb, in, &root, &errs)) {
        if (!errs.empty()) LOGE("archive json: %s", errs.c_str());
        return Json::Value();
    }
    return root;
}

void ArchiveOrg::searchConcerts(const std::string& artist,
        std::function<void(std::vector<ArchiveConcert>)> onDone) {
    std::thread([artist, onDone = std::move(onDone)]() mutable {
        const std::string url =
                "https://archive.org/advancedsearch.php"
                "?q=collection%3Aetree+AND+creator%3A%22" + urlEncode(artist) +
                "%22&fl%5B%5D=identifier&fl%5B%5D=title&fl%5B%5D=year"
                "&fl%5B%5D=creator&rows=40&output=json";
        std::vector<ArchiveConcert> out;
        const Json::Value root = parseJson(httpGet(url));
        const Json::Value docs = root["response"]["docs"];
        if (docs.isArray()) {
            for (const auto& d : docs) {
                ArchiveConcert c;
                c.identifier = d.get("identifier", "").asString();
                c.title = d.get("title", "").asString();
                c.year = d.get("year", "").asString();
                c.creator = d.get("creator", "").asString();
                if (c.creator.empty()) c.creator = artist;
                if (!c.identifier.empty()) out.push_back(std::move(c));
            }
        }
        postToMain([out = std::move(out), onDone = std::move(onDone)]() mutable {
            onDone(std::move(out));
        });
    }).detach();
}

// "241.23" seconds or "4:01" mm:ss -> ms
static int lengthToMs(const std::string& s) {
    if (s.empty()) return 0;
    const size_t colon = s.find(':');
    if (colon != std::string::npos)
        return (int)((atof(s.substr(0, colon).c_str()) * 60
                    + atof(s.c_str() + colon + 1)) * 1000);
    return (int)(atof(s.c_str()) * 1000);
}

void ArchiveOrg::fetchTracks(const std::string& identifier,
        std::function<void(std::vector<ArchiveTrack>)> onDone) {
    std::thread([identifier, onDone = std::move(onDone)]() mutable {
        const std::string url = "https://archive.org/metadata/" + identifier;
        std::vector<ArchiveTrack> out;
        const Json::Value root = parseJson(httpGet(url));
        const Json::Value files = root["files"];
        if (files.isArray()) {
            // etree items ship several formats per track; VBR MP3 is the
            // full-quality derivative every item carries.
            std::vector<const Json::Value*> vbr, any;
            for (const auto& f : files) {
                const std::string fmt = f.get("format", "").asString();
                const std::string name = f.get("name", "").asString();
                if (name.size() < 5 || name.compare(name.size() - 4, 4, ".mp3") != 0)
                    continue;
                if (name.find("_sample") != std::string::npos) continue;
                if (fmt == "VBR MP3") vbr.push_back(&f);
                else if (fmt.find("MP3") != std::string::npos) any.push_back(&f);
            }
            const auto& pick = !vbr.empty() ? vbr : any;
            // Keep item order (already track-sorted); a stable numeric key
            // fixes the occasional scrambled listing.
            std::vector<std::pair<int, const Json::Value*>> keyed;
            for (const Json::Value* f : pick)
                keyed.push_back({atoi(f->get("track", "0").asString().c_str()), f});
            std::stable_sort(keyed.begin(), keyed.end(),
                    [](const auto& a, const auto& b) { return a.first < b.first; });
            for (auto& [_, f] : keyed) {
                ArchiveTrack t;
                t.title = f->get("title", "").asString();
                if (t.title.empty()) {
                    const std::string name = f->get("name", "").asString();
                    const size_t dot = name.rfind('.');
                    t.title = dot == std::string::npos ? name : name.substr(0, dot);
                }
                t.url = "https://archive.org/download/" + identifier
                        + "/" + urlEncode(f->get("name", "").asString());
                t.durationMs = lengthToMs(f->get("length", "").asString());
                if (!t.url.empty()) out.push_back(std::move(t));
            }
        }
        postToMain([out = std::move(out), onDone = std::move(onDone)]() mutable {
            onDone(std::move(out));
        });
    }).detach();
}

} // namespace remusic

#endif // REMUSIC_ONLINE
