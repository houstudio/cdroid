// FFmpegPlayerBackend — the real decode/output engine behind
// MediaPlaybackService's PlayerBackend seam. FFmpeg demux+decode on a worker
// thread, swresample to S16 stereo 48k, RtAudio output pulling from a ring
// buffer. Replaces the wall-clock driver without touching queue logic.
#ifndef __REMUSIC_FFMPEGBACKEND_H__
#define __REMUSIC_FFMPEGBACKEND_H__

#ifdef REMUSIC_ENABLE_FFMPEG

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <rtaudio/RtAudio.h>

#include "mediaplaybackservice.h"

namespace remusic {

class FFmpegPlayerBackend : public PlayerBackend {
public:
    FFmpegPlayerBackend();
    ~FFmpegPlayerBackend() override;

    void open(const std::string& path, int durationMs) override;
    void start() override;
    void pause() override;
    void stop() override;
    void seek(int msec) override;
    bool isPlaying() const override { return mPlaying; }
    bool errored() const override { return mOpenFailed; }
    int position() const override;   // ms
    int duration() const override;   // ms
    void setOnPlayingChanged(const std::function<void(bool)>& cb) override;

private:
    void decodeLoop();
    bool openInput();
    void closeInput();
    void flushCodec();
    int readFromRing(int16_t* out, int frames);
    int writeToRing(const int16_t* in, int frames);
    bool ringFree() const;

    static int32_t audioCallback(void* outputBuffer, void* /*inputBuffer*/,
            unsigned int nBufferFrames, double /*streamTime*/,
            uint32_t /*status*/, void* userData);

    std::string mPath;
    std::function<void(bool)> mOnPlayingChanged;   // decode thread fires on the async flip
    std::atomic<bool> mWantPlaying{false};   // desired state (UI thread sets)
    std::atomic<bool> mPendingOpen{false};   // decode thread owns the actual open
    std::atomic<bool> mOpenFailed{false};
    std::atomic<bool> mPlaying{false};       // actual: stream running
    std::atomic<bool> mQuit{false};
    std::atomic<bool> mSeeking{false};
    std::atomic<int> mSeekTargetMs{-1};
    std::atomic<long long> mFramesPlayed{0};   // output frames since open/seek base
    std::atomic<long long> mBaseMs{0};         // stream time already accounted
    long long mDurationMs = 0;
    int mOutRate = 48000;          // follows the source rate (see openInput)

    // FFmpeg state (decode thread owned; guarded by mStateMutex for open/close)
    void* mFmtCtx = nullptr;        // AVFormatContext*
    void* mCodecCtx = nullptr;      // AVCodecContext*
    void* mSwr = nullptr;           // SwrContext*
    void* mPacket = nullptr;        // AVPacket*
    void* mFrame = nullptr;         // AVFrame*
    int mStreamIndex = -1;

    std::mutex mStateMutex;      // FFmpeg state (decode thread's open/close)
    std::mutex mPathMutex;       // the staged path (UI thread writes, decode reads)
    std::mutex mRingMutex;
    std::condition_variable mRingSpace;
    std::thread mDecodeThread;
    std::vector<int16_t> mRing;
    size_t mRingHead = 0;           // read pos (frames)
    size_t mRingFill = 0;           // frames buffered
    static constexpr size_t RING_FRAMES = 48000 * 4;   // ~4s at 48k stereo pairs

    std::unique_ptr<RtAudio> mAudio;
};

} // namespace remusic
#endif // REMUSIC_ENABLE_FFMPEG
#endif
