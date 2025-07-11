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
#ifndef __AUDIO_MANAGER_H__
#define __AUDIO_MANAGER_H__
#include <core/sparsearray.h>
class RtAudio;
namespace cdroid{
class Context;
class SoundPool;
class AudioManager{
public:
    /* Sound effect identifiers */
    static constexpr int FX_KEY_CLICK = 0;
    static constexpr int FX_FOCUS_NAVIGATION_UP = 1;
    static constexpr int FX_FOCUS_NAVIGATION_DOWN = 2;
    static constexpr int FX_FOCUS_NAVIGATION_LEFT = 3;
    static constexpr int FX_FOCUS_NAVIGATION_RIGHT= 4;
    static constexpr int FX_KEYPRESS_STANDARD = 5;
    static constexpr int FX_KEYPRESS_SPACEBAR = 6;
    static constexpr int FX_KEYPRESS_DELETE = 7;
    static constexpr int FX_KEYPRESS_RETURN = 8;
    static constexpr int FX_KEYPRESS_INVALID= 9;
    static constexpr int FX_BACK= 10;
    static constexpr int FX_HOME= 11;
    static constexpr int NUM_SOUND_EFFECTS = 12;
private:
    Context*mContext;
    std::unique_ptr<SoundPool>mSoundPool;
    std::vector<std::string>SOUND_EFFECT_FILES;
    std::vector<int>SOUND_EFFECT_FILES_MAP;
private:
    AudioManager(Context*);
    void loadTouchSoundAssetDefaults();
public:
    static AudioManager&getInstance();
    void playSoundEffect(int effectType);
    void playSoundEffect(int effectType,int userId);
    void playSoundEffect(int effectType, float volume);
    void loadSoundEffects();
    void unloadSoundEffects();
};

}/*endof namespace*/
#endif/*__AUDIO_MANAGER_H__*/

