// RingtoneBackend — the real decode/output engine behind
// AsyncRingtonePlayer's PlaybackDelegate seam (the remusic FFmpegPlayerBackend
// pattern applied to ringtones): FFmpeg whole-file decode to S16 stereo in
// memory on the caller's thread, RtAudio output streaming from that buffer
// with alarm-looping and a volume scalar the crescendo walks. Built only when
// the toolchain's vcpkg triplet has ffmpeg+rtaudio (see CMakeLists); absent
// that, AsyncRingtonePlayer degrades to its logging stub.
#ifndef __DESKCLOCK_RINGTONEBACKEND_H__
#define __DESKCLOCK_RINGTONEBACKEND_H__

#ifdef DESKCLOCK_ENABLE_FFMPEG

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <rtaudio/RtAudio.h>

namespace cdroid {
namespace deskclock {

class RingtoneBackend {
public:
    RingtoneBackend();
    ~RingtoneBackend();

    /** Whole-file decode of `path` (any ffmpeg container/codec: ogg/flac/...).
     *  Returns false when the file cannot be decoded; nothing is playing then. */
    bool open(const std::string& path);
    /** Start looped playback of the buffer opened by open(). */
    bool play();
    /** Stop and close the RtAudio stream (idempotent). */
    void stop();
    bool isPlaying() const { return mPlaying; }
    /** Scalar volume 0..1 applied in the audio callback (crescendo target). */
    void setVolume(float volume) { mVolume = volume; }

private:
    static int32_t audioCallback(void* outputBuffer, void* inputBuffer,
            unsigned int nBufferFrames, double streamTime, uint32_t status, void* userData);

    std::shared_ptr<RtAudio> mAudio;
    std::vector<int16_t> mPcm;      // S16 interleaved stereo
    unsigned int mSampleRate = 48000;
    size_t mCursor = 0;             // frame cursor into mPcm (callback-owned)
    std::atomic<bool> mPlaying{false};
    std::atomic<float> mVolume{1.0f};
    std::mutex mStreamMutex;        // guards openStream/closeStream vs callback teardown
};

} // namespace deskclock
} // namespace cdroid

#endif // DESKCLOCK_ENABLE_FFMPEG
#endif // __DESKCLOCK_RINGTONEBACKEND_H__
