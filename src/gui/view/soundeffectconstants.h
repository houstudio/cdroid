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
#ifndef __SOUND_EFFECT_CONSTANTS_H__
#define __SOUND_EFFECT_CONSTANTS_H__
namespace cdroid{
class SoundEffectConstants {
private:
    static int sLastNavigationRepeatSoundEffectId;
    SoundEffectConstants() {}
public:
    static constexpr int CLICK = 0;
    static constexpr int NAVIGATION_LEFT = 1;
    static constexpr int NAVIGATION_UP = 2;
    static constexpr int NAVIGATION_RIGHT = 3;
    static constexpr int NAVIGATION_DOWN = 4;
    static constexpr int NAVIGATION_REPEAT_LEFT = 5;
    static constexpr int NAVIGATION_REPEAT_UP = 6;
    static constexpr int NAVIGATION_REPEAT_RIGHT = 7;
    static constexpr int NAVIGATION_REPEAT_DOWN = 8;

    static int getContantForFocusDirection(int direction);
    static int getConstantForFocusDirection(int direction, bool repeating);
    static bool isNavigationRepeat(int effectId);
    static int nextNavigationRepeatSoundEffectId();
};
}//namespace
#endif

