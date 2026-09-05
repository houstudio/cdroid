// Audius (audius.org) discovery client — key-free, the closest live
// equivalent to the original's Baidu-ting on-demand backend: trending
// tracks plus by-name search, streaming via /v1/tracks/{id}/stream (a
// redirect chain to an audio/mpeg content node; needs a TLS-capable
// FFmpeg — the app's test deployment ships one).
#ifndef __REMUSIC_AUDIUS_H__
#define __REMUSIC_AUDIUS_H__

#include <functional>
#include <string>
#include <vector>

namespace remusic {

struct AudiusTrack {
    std::string id;
    std::string title;
    std::string artist;
    std::string artwork;   // direct https URL ("" when absent)
    int durationMs = 0;
};

class Audius {
public:
    static void trending(std::function<void(std::vector<AudiusTrack>)> onDone);
    /** Trending within one genre ("Electronic", "Hip-Hop/Rap", ...). */
    static void trendingGenre(const std::string& genre,
            std::function<void(std::vector<AudiusTrack>)> onDone);
    static void search(const std::string& query,
            std::function<void(std::vector<AudiusTrack>)> onDone);
    /** Playable URL for a track id (FFmpeg follows the redirect chain). */
    static std::string streamUrl(const std::string& trackId);
};

} // namespace remusic
#endif
