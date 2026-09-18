// Port of provider/PlaylistInfo + PlaylistsManager — the user's own
// playlists, persisted via SharedPreferences ("playlists": one
// "name\tid,id,..." line each; Gson'd collections in the original).
#ifndef __REMUSIC_PLAYLISTSTORE_H__
#define __REMUSIC_PLAYLISTSTORE_H__

#include <string>
#include <vector>

#include <core/context.h>

namespace remusic {

struct Playlist {
    std::string name;
    std::vector<long> songIds;
};

class PlaylistStore {
public:
    static PlaylistStore& get();

    void init(cdroid::Context* context) { mContext = context; }

    std::vector<Playlist> getPlaylists() const;
    bool createPlaylist(const std::string& name);
    bool deletePlaylist(const std::string& name);
    bool renamePlaylist(const std::string& from, const std::string& to);
    void addSong(const std::string& name, long songId);
    void removeSong(const std::string& name, long songId);

private:
    PlaylistStore() = default;
    void save(const std::vector<Playlist>& lists);

    cdroid::Context* mContext = nullptr;
};

} // namespace remusic
#endif
