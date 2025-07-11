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
#ifndef __AUDIO_RECORD_H__
#define __AUDIO_RECORD_H__
#include <vector>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#if ENABLE(AUDIO)
#include <rtaudio/RtAudio.h>
#endif
namespace cdroid {

class AudioRecord {
public:
    enum AudioSource {
        MIC = 0
    };
    enum AudioFormat {
        PCM_16BIT = 16,
        PCM_8BIT = 8
    };
    enum ChannelConfig {
        CHANNEL_IN_MONO = 1,
        CHANNEL_IN_STEREO = 2
    };

    using RecordCallback = std::function<void(const void* data, size_t frames)>;

    AudioRecord(AudioSource source,
                unsigned int sampleRate,
                ChannelConfig channelConfig,
                AudioFormat audioFormat,
                unsigned int bufferFrames);

    ~AudioRecord();

    bool start();
    void stop();
    void release();

    // 阻塞读取
    size_t read(void* buffer, size_t frames);

    // 设置回调（非阻塞模式）
    void setRecordCallback(RecordCallback cb);

    bool isRecording() const;

    unsigned int getSampleRate() const { return mSampleRate; }
    ChannelConfig getChannelConfig() const { return mChannelConfig; }
    AudioFormat getAudioFormat() const { return mAudioFormat; }
    unsigned int getBufferFrames() const { return mBufferFrames; }

private:
    RtAudio mRtAudio;
    RtAudio::StreamParameters mInputParams;
    unsigned int mSampleRate;
    ChannelConfig mChannelConfig;
    AudioFormat mAudioFormat;
    unsigned int mBufferFrames;

    std::vector<char> mBuffer;
    std::mutex mMutex;
    std::condition_variable mCondVar;
    std::atomic<bool> mIsRecording;
    std::atomic<bool> mStopFlag;
    RecordCallback mCallback;

    static int rtAudioCallback(void* outputBuffer, void* inputBuffer,
                              unsigned int nFrames, double streamTime,
                              RtAudioStreamStatus status, void* userData);
};

} // namespace cdroid
#endif/*__AUDIO_RECORD_H__*/
