// C++ port of AOSP DeskClock AsyncRingtonePlayer (packages/apps/DeskClock,
// src/com/android/deskclock/AsyncRingtonePlayer.kt). Controls alarm ringtone
// playback from a dedicated thread ("ringtone-player" HandlerThread in AOSP;
// here a std::thread with its own cdroid Looper) so the main thread never
// blocks on decode. The AOSP platform split (Ringtone vs MediaPlayer delegate)
// collapses to one backend: RingtoneBackend (FFmpeg decode + RtAudio output,
// the remusic pattern) behind DESKCLOCK_ENABLE_FFMPEG. Without it the calls
// degrade to logging — playback observability without audio. Crescendo keeps
// the upstream 50ms volume-adjustment cadence and dB-linear curve.
//
// Not ported (no cdroid counterparts): AudioManager stream volume/focus gates,
// in-call ringtone substitution (TelephonyManager), Vibrator (see Klaxons).
#ifndef __DESKCLOCK_ASYNCRINGTONEPLAYER_H__
#define __DESKCLOCK_ASYNCRINGTONEPLAYER_H__

#include <cstdint>
#include <mutex>
#include <string>
#include <thread>

#include <core/context.h>
#include <core/handler.h>
#include <core/uri.h>

namespace cdroid {
namespace deskclock {

class AsyncRingtonePlayer {
public:
    explicit AsyncRingtonePlayer(Context* context);
    ~AsyncRingtonePlayer();

    /** Plays the ringtone (AOSP play(uri, crescendoDuration)). */
    void play(Uri* ringtoneUri, int64_t crescendoDuration);
    /** Stops playing the ringtone. */
    void stop();

    /** AOSP getFallbackRingtoneUri: the in-app fallback when the chosen
     *  ringtone cannot be played — playing SOMETHING beats silence. */
    static Uri* getFallbackRingtoneUri(Context* context);

private:
    // Message codes used with the ringtone thread (AOSP companion values).
    static constexpr int EVENT_PLAY = 1;
    static constexpr int EVENT_STOP = 2;
    static constexpr int EVENT_VOLUME = 3;

    class RingtoneHandler;

    void postMessage(int messageCode, Uri* ringtoneUri, int64_t crescendoDuration,
            int64_t delayMillis);
    void scheduleVolumeAdjustment();

    // Ringtone-thread bodies (the AOSP PlaybackDelegate surface folded onto
    // this class; the actual decode/output sits in RingtoneBackend).
    bool handlePlay(Uri* ringtoneUri, int64_t crescendoDuration);
    void handleStop();
    bool handleAdjustVolume();

    static float computeVolume(int64_t currentTime, int64_t stopTime, int64_t duration);

    Context* mContext;
    std::mutex mSync;                 // guards mHandler creation (AOSP synchronized)
    RingtoneHandler* mHandler = nullptr;
    std::thread mThread;              // the "ringtone-player" thread
    bool mThreadStarted = false;

    // Delegate state — owned by the ringtone thread only.
    int64_t mCrescendoDuration = 0;   // duration over which to increase volume
    int64_t mCrescendoStopTime = 0;   // time at which the crescendo ceases; 0 = none
    void* mDelegate = nullptr;        // RingtoneBackend* when DESKCLOCK_ENABLE_FFMPEG
};

// App-side Uri→playable-path resolution shared with the Klaxons:
//   android.resource://cdroid.deskclock/<id> → pak asset extracted to a cache file
//   file://… or a bare path                     → the path itself
//   content://… (system settings uris)          → "" (no system provider)
std::string ringtonePathFromUri(Context* context, const Uri* uri);

} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_ASYNCRINGTONEPLAYER_H__
