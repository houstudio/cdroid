// Port of com.wm.remusic.service.MediaService — the Timber-lineage playback
// service — as a CDROID in-process singleton (no Service/AIDL here; the UI's
// static MusicPlayer facade talks to ::getInstance() directly, the apps/music
// pattern). Online paths (RequestPlayUrl / proxy / lyrics fetch) are trimmed
// by construction: every track in this port is local.
//
// Decode/output sits behind PlayerBackend so a real decoder can replace the
// wall-clock driver without touching the queue/notify logic.
#ifndef __REMUSIC_MEDIAPLAYBACKSERVICE_H__
#define __REMUSIC_MEDIAPLAYBACKSERVICE_H__

#include <functional>
#include <map>
#include <string>
#include <vector>

#include <core/handler.h>
#include <core/systemclock.h>

#include "musicinfo.h"

namespace cdroid { class Context; }

namespace remusic {

// Broadcast action strings — identical to MediaService.java (the UI's
// PlaybackStatus receivers key on these literals).
struct MediaServiceActions {
    static constexpr const char* PLAYSTATE_CHANGED   = "com.wm.remusic.playstatechanged";
    static constexpr const char* POSITION_CHANGED    = "com.wm.remusic.positionchanged";
    static constexpr const char* META_CHANGED        = "com.wm.remusic.metachanged";
    static constexpr const char* PLAYLIST_ITEM_MOVED = "com.wm.remusic.mmoved";
    static constexpr const char* QUEUE_CHANGED       = "com.wm.remusic.queuechanged";
    static constexpr const char* PLAYLIST_CHANGED    = "com.wm.remusic.playlistchanged";
    static constexpr const char* REPEATMODE_CHANGED  = "com.wm.remusic.repeatmodechanged";
    static constexpr const char* SHUFFLEMODE_CHANGED = "com.wm.remusic.shufflemodechanged";
    static constexpr const char* TRACK_ERROR         = "com.wm.remusic.trackerror";
    static constexpr const char* REFRESH             = "com.wm.remusic.refresh";
    static constexpr const char* LRC_UPDATED         = "com.wm.remusic.updatelrc";
    static constexpr const char* MUSIC_CHANGED       = "com.wm.remusi.change_music";
};

// Timber constants (MediaService.java:162-167).
enum {
    SHUFFLE_NONE = 0,
    SHUFFLE_NORMAL = 1,
    SHUFFLE_AUTO = 2,
    REPEAT_NONE = 0,
    REPEAT_CURRENT = 1,
    REPEAT_ALL = 2,
};

/** Pluggable decode/output engine. The wall-clock backend ships first
 * (apps/music precedent); an FFmpeg+RtAudio engine slots in unchanged. */
class PlayerBackend {
public:
    virtual ~PlayerBackend() = default;
    virtual void open(const std::string& path, int durationMs) = 0;
    virtual void start() = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;
    virtual void seek(int msec) = 0;
    virtual bool isPlaying() const = 0;
    virtual int position() const = 0;   // ms
    virtual int duration() const = 0;   // ms
    /** Async prepare failed (bad stream / unreachable URL). */
    virtual bool errored() const { return false; }
};

/** MediaPlayer-free wall-clock driver: advances position on a 200ms Handler
 * tick while "playing" and fires the end-of-track callback. */
class WallClockPlayerBackend : public PlayerBackend {
public:
    void open(const std::string& path, int durationMs) override;
    void start() override;
    void pause() override;
    void stop() override;
    void seek(int msec) override;
    bool isPlaying() const override { return mPlaying; }
    int position() const override;
    int duration() const override { return mDuration; }

    // Ends-of-track are polled by the service's own tick.
    void setEndListener(std::function<void()> fn) { mOnEnd = std::move(fn); }
    void tick();
private:
    std::string mPath;
    int mDuration = 0;
    long mPositionMs = 0;
    long mStartedAtMs = 0;   // SystemClock.uptimeMillis when start() hit
    bool mPlaying = false;
    std::function<void()> mOnEnd;
};

class MediaPlaybackService {
public:
    static MediaPlaybackService& getInstance();
    /** Store the application context (main.cc, right after App construction)
     *  — prefs-backed queue persistence needs it. */
    static void init(cdroid::Context& context) { getInstance().mContext = &context; }

    using Listener = std::function<void(const std::string& action)>;
    void addListener(void* token, Listener listener);
    void removeListener(void* token);

    // ---- queue (AIDL surface, in-process) ----
    void open(const std::map<long, MusicInfo>& infos, const std::vector<long>& list, int position);
    void stop();
    void play();                       // CMDPLAY
    void pause();
    void playOrPause();                // TOGGLEPAUSE
    void next();
    void previous(bool force);
    void seek(long msec);
    void seekRelative(long deltaMs);
    void setQueuePosition(int position);
    void moveQueueItem(int from, int to);
    bool removeTrackAtPosition(long id, int position);
    int  removeTrack(long id);
    void playAll(const std::map<long, MusicInfo>& infos, const std::vector<long>& list,
                 int position, bool forceShuffle);
    void playNext(const std::map<long, MusicInfo>& infos, const std::vector<long>& list);

    // ---- modes / queries (MusicPlayer facade surface) ----
    void cycleRepeat();
    void cycleShuffle();

    // Sleep timer (TimingFragment -> MusicPlayer.timing): pause after msec,
    // 0 cancels. The original arms AlarmManager with a PAUSE_ACTION broadcast.
    void timing(int msec);
    bool timingActive() const { return mTimingDeadline != 0; }
    long timingRemainingMs() const;
    int  shuffleMode() const { return mShuffleMode; }
    void setShuffleMode(int mode);
    int  repeatMode() const { return mRepeatMode; }
    bool isPlaying() const;
    long position() const;
    long duration() const;
    long audioId() const;             // current track's songId (queue identity)
    std::string trackName() const;
    std::string artistName() const;
    std::string albumName() const;
    std::string albumPath() const;
    std::string path() const;
    MusicTrack currentTrack() const;
    MusicTrack track(int index) const;
    const MusicInfo* currentTrackInfo() const { return currentInfo(); }
    const std::vector<long>& queue() const { return mPlayList; }
    const std::map<long, MusicInfo>& playInfos() const { return mPlayInfos; }
    int  queueSize() const { return (int)mPlayList.size(); }
    int  queuePosition() const { return mPlayPos; }
    const std::vector<int>& history() const { return mHistory; }
    long nextAudioId() const;
    long previousAudioId() const;

    // ---- persistence (MusicPlaybackState: queue/history via prefs) ----
    void saveQueue(bool full);
    void restoreQueueIfNeeded();

    // RecentStore seam: most-recently-played ids (prefs-backed).
    void noteSongPlay(long songId);
    static std::vector<long> recentSongIds();

    void setBackend(PlayerBackend* backend);   // service owns a default

private:
    MediaPlaybackService();
    void notifyChange(const std::string& what);
    void setAndNotify(int& field, int value, const char* action);
    bool openCurrent();
    bool playCurrentSource(bool play);
    int  getNextPosition(bool force);
    int  getPreviousPosition(bool force);
    const MusicInfo* currentInfo() const;
    long nextRandomPosition();      // Shuffler (play-count-minimizing simplified)

    std::map<void*, Listener> mListeners;
    std::vector<long> mPlayList;
    std::map<long, MusicInfo> mPlayInfos;
    std::vector<int> mHistory;
    int mPlayPos = -1;
    int mShuffleMode = SHUFFLE_NONE;
    int mRepeatMode = REPEAT_ALL;
    bool mQueueIsSaveable = false;
    bool mServiceInUse = false;
    bool mErrorNotified = false;
    long mTimingDeadline = 0;       // uptimeMillis; 0 = timer off
    int mTimingGen = 0;             // cancels stale postDelayed firings

    PlayerBackend* mBackend = nullptr;      // owned default
    WallClockPlayerBackend mClock;
    cdroid::Handler mTickHandler;           // 200ms wall-clock heartbeat
    bool mTickScheduled = false;
    cdroid::Context* mContext = nullptr;    // application context (init())
    void scheduleTick();
};

} // namespace remusic
#endif
