// LRCLIB (lrclib.net) lyrics client — free, key-less, synced LRC. Serves
// tracks with no sibling .lrc: the on-demand (Audius/ccMixter) songs and
// lrc-less local files alike. The text is standard LRC, fed straight into
// the existing parseLrc()/LrcView path.
#ifndef __REMUSIC_LRCLIB_H__
#define __REMUSIC_LRCLIB_H__

#include <string>

namespace remusic {

class LrcLib {
public:
    /** Synced LRC text for the track, "" when nothing matched. Blocking
     *  (network) — call from a worker thread. Hits and misses are memoized
     *  so flipping between tracks does not refetch. */
    static std::string fetchSyncedLyrics(const std::string& title,
            const std::string& artist, long durationMs);
};

} // namespace remusic
#endif
