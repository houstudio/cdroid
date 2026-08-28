/*********************************************************************************
 * In-process facade of com.android.music.MediaPlaybackService.
 *
 * The original is a 2200-line foreground Service wrapping MediaPlayer with
 * MediaSession/notification/audio-focus plumbing. CDROID has no Service,
 * no MediaPlayer C++ class (the MP* HAL is stubbed on x64) and no media
 * session stack, so this facade keeps the surface the UI consumes — queue,
 * position/duration, play/pause/seek/next/prev, shuffle/repeat modes and
 * META_CHANGED/PLAYSTATE_CHANGED/QUEUE_CHANGED notifications — but "plays"
 * nothing: position is a wall-clock-driven counter that advances while
 * mPlaying and auto-advances at the (fake) track end.
 *********************************************************************************/
#ifndef CDROID_MUSIC_MEDIAPLAYBACKSERVICE_H
#define CDROID_MUSIC_MEDIAPLAYBACKSERVICE_H

#include <functional>
#include <map>
#include <string>
#include <vector>

#include <core/handler.h>

namespace cdroid {
namespace music {

class MediaPlaybackService {
public:
    // MediaPlaybackService.SHUFFLE_*/REPEAT_* constants (original values).
    enum {
        SHUFFLE_NONE = 0,
        SHUFFLE_NORMAL = 1,
        SHUFFLE_AUTO = 2,
    };
    enum {
        REPEAT_NONE = 0,
        REPEAT_ALL = 1,
        REPEAT_CURRENT = 2,
    };
    // enqueue() insertion points.
    enum {
        LAST = 0,
        NEXT = 1,
        NOW = 2,
    };

    // The service's broadcast actions; delivered to registered listeners
    // instead of BroadcastReceivers (no BroadcastReceiver in CDROID).
    static constexpr const char* PLAYSTATE_CHANGED = "com.android.music.playstatechanged";
    static constexpr const char* META_CHANGED = "com.android.music.metachanged";
    static constexpr const char* QUEUE_CHANGED = "com.android.music.queuechanged";

    static MediaPlaybackService& getInstance();

    using Listener = std::function<void(const std::string& what)>;
    // `token` is the registering object's `this` — listeners are plain
    // callbacks, removeListener must be called before the owner dies.
    void addListener(void* token, const Listener& l);
    void removeListener(void* token);

    // --- queue / playback (IMediaPlaybackService surface) -------------------
    void open(const std::vector<long>& list, int position);
    void play();
    void pause();
    void stop();
    void next();     // auto next: honors repeat mode, wraps per original setNext()
    void prev();     // position >= 2000 restarts current, else previous (original prev())
    void seek(long pos);
    long position() const;
    long duration() const;
    bool isPlaying() const;
    bool isInitialized() const { return mPlayPos >= 0 && !mQueue.empty(); }

    void enqueue(const std::vector<long>& list, int where);
    std::vector<long> getQueue() const { return mQueue; }
    long getAudioId() const;
    int getQueuePosition() const { return mPlayPos; }

    void setShuffleMode(int mode) { mShuffleMode = mode; }
    int getShuffleMode() const { return mShuffleMode; }
    void setRepeatMode(int mode) { mRepeatMode = mode; }
    int getRepeatMode() const { return mRepeatMode; }

    std::string getTrackName() const;
    std::string getArtistName() const;
    std::string getAlbumName() const;
    long getAlbumId() const;
    long getArtistId() const;

    // Queue editing the TrackBrowser "nowplaying" screen uses.
    void moveQueueItem(int from, int to);
    void removeTrack(long audioId);
    void removeTracks(int first, int last);

private:
    MediaPlaybackService();

    void startTrack(int position, bool autoplay);
    void tick();                       // 200ms heartbeat: advance fake position
    void notify(const std::string& what);

    std::vector<long> mQueue;
    int mPlayPos = -1;                 // index into mQueue; -1 = nothing open
    bool mPlaying = false;
    bool mSupposedToBePlaying = false;
    long mPosMs = 0;                   // fake playback head
    long mLastTickUptimeMs = 0;        // wall clock anchor for position deltas
    int mShuffleMode = SHUFFLE_NONE;
    int mRepeatMode = REPEAT_ALL;      // original default (MediaPlaybackService ctor)
    bool mTickScheduled = false;

    std::map<void*, Listener> mListeners;
    Handler mTickHandler;              // main-looper heartbeat driver
    Runnable mTickRunnable;
};

} // namespace music
} // namespace cdroid

#endif // CDROID_MUSIC_MEDIAPLAYBACKSERVICE_H
