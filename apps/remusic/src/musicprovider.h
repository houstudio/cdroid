// MediaStore replacement: com.wm.remusic.uitl.MusicUtils' query surface
// (queryMusic/queryArtist/queryAlbums/queryFolder) backed by a filesystem
// scan instead of ContentResolver. CDROID has no MediaStore (apps/music
// precedent), so the scan walks the music directories once per process and
// caches; sort orders follow uitl/SortOrder.java's keys.
#ifndef __REMUSIC_MUSICPROVIDER_H__
#define __REMUSIC_MUSICPROVIDER_H__

#include <map>
#include <string>
#include <vector>

#include "musicinfo.h"

namespace remusic {

class MusicProvider {
public:
    static MusicProvider& get();

    /** Scan the standard roots (./music, $HOME/Music) once; rescans cheap. */
    void scanIfNeeded();
    void addRoot(const std::string& dir);

    // ---- MusicUtils query shapes ----
    std::vector<MusicInfo> queryMusic(const std::string& sortOrder);
    std::map<long, std::string> queryArtist(const std::string& sortOrder);
    std::map<long, std::string> queryAlbums(const std::string& sortOrder);
    std::map<std::string, int> queryFolder();

    const MusicInfo* find(long songId) const;

    // Metadata learned by the decoder (ID3 etc.), cached across runs.
    void updateTags(const std::string& path, const std::string& artist,
                    const std::string& title, const std::string& album, long durationMs);

    // Detail pages (Artist/Album/Folder fragments).
    std::vector<MusicInfo> songsByArtist(long artistId);
    std::vector<MusicInfo> songsByAlbum(long albumId);
    std::vector<MusicInfo> songsByFolder(const std::string& folder);

    static constexpr const char* SORT_ORDER_A_Z = "sort_a_z";
    static constexpr const char* SORT_ORDER_Z_A = "sort_z_a";
    static constexpr const char* SORT_ORDER_DURATION = "duration";

private:
    MusicProvider() = default;
    void scan();
    void scanDir(const std::string& dir);

    std::vector<MusicInfo> mSongs;
    std::map<long, MusicInfo> mById;
    std::vector<std::string> mRoots;
    bool mScanned = false;
};

} // namespace remusic
#endif
