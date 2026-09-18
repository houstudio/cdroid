// RingtoneBackend — see ringtonebackend.h. The decode half mirrors remusic's
// FFmpegPlayerBackend::openInput (avformat/avcodec/swresample boilerplate);
// the output half is a memory-streaming variant of its RtAudio ring: ringtones
// are whole-file small, so the ring collapses to a loop cursor + volume
// multiply in the callback.
#include "ringtonebackend.h"

#ifdef DESKCLOCK_ENABLE_FFMPEG

#include <algorithm>

#include <porting/cdlog.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
}

namespace cdroid {
namespace deskclock {

namespace {
constexpr int OUT_CHANNELS = 2;   // S16 interleaved stereo, like remusic's output
}

RingtoneBackend::RingtoneBackend() = default;

RingtoneBackend::~RingtoneBackend() {
    stop();
}

bool RingtoneBackend::open(const std::string& path) {
    stop();

    AVFormatContext* fmt = nullptr;
    if (avformat_open_input(&fmt, path.c_str(), nullptr, nullptr) != 0 || fmt == nullptr) {
        LOGE("ringtone: open failed %s", path.c_str());
        return false;
    }
    if (avformat_find_stream_info(fmt, nullptr) < 0) {
        avformat_close_input(&fmt);
        return false;
    }
    const int idx = av_find_best_stream(fmt, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (idx < 0) {
        avformat_close_input(&fmt);
        return false;
    }
    const AVStream* st = fmt->streams[idx];
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
    // resampling paths (out_rate != in_rate), as remusic's backend noted.
    mSampleRate = cctx->sample_rate > 0 ? (unsigned)cctx->sample_rate : 48000;
    SwrContext* swr = nullptr;
    AVChannelLayout outLayout = AV_CHANNEL_LAYOUT_STEREO;
    if (swr_alloc_set_opts2(&swr,
            &outLayout, AV_SAMPLE_FMT_S16, mSampleRate,
            (AVChannelLayout*) &cctx->ch_layout, cctx->sample_fmt, cctx->sample_rate,
            0, nullptr) < 0 || swr == nullptr || swr_init(swr) < 0) {
        // swr_alloc_set_opts2 only ALLOCATES; without swr_init every later
        // swr_convert rejects — silent playback (remusic hit this first).
        if (swr) swr_free(&swr);
        avcodec_free_context(&cctx);
        avformat_close_input(&fmt);
        return false;
    }

    // Whole-file decode into mPcm: ringtones are seconds long, so the
    // streaming ring buffer remusic needs for songs collapses to memory.
    AVPacket* packet = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();
    mPcm.clear();
    while (av_read_frame(fmt, packet) >= 0) {
        if (packet->stream_index == idx && avcodec_send_packet(cctx, packet) >= 0) {
            while (avcodec_receive_frame(cctx, frame) >= 0) {
                // Estimate the worst-case S16 size then convert; swr keeps its
                // own internal buffering so a short out count is fine.
                const int64_t delay = swr_get_delay(swr, cctx->sample_rate);
                const int outCount = (int)(delay + frame->nb_samples + 1);
                const size_t oldSize = mPcm.size();
                mPcm.resize(oldSize + (size_t)outCount * OUT_CHANNELS);
                uint8_t* dstPlanes[1] = { reinterpret_cast<uint8_t*>(&mPcm[oldSize]) };
                const int converted = swr_convert(swr,
                        dstPlanes, outCount,
                        (const uint8_t**)frame->data, frame->nb_samples);
                if (converted < 0) {
                    mPcm.resize(oldSize);
                    break;
                }
                mPcm.resize(oldSize + (size_t)converted * OUT_CHANNELS);
            }
        }
        av_packet_unref(packet);
    }
    // Flush the decoder tail.
    avcodec_send_packet(cctx, nullptr);
    while (avcodec_receive_frame(cctx, frame) >= 0) {
        const int64_t delay = swr_get_delay(swr, cctx->sample_rate);
        const int outCount = (int)(delay + frame->nb_samples + 1);
        const size_t oldSize = mPcm.size();
        mPcm.resize(oldSize + (size_t)outCount * OUT_CHANNELS);
        uint8_t* dstPlanes[1] = { reinterpret_cast<uint8_t*>(&mPcm[oldSize]) };
        const int converted = swr_convert(swr,
                dstPlanes, outCount,
                (const uint8_t**)frame->data, frame->nb_samples);
        if (converted > 0) mPcm.resize(oldSize + (size_t)converted * OUT_CHANNELS);
        else mPcm.resize(oldSize);
    }
    av_frame_free(&frame);
    av_packet_free(&packet);
    swr_free(&swr);
    avcodec_free_context(&cctx);
    avformat_close_input(&fmt);

    const size_t frames = mPcm.size() / OUT_CHANNELS;
    LOGI("ringtone: decoded %s -> %zu frames @%uHz", path.c_str(), frames, mSampleRate);
    return frames > 0;
}

bool RingtoneBackend::play() {
    if (mPcm.empty()) return false;
    std::lock_guard<std::mutex> lock(mStreamMutex);
    if (!mAudio) mAudio = std::make_shared<RtAudio>();
    if (!mAudio->isStreamOpen()) {
        RtAudio::StreamParameters params;
        params.deviceId = mAudio->getDefaultOutputDevice();
        params.nChannels = OUT_CHANNELS;
        params.firstChannel = 0;
        unsigned int bufferFrames = 512;
        mAudio->openStream(&params, nullptr, RTAUDIO_SINT16, mSampleRate,
                &bufferFrames, &RingtoneBackend::audioCallback, this);
    }
    if (mAudio->isStreamOpen()) {
        mCursor = 0;
        if (!mAudio->isStreamRunning()) mAudio->startStream();
        mPlaying = true;
        return true;
    }
    LOGE("ringtone: RtAudio open failed (no output device?)");
    return false;
}

void RingtoneBackend::stop() {
    mPlaying = false;
    std::lock_guard<std::mutex> lock(mStreamMutex);
    if (mAudio) {
        if (mAudio->isStreamRunning()) mAudio->abortStream();
        if (mAudio->isStreamOpen()) mAudio->closeStream();
    }
}

int32_t RingtoneBackend::audioCallback(void* outputBuffer, void* /*inputBuffer*/,
        unsigned int nBufferFrames, double /*streamTime*/, uint32_t status, void* userData) {
    auto* self = static_cast<RingtoneBackend*>(userData);
    auto* out = static_cast<int16_t*>(outputBuffer);
    const size_t totalFrames = self->mPcm.size() / OUT_CHANNELS;
    if (status) std::fill(out, out + nBufferFrames * OUT_CHANNELS, int16_t(0));
    if (!self->mPlaying || totalFrames == 0) {
        std::fill(out, out + nBufferFrames * OUT_CHANNELS, int16_t(0));
        return 0;
    }
    // Loop at EOF (AOSP: MediaPlayer.isLooping / Ringtone.setLooping(true)) and
    // apply the crescendo's scalar volume on the way out.
    const float volume = self->mVolume.load();
    for (unsigned int f = 0; f < nBufferFrames; f++) {
        if (self->mCursor >= totalFrames) self->mCursor = 0;   // alarm loops forever
        const int16_t* src = &self->mPcm[self->mCursor * OUT_CHANNELS];
        for (int c = 0; c < OUT_CHANNELS; c++) {
            out[f * OUT_CHANNELS + c] = (int16_t)std::min(32767.0f,
                    std::max(-32768.0f, src[c] * volume));
        }
        self->mCursor++;
    }
    return 0;
}

} // namespace deskclock
} // namespace cdroid

#endif // DESKCLOCK_ENABLE_FFMPEG
