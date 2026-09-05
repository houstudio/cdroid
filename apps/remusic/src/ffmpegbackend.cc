// FFmpegPlayerBackend — decode thread + RtAudio sink. All FFmpeg types stay
// behind void* in the header; the includes live only here.
#include "ffmpegbackend.h"

#include "musicprovider.h"

#ifdef REMUSIC_ENABLE_FFMPEG

#include <algorithm>
#include <cstring>

// This vcpkg FFmpeg ships without __cplusplus guards in its headers — wrap
// the includes manually or every symbol links as C++-mangled.
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
}

#include <porting/cdlog.h>

namespace remusic {

static constexpr int OUT_CHANNELS = 2;

FFmpegPlayerBackend::FFmpegPlayerBackend() {
    mRing.resize(RING_FRAMES * OUT_CHANNELS);   // sized for 48k worst case
    mAudio = std::make_unique<RtAudio>(RtAudio::Api::LINUX_ALSA == RtAudio::Api::UNSPECIFIED
            ? RtAudio::Api::UNSPECIFIED : RtAudio::Api::UNSPECIFIED);
    mDecodeThread = std::thread([this] { decodeLoop(); });
}

FFmpegPlayerBackend::~FFmpegPlayerBackend() {
    mQuit = true;
    mRingSpace.notify_all();
    if (mDecodeThread.joinable()) mDecodeThread.join();
    if (mAudio) {
        if (mAudio->isStreamRunning()) mAudio->abortStream();
        if (mAudio->isStreamOpen()) mAudio->closeStream();
    }
    closeInput();
}

bool FFmpegPlayerBackend::openInput() {
    AVFormatContext* fmt = nullptr;
    // ccMixter's media host hotlink-checks Referer (403 without one); other
    // sources don't care, so send it only for that domain.
    AVDictionary* opts = nullptr;
    if (mPath.find("ccmixter.org") != std::string::npos)
        av_dict_set(&opts, "referer", "https://ccmixter.org/", 0);
    const int openRc = avformat_open_input(&fmt, mPath.c_str(), nullptr, &opts);
    av_dict_free(&opts);
    if (openRc != 0 || fmt == nullptr) {
        LOGE("ffmpeg: open failed %s", mPath.c_str());
        return false;
    }
    if (avformat_find_stream_info(fmt, nullptr) < 0) {
        avformat_close_input(&fmt);
        return false;
    }
    const AVStream* st = nullptr;
    int idx = av_find_best_stream(fmt, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (idx < 0) {
        avformat_close_input(&fmt);
        return false;
    }
    st = fmt->streams[idx];
    const AVCodec* codec = avcodec_find_decoder(st->codecpar->codec_id);
    if (codec == nullptr) {
        avformat_close_input(&fmt);
        return false;
    }
    AVCodecContext* cctx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(cctx, st->codecpar);
    if (avcodec_open2(cctx, codec, nullptr) < 0) {
        avcodec_free_context(&cctx);
        avformat_close_input(&fmt);
        return false;
    }
    // Output rate follows the source: this vcpkg swresample asserts on some
    // resampling paths (out_rate != in_rate at swresample.c:895), and the
    // position math is rate-relative anyway.
    mOutRate = cctx->sample_rate > 0 ? cctx->sample_rate : 48000;
    SwrContext* swr = nullptr;
    AVChannelLayout outLayout = AV_CHANNEL_LAYOUT_STEREO;
    if (swr_alloc_set_opts2(&swr,
            &outLayout, AV_SAMPLE_FMT_S16, mOutRate,
            (AVChannelLayout*) &cctx->ch_layout, cctx->sample_fmt, cctx->sample_rate,
            0, nullptr) < 0 || swr == nullptr || swr_init(swr) < 0) {
        // swr_alloc_set_opts2 only ALLOCATES: without swr_init every later
        // swr_convert rejects with "Context has not been initialized" and the
        // ring stays empty — silent playback.
        if (swr) swr_free(&swr);
        avcodec_free_context(&cctx);
        avformat_close_input(&fmt);
        return false;
    }
    // ID3/container metadata feeds the provider's tag cache (title/artist/
    // album win over the filename convention on the next scan).
    {
        const AVDictionaryEntry* e;
        std::string title, artist, album;
        if ((e = av_dict_get(fmt->metadata, "title", nullptr, 0))) title = e->value;
        if ((e = av_dict_get(fmt->metadata, "artist", nullptr, 0))) artist = e->value;
        if ((e = av_dict_get(fmt->metadata, "album", nullptr, 0))) album = e->value;
        if (title.empty() && (e = av_dict_get(st->metadata, "title", nullptr, 0))) title = e->value;
        if (artist.empty() && (e = av_dict_get(st->metadata, "artist", nullptr, 0))) artist = e->value;
        if (album.empty() && (e = av_dict_get(st->metadata, "album", nullptr, 0))) album = e->value;
        long long durMs = st->duration > 0
                ? (long long)(st->duration * av_q2d(st->time_base) * 1000.0)
                : (fmt->duration > 0 ? (long long)(fmt->duration / (AV_TIME_BASE / 1000)) : 0);
        MusicProvider::get().updateTags(mPath, artist, title, album, durMs);
    }
    mFmtCtx = fmt;
    mCodecCtx = cctx;
    mSwr = swr;
    mStreamIndex = idx;
    mPacket = av_packet_alloc();
    mFrame = av_frame_alloc();
    mDurationMs = st->duration > 0
            ? (long long)(st->duration * av_q2d(st->time_base) * 1000.0)
            : (fmt->duration > 0 ? (long long)(fmt->duration / (AV_TIME_BASE / 1000)) : 0);
    return true;
}

void FFmpegPlayerBackend::closeInput() {
    if (mFrame) { av_frame_free((AVFrame**)&mFrame); mFrame = nullptr; }
    if (mPacket) { av_packet_free((AVPacket**)&mPacket); mPacket = nullptr; }
    if (mSwr) { swr_free((SwrContext**)&mSwr); mSwr = nullptr; }
    if (mCodecCtx) { avcodec_free_context((AVCodecContext**)&mCodecCtx); mCodecCtx = nullptr; }
    if (mFmtCtx) { avformat_close_input((AVFormatContext**)&mFmtCtx); mFmtCtx = nullptr; }
    mStreamIndex = -1;
}

void FFmpegPlayerBackend::flushCodec() {
    if (mCodecCtx) avcodec_flush_buffers((AVCodecContext*)mCodecCtx);
    std::lock_guard<std::mutex> l(mRingMutex);
    mRingFill = 0;
    mRingHead = 0;
}

void FFmpegPlayerBackend::open(const std::string& path, int /*durationMs*/) {
    // Async prepare (Android MediaPlayer semantics): the UI thread only
    // stages the path; avformat_open_input on a garbage file can burn the
    // thread it runs on, so the decode thread owns the actual open.
    {
        // Never take mStateMutex here: the decode thread may hold it for the
        // whole (slow) openInput of the previous file — the UI thread would
        // stall inside open(). The path handoff has its own tiny lock.
        std::lock_guard<std::mutex> l(mPathMutex);
        mPath = path;
        mPendingOpen = true;
        mWantPlaying = false;
    }
    mDurationMs = 0;
    mBaseMs = 0;
    mFramesPlayed = 0;
    mSeekTargetMs = -1;
}

void FFmpegPlayerBackend::start() {
    mWantPlaying = true;   // decode thread opens/starts the stream once ready
    if (mFmtCtx == nullptr || mPlaying) return;
    if (!mAudio->isStreamOpen()) {
        RtAudio::StreamParameters params;
        params.deviceId = mAudio->getDefaultOutputDevice();
        params.nChannels = OUT_CHANNELS;
        params.firstChannel = 0;
        unsigned int bufferFrames = 1024;
#if RTAUDIO_VERSION_MAJOR > 5
        mAudio->openStream(&params, nullptr, RTAUDIO_SINT16, (unsigned)mOutRate, &bufferFrames,
                &FFmpegPlayerBackend::audioCallback, this);
#else
        mAudio->openStream(&params, nullptr, RTAUDIO_SINT16, (unsigned)mOutRate, &bufferFrames,
                &FFmpegPlayerBackend::audioCallback, this);
#endif
    }
    if (mAudio->isStreamOpen()) {
        if (!mAudio->isStreamRunning()) mAudio->startStream();
        mPlaying = true;
        mRingSpace.notify_all();
    } else {
        LOGE("ffmpeg: RtAudio open failed");
    }
}

void FFmpegPlayerBackend::pause() {
    mWantPlaying = false;
    if (!mPlaying) return;
    mPlaying = false;
    if (mAudio && mAudio->isStreamRunning()) mAudio->stopStream();
}

void FFmpegPlayerBackend::stop() {
    pause();
    std::lock_guard<std::mutex> l(mRingMutex);
    mRingFill = 0;
    mFramesPlayed = 0;
    mBaseMs = 0;
}

void FFmpegPlayerBackend::seek(int msec) {
    if (mFmtCtx == nullptr) return;
    pause();
    mBaseMs = msec;
    mFramesPlayed = 0;
    AVStream* st = ((AVFormatContext*)mFmtCtx)->streams[mStreamIndex];
    const int64_t ts = (int64_t)(msec / (av_q2d(st->time_base) * 1000.0));
    av_seek_frame((AVFormatContext*)mFmtCtx, mStreamIndex, ts, AVSEEK_FLAG_BACKWARD);
    flushCodec();
    if (mDurationMs > 0) start();   // restore prior playing state (lean: resume)
}

int FFmpegPlayerBackend::position() const {
    return (int)(mBaseMs + mFramesPlayed * 1000LL / (mOutRate ? mOutRate : 48000));
}

int FFmpegPlayerBackend::duration() const {
    // 0 = live/unknown (radio streams, failed opens) — the UI shows 00:00
    // and the service's tick never auto-advances.
    return mDurationMs > 0 ? (int)mDurationMs : 0;
}

bool FFmpegPlayerBackend::ringFree() const {
    return mRingFill < RING_FRAMES;
}

int FFmpegPlayerBackend::writeToRing(const int16_t* in, int frames) {
    std::lock_guard<std::mutex> l(mRingMutex);
    int written = 0;
    while (written < frames && mRingFill < RING_FRAMES) {
        const size_t tail = (mRingHead + mRingFill) % RING_FRAMES;
        const size_t chunk = std::min<size_t>(std::min<size_t>(frames - written, RING_FRAMES - mRingFill),
                RING_FRAMES - tail);   /* one memcpy span at most */
        memcpy(&mRing[tail * OUT_CHANNELS], in + written * OUT_CHANNELS,
                chunk * OUT_CHANNELS * sizeof(int16_t));
        mRingFill += chunk;
        written += chunk;
    }
    return written;
}

int FFmpegPlayerBackend::readFromRing(int16_t* out, int frames) {
    std::lock_guard<std::mutex> l(mRingMutex);
    int got = 0;
    while (got < frames && mRingFill > 0) {
        const size_t chunk = std::min<size_t>(frames - got, mRingFill);
        memcpy(out + got * OUT_CHANNELS, &mRing[mRingHead * OUT_CHANNELS],
                chunk * OUT_CHANNELS * sizeof(int16_t));
        mRingHead = (mRingHead + chunk) % RING_FRAMES;
        mRingFill -= chunk;
        got += chunk;
    }
    mFramesPlayed += got;
    return got;
}

int32_t FFmpegPlayerBackend::audioCallback(void* outputBuffer, void*, unsigned int nBufferFrames,
        double, uint32_t, void* userData) {
    auto* self = (FFmpegPlayerBackend*) userData;
    const int got = self->readFromRing((int16_t*) outputBuffer, (int)nBufferFrames);
    if (got < (int)nBufferFrames)
        memset((int16_t*) outputBuffer + got * OUT_CHANNELS, 0,
                ((int)nBufferFrames - got) * OUT_CHANNELS * sizeof(int16_t));
    self->mRingSpace.notify_all();
    return 0;
}

void FFmpegPlayerBackend::decodeLoop() {
    std::vector<int16_t> pcm;
    while (!mQuit) {
        // An idle nap is fine — but never before a staged open ran (the
        // prepare state machine below is the only thing that fills mFmtCtx).
        if ((mFmtCtx == nullptr && !mPendingOpen)
                || (!mPlaying && mRingFill > RING_FRAMES / 2)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }
        // Backpressure: wait for ring space while playing (or idle-drain).
        {
            std::unique_lock<std::mutex> l(mRingMutex);
            mRingSpace.wait_for(l, std::chrono::milliseconds(200),
                    [this] { return mQuit || mRingFill < RING_FRAMES; });
        }

        // Async prepare + stream state machine (all FFmpeg/RtAudio calls that
        // can block live on THIS thread).
        {
            std::lock_guard<std::mutex> state(mStateMutex);
            if (mPendingOpen) {
                mPendingOpen = false;
                mOpenFailed = false;
                if (mAudio && mAudio->isStreamRunning()) { mAudio->stopStream(); }
                mPlaying = false;
                closeInput();
                std::string path;
                { std::lock_guard<std::mutex> p(mPathMutex); path = mPath; }
                if (path.empty() || !openInput()) {
                    // Corrupt file / dead URL: surface it instead of silently
                    // sitting at 00:00 (TRACK_ERROR in the service tick).
                    mOpenFailed = true;
                    mWantPlaying = false;
                } else {
                    std::lock_guard<std::mutex> r(mRingMutex);
                    mRingFill = 0;
                    mRingHead = 0;
                }
            }
            if (!mWantPlaying && mPlaying) {
                mPlaying = false;
                if (mAudio && mAudio->isStreamRunning()) mAudio->stopStream();
            }
            if (mWantPlaying && !mPlaying && mFmtCtx != nullptr) {
                if (!mAudio->isStreamOpen()) {
                    RtAudio::StreamParameters params;
                    params.deviceId = mAudio->getDefaultOutputDevice();
                    params.nChannels = OUT_CHANNELS;
                    params.firstChannel = 0;
                    unsigned int bufferFrames = 1024;
                    mAudio->openStream(&params, nullptr, RTAUDIO_SINT16,
                            (unsigned)mOutRate, &bufferFrames,
                            &FFmpegPlayerBackend::audioCallback, this);
                }
                if (mAudio->isStreamOpen()) {
                    if (!mAudio->isStreamRunning()) mAudio->startStream();
                    mPlaying = true;
                } else {
                    LOGE("ffmpeg: RtAudio open failed");
                    mWantPlaying = false;
                }
            }
        }
        if (mFmtCtx == nullptr) continue;

        // open()/close() swap the FFmpeg state under mStateMutex; every
        // demux+decode pass takes it too or the pointers race.
        std::lock_guard<std::mutex> stateLock(mStateMutex);
        if (mFmtCtx == nullptr) continue;
        AVPacket* pkt = (AVPacket*) mPacket;
        AVFrame* frm = (AVFrame*) mFrame;
        if (av_read_frame((AVFormatContext*) mFmtCtx, pkt) < 0) {
            // EOF: if looping is desired it is the service's job to advance;
            // signal end-of-track by letting the ring run dry.
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            av_packet_unref(pkt);
            continue;
        }
        if (pkt->stream_index != mStreamIndex) {
            av_packet_unref(pkt);
            continue;
        }
        if (avcodec_send_packet((AVCodecContext*) mCodecCtx, pkt) == 0) {
            av_packet_unref(pkt);
            while (avcodec_receive_frame((AVCodecContext*) mCodecCtx, frm) == 0) {
                const int outSamples = swr_get_out_samples((SwrContext*) mSwr,
                        frm->nb_samples);
                if (outSamples <= 0) continue;
                pcm.resize((size_t) outSamples * OUT_CHANNELS);
                uint8_t* outPlane = (uint8_t*) pcm.data();
                const int converted = swr_convert((SwrContext*) mSwr, &outPlane, outSamples,
                        (const uint8_t**) frm->data, frm->nb_samples);
                if (converted > 0) writeToRing(pcm.data(), converted);
                av_frame_unref(frm);
            }
        } else {
            av_packet_unref(pkt);
        }
    }
}

} // namespace remusic
#endif // REMUSIC_ENABLE_FFMPEG
