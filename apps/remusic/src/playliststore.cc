#include "playliststore.h"

#include <cstdlib>

#include <content/sharedpreferences.h>

using namespace cdroid;

namespace remusic {

static const char* PREFS_PLAYLISTS = "playlists";

PlaylistStore& PlaylistStore::get() {
    static PlaylistStore instance;
    return instance;
}

std::vector<Playlist> PlaylistStore::getPlaylists() const {
    std::vector<Playlist> out;
    if (mContext == nullptr) return out;
    const std::string blob = mContext->getSharedPreferences(PREFS_PLAYLISTS, 0)
                                     ->getString("lists", "");
    size_t start = 0;
    while (start < blob.size()) {
        const size_t nl = blob.find('\n', start);
        const std::string line = blob.substr(start,
                nl == std::string::npos ? std::string::npos : nl - start);
        const size_t tab = line.find('\t');
        if (tab != std::string::npos) {
            Playlist pl;
            pl.name = line.substr(0, tab);
            size_t cs = tab + 1;
            while (cs <= line.size()) {
                const size_t comma = line.find(',', cs);
                const std::string tok = line.substr(cs,
                        comma == std::string::npos ? std::string::npos : comma - cs);
                if (!tok.empty()) pl.songIds.push_back(atol(tok.c_str()));
                if (comma == std::string::npos) break;
                cs = comma + 1;
            }
            out.push_back(pl);
        }
        if (nl == std::string::npos) break;
        start = nl + 1;
    }
    return out;
}

void PlaylistStore::save(const std::vector<Playlist>& lists) {
    if (mContext == nullptr) return;
    std::string blob;
    for (const auto& pl : lists) {
        blob += pl.name + "\t";
        for (size_t i = 0; i < pl.songIds.size(); i++) {
            char buf[24];
            snprintf(buf, sizeof(buf), "%s%ld", i ? "," : "", pl.songIds[i]);
            blob += buf;
        }
        blob += "\n";
    }
    mContext->getSharedPreferences(PREFS_PLAYLISTS, 0)
            ->edit().putString("lists", blob).apply();
}

bool PlaylistStore::createPlaylist(const std::string& name) {
    if (name.empty()) return false;
    auto lists = getPlaylists();
    for (auto& pl : lists)
        if (pl.name == name) return false;
    lists.push_back(Playlist{name, {}});
    save(lists);
    return true;
}

bool PlaylistStore::renamePlaylist(const std::string& from, const std::string& to) {
    auto lists = getPlaylists();
    for (auto& pl : lists) {
        if (pl.name == from) {
            pl.name = to;
            save(lists);
            return true;
        }
    }
    return false;
}

bool PlaylistStore::deletePlaylist(const std::string& name) {
    auto lists = getPlaylists();
    for (size_t i = 0; i < lists.size(); i++) {
        if (lists[i].name == name) {
            lists.erase(lists.begin() + i);
            save(lists);
            return true;
        }
    }
    return false;
}

void PlaylistStore::addSong(const std::string& name, long songId) {
    auto lists = getPlaylists();
    for (auto& pl : lists) {
        if (pl.name != name) continue;
        for (long id : pl.songIds)
            if (id == songId) return;   // dedup like PlaylistsManager
        pl.songIds.push_back(songId);
        save(lists);
        return;
    }
    // cdroid's apply() lands asynchronously: a createPlaylist() immediately
    // before this may not be visible to getPlaylists() yet. Creating the
    // (same-named) list here converges — createPlaylist refuses duplicates
    // on the next read, and the song lands either way.
    lists.push_back(Playlist{name, {songId}});
    save(lists);
}

void PlaylistStore::removeSong(const std::string& name, long songId) {
    auto lists = getPlaylists();
    for (auto& pl : lists) {
        if (pl.name != name) continue;
        for (size_t i = 0; i < pl.songIds.size(); i++) {
            if (pl.songIds[i] == songId) {
                pl.songIds.erase(pl.songIds.begin() + i);
                save(lists);
                return;
            }
        }
    }
}

} // namespace remusic
