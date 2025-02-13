#ifndef __SOUND_POOL_H__
#define __SOUND_POOL_H__
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
class RtAudio;
namespace cdroid{
class SoundPool {
public:
    SoundPool();
    ~SoundPool();
    int load(const std::string& filePath);
    void play(int soundId);
    void stop(int soundId);
    void setVolume(int soundId, float volume);

private:
    struct Sound {
        std::vector<char> data;
        unsigned int sampleRate;
        unsigned int channels;
        float volume;
    };

    std::unique_ptr<RtAudio> audio;
    std::unordered_map<int, Sound> sounds;
    int nextSoundId;
    static int audioCallback(void* outputBuffer, void* inputBuffer, unsigned int nBufferFrames,
                             double streamTime, uint32_t status, void* userData);
};
}/*end namespace*/
#endif/*__SOUND_POOL_H__*/
