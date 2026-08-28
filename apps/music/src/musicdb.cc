#include <musicdb.h>

#include <algorithm>

namespace cdroid {
namespace music {

namespace {

// Seed library: invented names (no real-world catalog), sized like a small
// phone library — enough rows that every list scrolls.
struct SeedTrack {
    const char* title;
    int trackNo;
    int secs;
};

struct SeedAlbum {
    const char* name;
    int trackCount;
};

struct SeedArtist {
    const char* name;
    SeedAlbum albums[3];
};

// durationMs comes from secs; ids are assigned sequentially on load.
const SeedArtist kSeed[] = {
    {"Aurora Skies", {
        {"Northern Lights", 6},
        {"Drift Season", 5},
    }},
    {"Neon District", {
        {"Midnight Transit", 7},
        {"Signal Fade", 4},
    }},
    {"The Paper Kites Cover Band", {
        {"Field Recordings", 5},
    }},
    {"Copper Lanes", {
        {"Terra Cotta", 6},
        {"Side Streets", 4},
    }},
    {"Marisol Vega", {
        {"Salt And Rain", 8},
    }},
    {"Blue Hour Collective", {
        {"Twelve Frames", 5},
        {"Bloom Static", 6},
    }},
    {"Kestrel", {
        {"Overpass", 4},
    }},
    {"Junction East", {
        {"Terminal 3", 5},
        {"Cargo Bay", 5},
    }},
    {"Hazel And The Fen", {
        {"Low Country", 6},
    }},
    {"Silent Meridian", {
        {"Depth Of Field", 7},
        {"Long Exposure", 5},
    }},
};

const SeedTrack kTitles[] = {
    {"First Light", 1, 213}, {"Over The Ridge", 2, 187}, {"Paper Boats", 3, 241},
    {"Copper Wire", 4, 176}, {"Slow Tide", 5, 258}, {"Nightjar", 6, 204},
    {"Undertow", 7, 232}, {"Halfway Home", 8, 199}, {"Static Bloom", 1, 221},
    {"Motel Neon", 2, 165}, {"Vanishing Point", 3, 247}, {"Cold Open", 4, 193},
    {"Glass Coast", 5, 226}, {"Old Growth", 1, 239}, {"Moss & Stone", 2, 184},
    {"Trailhead", 3, 210}, {"Wading In", 4, 263}, {"Kestrel's Cry", 1, 178},
    {"Sodium Glow", 2, 205}, {"Last Bus", 3, 231}, {"Firefly Field", 4, 168},
    {"Gravel Road", 5, 245}, {"Cargo Cult", 1, 217}, {"Salt Flats", 2, 254},
    {"Harbor Lights", 3, 189}, {"Monsoon Season", 4, 236}, {"Cicada Summer", 5, 207},
    {"Fen Water", 1, 223}, {"Reed Beds", 2, 261}, {"Lantern Fly", 3, 182},
    {"Quiet Reach", 4, 248}, {"Aperture", 1, 214}, {"Shutter Count", 2, 197},
    {"Contact Sheet", 3, 229}, {"Wide Open", 4, 255}, {"Soft Focus", 5, 186},
    {"Push Processing", 6, 233}, {"Silver Halide", 7, 202}, {"Deep Field", 1, 271},
    {"Star Trails", 2, 298}, {"Sidereal", 3, 264}, {"Occultation", 4, 249},
    {"First Quarter", 5, 231},
};

int kSeedTrackCount = sizeof(kTitles) / sizeof(kTitles[0]);

} // namespace

MusicDB& MusicDB::get() {
    static MusicDB instance;
    return instance;
}

MusicDB::MusicDB() : mNextPlaylistId(100) {
    long artistId = 1;
    long albumId = 1;
    long trackId = 1;
    int titleIdx = 0;

    for (const SeedArtist& sa : kSeed) {
        mArtists.push_back({artistId, sa.name});
        for (const SeedAlbum& sal : sa.albums) {
            if (sal.name == nullptr || sal.trackCount == 0) break;
            mAlbums.push_back({albumId, sal.name, artistId});
            for (int t = 0; t < sal.trackCount; t++) {
                const SeedTrack& st = kTitles[titleIdx % kSeedTrackCount];
                titleIdx++;
                mTracks.push_back({trackId, st.title, artistId, albumId, t + 1, st.secs * 1000});
                trackId++;
            }
            albumId++;
        }
        artistId++;
    }

    // Playlists (the original ships none, but the facade wants the screen
    // populated; ids in the 100+ range so they never collide with track ids).
    const long all = 0;
    (void)all;
    Playlist fav;
    fav.id = 100;
    fav.name = "Favorite tracks";
    for (int i = 0; i < 8 && i < (int)mTracks.size(); i++)
        fav.trackIds.push_back(mTracks[i * 3 + 1].id);
    mPlaylists.push_back(fav);

    Playlist road;
    road.id = 101;
    road.name = "Road trip";
    for (int i = 0; i < 12 && i < (int)mTracks.size(); i++)
        road.trackIds.push_back(mTracks[(i * 5 + 2) % mTracks.size()].id);
    mPlaylists.push_back(road);

    Playlist late;
    late.id = 102;
    late.name = "Late night";
    for (int i = 0; i < 6 && i < (int)mTracks.size(); i++)
        late.trackIds.push_back(mTracks[(i * 7 + 4) % mTracks.size()].id);
    mPlaylists.push_back(late);
}

std::vector<Album> MusicDB::albumsForArtist(long artistId) const {
    std::vector<Album> out;
    for (const Album& a : mAlbums)
        if (a.artistId == artistId) out.push_back(a);
    return out;
}

std::vector<Track> MusicDB::tracksForArtist(long artistId) const {
    // ALBUM_KEY,TRACK order: walk the album list (already in seed order),
    // collecting each album's tracks in trackNo order.
    std::vector<Track> out;
    for (const Album& a : mAlbums) {
        if (a.artistId != artistId) continue;
        for (const Track& t : mTracks)
            if (t.albumId == a.id) out.push_back(t);
    }
    return out;
}

std::vector<Track> MusicDB::tracksForAlbum(long albumId) const {
    std::vector<Track> out;
    for (const Track& t : mTracks)
        if (t.albumId == albumId) out.push_back(t);
    return out;
}

std::vector<Track> MusicDB::tracksForPlaylist(long playlistId) const {
    std::vector<Track> out;
    const Playlist* pl = playlist(playlistId);
    if (pl == nullptr) return out;
    for (long id : pl->trackIds)
        if (const Track* t = track(id)) out.push_back(*t);
    return out;
}

std::vector<long> MusicDB::songIdsForArtist(long artistId) const {
    std::vector<long> ids;
    for (const Track& t : tracksForArtist(artistId)) ids.push_back(t.id);
    return ids;
}

std::vector<long> MusicDB::songIdsForAlbum(long albumId) const {
    std::vector<long> ids;
    for (const Track& t : tracksForAlbum(albumId)) ids.push_back(t.id);
    return ids;
}

std::vector<long> MusicDB::songIdsForPlaylist(long playlistId) const {
    std::vector<long> ids;
    for (const Track& t : tracksForPlaylist(playlistId)) ids.push_back(t.id);
    return ids;
}

std::vector<long> MusicDB::allSongIds() const {
    std::vector<long> ids;
    for (const Track& t : mTracks) ids.push_back(t.id);
    return ids;
}

const Track* MusicDB::track(long id) const {
    for (const Track& t : mTracks)
        if (t.id == id) return &t;
    return nullptr;
}

const Album* MusicDB::album(long id) const {
    for (const Album& a : mAlbums)
        if (a.id == id) return &a;
    return nullptr;
}

const Artist* MusicDB::artist(long id) const {
    for (const Artist& a : mArtists)
        if (a.id == id) return &a;
    return nullptr;
}

const Playlist* MusicDB::playlist(long id) const {
    for (const Playlist& p : mPlaylists)
        if (p.id == id) return &p;
    return nullptr;
}

int MusicDB::albumCountForArtist(long artistId) const {
    int n = 0;
    for (const Album& a : mAlbums)
        if (a.artistId == artistId) n++;
    return n;
}

int MusicDB::trackCountForArtist(long artistId) const {
    int n = 0;
    for (const Track& t : mTracks)
        if (t.artistId == artistId) n++;
    return n;
}

long MusicDB::createPlaylist(const std::string& name) {
    Playlist pl;
    pl.id = mNextPlaylistId++;
    pl.name = name;
    mPlaylists.push_back(pl);
    return pl.id;
}

} // namespace music
} // namespace cdroid
