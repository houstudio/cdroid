/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#include <media/audiorecord.h>
#include <cstring>

namespace cdroid {

AudioRecord::AudioRecord(AudioSource source,
                         unsigned int sampleRate,
                         ChannelConfig channelConfig,
                         AudioFormat audioFormat,
                         unsigned int bufferFrames)
    : mSampleRate(sampleRate),
      mChannelConfig(channelConfig),
      mAudioFormat(audioFormat),
      mBufferFrames(bufferFrames),
      mIsRecording(false),
      mStopFlag(false)
{
    mInputParams.deviceId = mRtAudio.getDefaultInputDevice();
    mInputParams.nChannels = static_cast<unsigned int>(channelConfig);
    mInputParams.firstChannel = 0;

    unsigned int bytesPerSample = (audioFormat == PCM_16BIT) ? 2 : 1;
    mBuffer.resize(bufferFrames * mInputParams.nChannels * bytesPerSample);
}

AudioRecord::~AudioRecord() {
    stop();
    release();
}

bool AudioRecord::start() {
    if (mIsRecording) return true;
    mStopFlag = false;

    RtAudioFormat fmt = (mAudioFormat == PCM_16BIT) ? RTAUDIO_SINT16 : RTAUDIO_SINT8;

    try {
#if RTAUDIO_VERSION_MAJOR>5
        RtAudio::StreamParameters parameters;
        RtAudioErrorType err = mRtAudio.openStream(nullptr,&parameters, fmt, mSampleRate, &mBufferFrames, &AudioRecord::rtAudioCallback,this);
#else
        mRtAudio.openStream(
            nullptr, // output
            &mInputParams,
            fmt,
            mSampleRate,
            &mBufferFrames,
            &AudioRecord::rtAudioCallback,
            this
        );
#endif
        mIsRecording = true;
    } catch (...) {
        mIsRecording = false;
        return false;
    }
    return true;
}

void AudioRecord::stop() {
    mStopFlag = true;
    if (mIsRecording) {
        try {
            mRtAudio.stopStream();
        } catch (...) {}
        mIsRecording = false;
    }
    mCondVar.notify_all();
}

void AudioRecord::release() {
    if (mRtAudio.isStreamOpen()) {
        try {
            mRtAudio.closeStream();
        } catch (...) {}
    }
}

size_t AudioRecord::read(void* buffer, size_t frames) {
    std::unique_lock<std::mutex> lock(mMutex);
    size_t bytesPerFrame = (mAudioFormat == PCM_16BIT ? 2 : 1) * mChannelConfig;
    size_t bytesToRead = frames * bytesPerFrame;
    while (mIsRecording && mBuffer.size() < bytesToRead) {
        mCondVar.wait(lock);
        if (mStopFlag) return 0;
    }
    size_t copyBytes = std::min(bytesToRead, mBuffer.size());
    std::memcpy(buffer, mBuffer.data(), copyBytes);
    mBuffer.erase(mBuffer.begin(), mBuffer.begin() + copyBytes);
    return copyBytes / bytesPerFrame;
}

void AudioRecord::setRecordCallback(RecordCallback cb) {
    mCallback = cb;
}

bool AudioRecord::isRecording() const {
    return mIsRecording;
}

int AudioRecord::rtAudioCallback(void* outputBuffer, void* inputBuffer,
                                unsigned int nFrames, double /*streamTime*/,
                                RtAudioStreamStatus /*status*/, void* userData) {
    AudioRecord* self = static_cast<AudioRecord*>(userData);
    if (!self->mIsRecording || self->mStopFlag) return 1;
    size_t bytesPerFrame = (self->mAudioFormat == PCM_16BIT ? 2 : 1) * self->mChannelConfig;
    size_t bytes = nFrames * bytesPerFrame;

    if (self->mCallback) {
        self->mCallback(inputBuffer, nFrames);
    } else {
        std::lock_guard<std::mutex> lock(self->mMutex);
        const char* in = static_cast<const char*>(inputBuffer);
        self->mBuffer.insert(self->mBuffer.end(), in, in + bytes);
        self->mCondVar.notify_one();
    }
    return 0;
}

} // namespace cdroid
