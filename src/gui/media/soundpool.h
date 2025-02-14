#ifndef __SOUND_POOL_H__
#define __SOUND_POOL_H__
#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <unordered_map>
class RtAudio;
namespace cdroid{
class SoundPool {
private:
    struct Sound {
        std::vector<char> data;
        int32_t format;
        int32_t channels;
        int32_t sampleRate;
        int32_t byteRate;
        int16_t blockAlign;
        int16_t bitsPerSample;
        uint32_t position;
        float volume;
        bool playing;
    };
    class RtAudio* audio;
    std::unordered_map<int, Sound> sounds;
    int32_t readChunk(std::istream&,Sound&s);
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
