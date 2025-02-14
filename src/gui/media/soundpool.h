#ifndef __SOUND_POOL_H__
#define __SOUND_POOL_H__
#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <core/sparsearray.h>
class RtAudio;
namespace cdroid{
class SoundPool {
private:
    struct Sound;
    struct Channel;
    SparseArray<std::shared_ptr<Channel>>mAudioChannels;
    SparseArray<std::shared_ptr<Sound>>mSounds;
    int32_t readChunk(std::istream&,Sound&s);
    void sendOneSample(SoundPool::Sound&sound,void*outputBuffer,uint32_t i);
    static int32_t audioCallback(void* outputBuffer, void* inputBuffer, unsigned int nBufferFrames,
                    double streamTime, uint32_t status, void* userData);
public:
    SoundPool();
    ~SoundPool();
    int32_t load(const std::string& filePath);
    void play(int soundId);
    void stop(int soundId);
    void setVolume(int soundId, float volume);
};
}/*end namespace*/
#endif/*__SOUND_POOL_H__*/
