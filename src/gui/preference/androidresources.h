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
#ifndef __PREFERENCE_ANDROID_RESOURCES_H__
#define __PREFERENCE_ANDROID_RESOURCES_H__

#include <widget/internal_R.h>

namespace cdroid {

/**
 * Port of androidx.preference.AndroidResources — utility constants for
 * framework resources referenced by the preference library.
 */
class AndroidResources {
public:
    AndroidResources() = delete;

    static constexpr int ANDROID_R_ICON_FRAME = (int)internal::R::id::icon_frame;
    static constexpr int ANDROID_R_LIST_CONTAINER = (int)internal::R::id::list_container;
    static constexpr int ANDROID_R_SWITCH_WIDGET = (int)internal::R::id::switch_widget;
    static constexpr int ANDROID_R_PREFERENCE_FRAGMENT_STYLE = (int)internal::R::attr::preferenceFragmentStyle;
};

} // namespace cdroid

#endif // __PREFERENCE_ANDROID_RESOURCES_H__
