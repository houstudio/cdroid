// Internet Archive (archive.org) live-music integration — the etree
// collection: trade-friendly concert recordings, key-free REST. Search
// concerts by artist, resolve an item's tracklist with direct file URLs.
// Callbacks are delivered on the main looper; failures deliver empty lists.
#ifndef __REMUSIC_ARCHIVEORG_H__
#define __REMUSIC_ARCHIVEORG_H__

#include <functional>
#include <string>
#include <vector>

namespace remusic {

struct ArchiveConcert {
    std::string identifier;   // item id, e.g. gd1977-05-08.sbd...
    std::string title;
    std::string year;
    std::string creator;      // artist
};

struct ArchiveTrack {
    std::string title;
    std::string url;          // direct download URL
    int durationMs = 0;
};

class ArchiveOrg {
public:
    /** Browse the netlabels collection (free albums) by popularity — the
     *  点播 catalog that needs no search term. */
    static void browseNetlabels(std::function<void(std::vector<ArchiveConcert>)> onDone);
    static void searchConcerts(const std::string& artist,
            std::function<void(std::vector<ArchiveConcert>)> onDone);
    static void fetchTracks(const std::string& identifier,
            std::function<void(std::vector<ArchiveTrack>)> onDone);
};

} // namespace remusic
#endif
