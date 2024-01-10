#ifndef __AUDIO_MANAGER_H__
#define __AUDIO_MANAGER_H__
namespace cdroid{
class AudioManager{
public:
    void  playSoundEffect(int effectType);
    void  playSoundEffect(int effectType, float volume);
};

}/*endof namespace*/
#endif/*__AUDIO_MANAGER_H__*/

