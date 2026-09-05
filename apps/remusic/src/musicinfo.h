// Port of com.wm.remusic.info.MusicInfo + com.wm.remusic.service.MusicTrack.
// Plain value types; the Parcelable bundle-key surface is kept as constants
// where the UI passes MusicInfo through intents/bundles.
#ifndef __REMUSIC_MUSICINFO_H__
#define __REMUSIC_MUSICINFO_H__
#include <string>

namespace remusic {

class MusicInfo {
public:
    static constexpr const char* KEY_SONG_ID    = "songid";
    static constexpr const char* KEY_ALBUM_ID   = "albumid";
    static constexpr const char* KEY_ALBUM_NAME = "albumname";
    static constexpr const char* KEY_ALBUM_DATA = "albumdata";
    static constexpr const char* KEY_DURATION   = "duration";
    static constexpr const char* KEY_MUSIC_NAME = "musicname";
    static constexpr const char* KEY_ARTIST     = "artist";
    static constexpr const char* KEY_ARTIST_ID  = "artist_id";
    static constexpr const char* KEY_DATA       = "data";
    static constexpr const char* KEY_FOLDER     = "folder";
    static constexpr const char* KEY_SIZE       = "size";
    static constexpr const char* KEY_FAVORITE   = "favorite";
    static constexpr const char* KEY_LRC        = "lrc";
    static constexpr const char* KEY_ISLOCAL    = "islocal";
    static constexpr const char* KEY_SORT       = "sort";

    long songId = -1;
    int  albumId = -1;
    std::string albumName;
    std::string albumData;   // album art path
    int  duration = 0;       // ms
    std::string musicName;
    std::string artist;
    long artistId = -1;
    std::string data;        // absolute file path
    std::string folder;
    std::string lrc;
    bool islocal = true;     // online path is trimmed in this port
    std::string sort;        // pinyin index key (A-Z / #)
    int  size = 0;           // bytes
    bool favorite = false;
};

// Timber's queue row: identity used to address the play queue.
class MusicTrack {
public:
    long id = -1;        // music id
    long audioId = -1;   // audio (MediaStore) id
};

} // namespace remusic
#endif
