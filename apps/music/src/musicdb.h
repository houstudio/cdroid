/*********************************************************************************
 * Mock media store for the AOSP Music port (UI facade).
 *
 * The original app reads artists/albums/tracks/playlists through
 * MediaStore content URIs backed by the platform MediaProvider. CDROID has
 * no ContentResolver/MediaStore, so this module plays that role with a
 * hardcoded in-memory database: same entities (artist/album/track/playlist
 * ids) and the same query shapes (song lists per artist/album/playlist).
 * There are no audio files behind the tracks — see mediaplaybackservice.h.
 *********************************************************************************/
#ifndef CDROID_MUSIC_MUSICDB_H
#define CDROID_MUSIC_MUSICDB_H

#include <string>
#include <vector>

namespace cdroid {
namespace music {

struct Artist {
    long id;
    std::string name;
};

struct Album {
    long id;
    std::string name;
    long artistId;
};

struct Track {
    long id;
    std::string title;
    long artistId;
    long albumId;
    int trackNo;       // 1-based index within the album
    int durationMs;    // fake duration (2–5 min range in the seed data)
};

struct Playlist {
    long id;
    std::string name;
    std::vector<long> trackIds;
};

class MusicDB {
public:
    static MusicDB& get();

    const std::vector<Artist>& artists() const { return mArtists; }
    const std::vector<Album>& albums() const { return mAlbums; }
    const std::vector<Track>& tracks() const { return mTracks; }
    std::vector<Playlist>& playlists() { return mPlaylists; }

    // MediaStore.Audio.Albums for a given artist
    std::vector<Album> albumsForArtist(long artistId) const;
    // MediaStore.Audio.Media "ARTIST_ID=? AND IS_MUSIC=1", ALBUM_KEY,TRACK order
    std::vector<Track> tracksForArtist(long artistId) const;
    // MediaStore.Audio.Media "ALBUM_ID=?", TRACK order
    std::vector<Track> tracksForAlbum(long albumId) const;
    // MediaStore.Audio.Playlists.Members
    std::vector<Track> tracksForPlaylist(long playlistId) const;

    std::vector<long> songIdsForArtist(long artistId) const;
    std::vector<long> songIdsForAlbum(long albumId) const;
    std::vector<long> songIdsForPlaylist(long playlistId) const;
    std::vector<long> allSongIds() const;

    const Track* track(long id) const;
    const Album* album(long id) const;
    const Artist* artist(long id) const;
    const Playlist* playlist(long id) const;

    int albumCountForArtist(long artistId) const;
    int trackCountForArtist(long artistId) const;

    long createPlaylist(const std::string& name);   // returns the new id

private:
    MusicDB();

    std::vector<Artist> mArtists;
    std::vector<Album> mAlbums;
    std::vector<Track> mTracks;
    std::vector<Playlist> mPlaylists;
    long mNextPlaylistId;
};

} // namespace music
} // namespace cdroid

#endif // CDROID_MUSIC_MUSICDB_H
