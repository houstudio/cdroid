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
#ifndef __SOUND_POOL_H__
#define __SOUND_POOL_H__
#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <mutex>
#include <unordered_map>
#include <core/sparsearray.h>
namespace cdroid{
class Context;
class SoundPool {
private:
    struct Sound;
    struct Channel;
    struct Stream;
    int32_t mNextStreamId;
    uint32_t mMaxStreams;
    std::recursive_mutex mLock;
    SparseArray<std::shared_ptr<Channel>>mAudioChannels;
    SparseArray<std::shared_ptr<Sound>> mSounds;
    SparseArray<std::shared_ptr<Stream>>mStreams;
    int32_t readChunk(std::istream&,Sound&s);
    void sendOneSample(Channel*channel,void*outputBuffer,uint32_t i);
    static int32_t audioCallback(void* outputBuffer, void* inputBuffer, unsigned int nBufferFrames,
                    double streamTime, uint32_t status, void* userData);
public:
    SoundPool(int maxStreams, int streamType, int srcQuality);
    ~SoundPool();
    int32_t load(const std::string& filePath,int priority);
    int32_t load(Context* context, const std::string& resId, int priority);
    bool unload(int soundID);
    int play(int soundId);
    int play(int soundId,float volume);
    int play(int soundID,float leftVolume, float rightVolume);
    int play(int soundID,float leftVolume, float rightVolume,int priority, int loop, float rate);
    void pause(int streamID);
    void resume(int streamID);
    void stop(int streamId);
    void autoPause();
    void autoResume();
    void setVolume(int soundId, float volume);
    void setVolume(int streamID, float leftVolume, float rightVolume);
    void setPriority(int streamID, int priority);
    void setLoop(int streamID, int loop);
    void setRate(int streamID, float rate);
};
}/*end namespace*/
#endif/*__SOUND_POOL_H__*/
