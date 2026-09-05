// Port of com.wm.remusic.service.MusicPlayer — the static facade the whole UI
// calls. The AIDL binder surface collapses onto the in-process singleton
// (bindToService/unbindFromService become no-ops; init() wires the context).
#ifndef __REMUSIC_MUSICPLAYER_H__
#define __REMUSIC_MUSICPLAYER_H__

#include <map>
#include <string>
#include <vector>

#include "musicinfo.h"
#include "mediaplaybackservice.h"

namespace remusic {

class MusicPlayer {
public:
    static void init(cdroid::Context& context);

    static void next() { service().next(); }
    static void previous(bool force) { service().previous(force); }
    static void playOrPause() { service().playOrPause(); }
    static void pause() { service().pause(); }
    static void play() { service().play(); }
    static void stop() { service().stop(); }
    static void seek(long msec) { service().seek(msec); }
    static void seekRelative(long delta) { service().seekRelative(delta); }
    static void setQueuePosition(int position) { service().setQueuePosition(position); }
    static void moveQueueItem(int from, int to) { service().moveQueueItem(from, to); }
    static int  removeTrack(long id) { return service().removeTrack(id); }
    static void cycleRepeat() { service().cycleRepeat(); }
    static void cycleShuffle() { service().cycleShuffle(); }
    static void timing(int msec) { service().timing(msec); }
    static bool timingActive() { return service().timingActive(); }
    static void setShuffleMode(int mode) { service().setShuffleMode(mode); }

    static void playAll(const std::map<long, MusicInfo>& infos,
                        const std::vector<long>& list, int position, bool forceShuffle) {
        service().playAll(infos, list, position, forceShuffle);
    }
    static void playNext(const std::map<long, MusicInfo>& infos,
                         const std::vector<long>& list) {
        service().playNext(infos, list);
    }

    static bool isPlaying() { return service().isPlaying(); }
    static long position() { return service().position(); }
    static long duration() { return service().duration(); }
    static int  shuffleMode() { return service().shuffleMode(); }
    static int  repeatMode() { return service().repeatMode(); }

    static std::string getTrackName() { return service().trackName(); }
    static std::string getArtistName() { return service().artistName(); }
    static std::string getAlbumName() { return service().albumName(); }
    static std::string getAlbumPath() { return service().albumPath(); }
    static std::string getPath() { return service().path(); }
    static long getCurrentAlbumId() {
        const MusicInfo* i = service().currentTrackInfo();
        return i ? i->albumId : -1;
    }
    static long getCurrentAudioId() {
        const MusicInfo* i = service().currentTrackInfo();
        return i ? i->songId : -1;
    }
    static long getCurrentArtistId() {
        const MusicInfo* i = service().currentTrackInfo();
        return i ? i->artistId : -1;
    }
    static bool isTrackLocal() { return true; }   // online path trimmed
    static const MusicInfo* currentTrackInfo() { return service().currentTrackInfo(); }

    static MusicTrack getCurrentTrack() { return service().currentTrack(); }
    static MusicTrack getTrack(int index) { return service().track(index); }
    static const std::vector<long>& getQueue() { return service().queue(); }
    static const std::map<long, MusicInfo>& getPlayinfos() { return service().playInfos(); }
    static int  getQueueSize() { return service().queueSize(); }
    static int  getQueuePosition() { return service().queuePosition(); }
    static long getQueueItemAtPosition(int position) {
        const auto& q = service().queue();
        return position >= 0 && position < (int)q.size() ? q[position] : -1;
    }
    static long getNextAudioId() { return service().nextAudioId(); }
    static long getPreviousAudioId() { return service().previousAudioId(); }

private:
    static MediaPlaybackService& service() { return MediaPlaybackService::getInstance(); }
};

} // namespace remusic
#endif
