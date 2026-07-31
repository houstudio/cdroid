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
#ifndef __NAV_OPTIONS_H__
#define __NAV_OPTIONS_H__
#include <core/bundle.h>
#include <string>
namespace cdroid{
class Window;
class Intent;
typedef Window Activity;
class NavOptions {
private:
    static constexpr const char*const KEY_LAUNCH_MODE = "launchMode";
    static constexpr const char*const KEY_POP_UP_TO = "popUpTo";
    static constexpr const char*const KEY_POP_UP_TO_INCLUSIVE = "popUpToInclusive";
    static constexpr const char*const KEY_ENTER_ANIM = "enterAnim";
    static constexpr const char*const KEY_EXIT_ANIM = "exitAnim";
    static constexpr const char*const KEY_POP_ENTER_ANIM = "popEnterAnim";
    static constexpr const char*const KEY_POP_EXIT_ANIM = "popExitAnim";
public:
    static constexpr int LAUNCH_SINGLE_TOP = 0x1;
    static constexpr int LAUNCH_DOCUMENT = 0x2;
    static constexpr int LAUNCH_CLEAR_TASK = 0x4;
private:
    int mLaunchMode;
    bool mPopUpToInclusive;
    std::string mPopUpTo;            // also serves as popUpToRoute (modern)
    int mPopUpToId = -1;             // popUpTo by destination id (-1 = none); mutually exclusive with mPopUpTo
    std::string mEnterAnim;
    std::string mExitAnim;
    std::string mPopEnterAnim;
    std::string mPopExitAnim;
    bool mShouldRestoreState = false;
    bool mShouldPopUpToSaveState = false;
    Bundle* toBundle();
    static NavOptions* fromBundle(const Bundle& b);
public:
    class Builder;
    static void addPopAnimationsToIntent(Intent& intent, NavOptions* navOptions);
    static void applyPopAnimationsToPendingTransition(Activity& activity);
    NavOptions(int launchMode, const std::string& popUpTo, bool popUpToInclusive,
        const std::string& enterAnim, const std::string& exitAnim,
        const std::string& popEnterAnim, const std::string& popExitAnim,
        int popUpToId = -1);
    NavOptions(int launchMode, const std::string& popUpTo, bool popUpToInclusive,
        const std::string& enterAnim, const std::string& exitAnim,
        const std::string& popEnterAnim, const std::string& popExitAnim,
        bool shouldRestoreState, bool shouldPopUpToSaveState, int popUpToId = -1);
    bool shouldLaunchSingleTop() const;
    bool shouldLaunchDocument() const;
    bool shouldClearTask() const;
    const std::string getPopUpTo() const;
    bool isPopUpToInclusive() const;
    const std::string getEnterAnim() const;
    const std::string getExitAnim() const;
    const std::string getPopEnterAnim() const;
    const std::string getPopExitAnim() const;
    bool shouldRestoreState() const { return mShouldRestoreState; }
    bool shouldPopUpToSaveState() const { return mShouldPopUpToSaveState; }
    const std::string& getPopUpToRoute() const { return mPopUpTo; }
    int getPopUpToId() const { return mPopUpToId; }
};

class NavOptions::Builder {
    int mLaunchMode = 0;
    std::string mPopUpTo;
    int mPopUpToId = -1;
    bool mPopUpToInclusive = false;
    std::string mEnterAnim;
    std::string mExitAnim;
    std::string mPopEnterAnim;
    std::string mPopExitAnim;
    bool mShouldRestoreState = false;
    bool mShouldPopUpToSaveState = false;
public:
    Builder();
    Builder& setLaunchSingleTop(bool singleTop);
    Builder& setLaunchDocument(bool launchDocument);
    Builder& setClearTask(bool clearTask);
    Builder& setPopUpTo(const std::string& destinationId, bool inclusive);
    Builder& setPopUpTo(const std::string& route, bool inclusive, bool saveState);
    Builder& setPopUpTo(int destinationId, bool inclusive);
    Builder& setPopUpTo(int destinationId, bool inclusive, bool saveState);
    Builder& setRestoreState(bool restoreState);
    Builder& setEnterAnim(const std::string& enterAnim);
    Builder& setExitAnim(const std::string& exitAnim);
    Builder& setPopEnterAnim(const std::string& popEnterAnim);
    Builder& setPopExitAnim(const std::string& popExitAnim);
    NavOptions* build();
};

}/*endof namespace*/
#endif/*__NAV_OPTIONS_H__*/
