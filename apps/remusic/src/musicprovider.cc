#include "musicprovider.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include <core/app.h>
#include <core/context.h>
#include <content/sharedpreferences.h>

namespace remusic {

// prefs "tags": one "path\tartist\ttitle\talbum\tdurationMs" line per file.
static const char* PREFS_TAGS = "musictags";

static std::string tagField(const std::string& line, size_t idx) {
    size_t start = 0;
    for (size_t i = 0; i <= idx; i++) {
        const size_t tab = line.find('\t', start);
        const size_t end = (i == idx)
                ? (tab == std::string::npos ? line.size() : tab) : tab;
        if (i == idx) return line.substr(start, end - start);
        if (tab == std::string::npos) return std::string();
        start = tab + 1;
    }
    return std::string();
}

static void loadTagsCache(std::map<std::string, std::string>& tags) {
    cdroid::Context* ctx = &cdroid::App::getInstance();
    if (ctx == nullptr) return;
    const std::string blob = ctx->getSharedPreferences(PREFS_TAGS, 0)->getString("cache", "");
    size_t start = 0;
    while (start < blob.size()) {
        const size_t nl = blob.find('\n', start);
        const std::string line = blob.substr(start,
                nl == std::string::npos ? std::string::npos : nl - start);
        const std::string path = tagField(line, 0);
        if (!path.empty()) tags[path] = line;
        if (nl == std::string::npos) break;
        start = nl + 1;
    }
}

MusicProvider& MusicProvider::get() {
    static MusicProvider instance;
    return instance;
}

void MusicProvider::addRoot(const std::string& dir) {
    mRoots.push_back(dir);
    mScanned = false;
}

void MusicProvider::scanIfNeeded() {
    if (mScanned) return;
    if (mRoots.empty()) {
        mRoots.push_back("./music");
        const char* home = getenv("HOME");
        if (home) mRoots.push_back(std::string(home) + "/Music");
    }
    scan();
    mScanned = true;
}

static bool hasSuffix(const std::string& s, const char* suf) {
    const size_t n = strlen(suf);
    return s.size() >= n && strcasecmp(s.c_str() + s.size() - n, suf) == 0;
}

// Stable song id from the file path (the queue addresses tracks by id and is
// persisted across runs, so the id must survive restarts).
static long pathId(const std::string& path) {
    long h = 1125899906842597L;   // FNV-1a-ish
    for (unsigned char c : path) { h = 31 * h + c; }
    return h & 0x7fffffff;
}

// "Artist - Title.ext" filename convention; falls back to the bare stem.
static void parseStem(const std::string& stem, std::string& artist, std::string& title) {
    const size_t dash = stem.find(" - ");
    if (dash != std::string::npos && dash > 0 && dash + 3 < stem.size()) {
        artist = stem.substr(0, dash);
        title = stem.substr(dash + 3);
    } else {
        artist = "<未知艺术家>";
        title = stem;
    }
}

// A-Z sort key like ConverPinYin: first ASCII letter uppercased, else '#'.
static std::string sortKeyOf(const std::string& name) {
    for (unsigned char c : name) {
        if (isascii(c) && isalpha(c)) return std::string(1, (char)toupper(c));
        if (isascii(c) && isdigit(c)) break;
    }
    return "#";
}

static off_t fileSize(const std::string& path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0 ? st.st_size : 0;
}

// First cover-ish image in the folder (album art convention).
static std::string findCover(const std::string& dir) {
    static const char* names[] = {"cover.jpg", "cover.png", "folder.jpg", "folder.png",
                                  "Cover.jpg", "Folder.jpg", nullptr};
    for (int i = 0; names[i]; i++) {
        const std::string p = dir + "/" + names[i];
        if (access(p.c_str(), R_OK) == 0) return p;
    }
    return std::string();
}

void MusicProvider::scanDir(const std::string& dir) {
    DIR* d = opendir(dir.c_str());
    if (d == nullptr) return;
    const std::string cover = findCover(dir);
    struct dirent* e;
    while ((e = readdir(d)) != nullptr) {
        const std::string name = e->d_name;
        if (name == "." || name == "..") continue;
        const std::string full = dir + "/" + name;
        struct stat st;
        if (stat(full.c_str(), &st) != 0) continue;
        if (S_ISDIR(st.st_mode)) { scanDir(full); continue; }
        if (!(hasSuffix(name, ".mp3") || hasSuffix(name, ".flac") || hasSuffix(name, ".wav")
                || hasSuffix(name, ".ogg") || hasSuffix(name, ".aac"))) continue;
        if (st.st_size < 1024 * 1024) continue;   // MusicUtils' >1MB filter

        MusicInfo info;
        std::string stem = name;
        const size_t dot = stem.rfind('.');
        if (dot != std::string::npos) stem = stem.substr(0, dot);
        std::string artist, title;
        parseStem(stem, artist, title);
        // Decoder-learned tags (ID3) win over the filename convention.
        std::string tagArtist, tagTitle, tagAlbum;
        long tagDur = 0;
        static std::map<std::string, std::string> sTagCache = [] {
            std::map<std::string, std::string> m;
            loadTagsCache(m);
            return m;
        }();
        auto tag = sTagCache.find(full);
        if (tag != sTagCache.end()) {
            tagArtist = tagField(tag->second, 1);
            tagTitle = tagField(tag->second, 2);
            tagAlbum = tagField(tag->second, 3);
            tagDur = atol(tagField(tag->second, 4).c_str());
        }
        info.songId = pathId(full);
        info.musicName = title;
        info.artist = artist;
        info.albumName = "<未知专辑>";   // real tag parsing lands with the decode backend
        info.albumId = 0;
        info.artistId = pathId(artist);
        info.data = full;
        info.folder = dir;
        info.albumData = cover;   // DraweeView stand-in art source
        info.size = (int)st.st_size;
        info.islocal = true;
        // Decoder-learned tags (ID3) win over the filename convention.
        if (!tagArtist.empty()) { info.artist = tagArtist; info.artistId = pathId(tagArtist); }
        if (!tagTitle.empty()) { info.musicName = tagTitle; info.sort = sortKeyOf(tagTitle); }
        if (!tagAlbum.empty()) info.albumName = tagAlbum;
        info.duration = tagDur > 0 ? (int)tagDur : 210000;
        mSongs.push_back(info);
    }
    closedir(d);
}

void MusicProvider::scan() {
    mSongs.clear();
    for (auto& root : mRoots) scanDir(root);
    std::sort(mSongs.begin(), mSongs.end(), [](const MusicInfo& a, const MusicInfo& b) {
        return a.musicName < b.musicName;
    });
    mById.clear();
    for (auto& s : mSongs) mById[s.songId] = s;
}

std::vector<MusicInfo> MusicProvider::queryMusic(const std::string& sortOrder) {
    scanIfNeeded();
    std::vector<MusicInfo> out = mSongs;
    if (sortOrder == SORT_ORDER_Z_A) {
        std::sort(out.begin(), out.end(), [](const MusicInfo& a, const MusicInfo& b) {
            return a.musicName > b.musicName; });
    } else if (sortOrder == SORT_ORDER_DURATION) {
        std::sort(out.begin(), out.end(), [](const MusicInfo& a, const MusicInfo& b) {
            return a.duration > b.duration; });
    } else {
        std::sort(out.begin(), out.end(), [](const MusicInfo& a, const MusicInfo& b) {
            if (a.sort != b.sort) return a.sort < b.sort;
            return a.musicName < b.musicName; });
    }
    return out;
}

std::map<long, std::string> MusicProvider::queryArtist(const std::string&) {
    scanIfNeeded();
    std::map<long, std::string> out;
    for (auto& s : mSongs) out[s.artistId] = s.artist;
    return out;
}

std::map<long, std::string> MusicProvider::queryAlbums(const std::string&) {
    scanIfNeeded();
    std::map<long, std::string> out;
    for (auto& s : mSongs) out[s.albumId] = s.albumName;
    return out;
}

std::map<std::string, int> MusicProvider::queryFolder() {
    scanIfNeeded();
    std::map<std::string, int> out;
    for (auto& s : mSongs) out[s.folder]++;
    return out;
}

void MusicProvider::updateTags(const std::string& path, const std::string& artist,
                               const std::string& title, const std::string& album,
                               long durationMs) {
    if (path.empty()) return;
    auto it = mById.end();
    for (auto& s : mSongs) {
        if (s.data == path) { it = mById.find(s.songId); break; }
    }
    if (durationMs <= 0 && artist.empty() && title.empty() && album.empty()) return;

    cdroid::Context* ctx = &cdroid::App::getInstance();
    if (ctx == nullptr) return;
    auto prefs = ctx->getSharedPreferences(PREFS_TAGS, 0);
    const std::string blob = prefs->getString("cache", "");
    std::string out;
    size_t start = 0;
    bool replaced = false;
    while (start < blob.size()) {
        const size_t nl = blob.find('\n', start);
        const std::string line = blob.substr(start,
                nl == std::string::npos ? std::string::npos : nl - start);
        if (tagField(line, 0) != path) out += line + "\n";
        else replaced = true;
        if (nl == std::string::npos) break;
        start = nl + 1;
    }
    char dur[24];
    snprintf(dur, sizeof(dur), "%ld", durationMs);
    out += path + "\t" + artist + "\t" + title + "\t" + album + "\t" + dur + "\n";
    if (!replaced && out.size() > 128 * 1024) out = out.substr(out.find('\n') + 1);
    prefs->edit().putString("cache", out).apply();
}

std::vector<MusicInfo> MusicProvider::songsByArtist(long artistId) {
    scanIfNeeded();
    std::vector<MusicInfo> out;
    for (auto& s : mSongs)
        if (s.artistId == artistId) out.push_back(s);
    return out;
}

std::vector<MusicInfo> MusicProvider::songsByAlbum(long albumId) {
    scanIfNeeded();
    std::vector<MusicInfo> out;
    for (auto& s : mSongs)
        if (s.albumId == albumId) out.push_back(s);
    return out;
}

std::vector<MusicInfo> MusicProvider::songsByFolder(const std::string& folder) {
    scanIfNeeded();
    std::vector<MusicInfo> out;
    for (auto& s : mSongs)
        if (s.folder == folder) out.push_back(s);
    return out;
}

const MusicInfo* MusicProvider::find(long songId) const {
    auto it = mById.find(songId);
    return it == mById.end() ? nullptr : &it->second;
}

} // namespace remusic
