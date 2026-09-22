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
*/
#ifndef __WEAR_RESOURCES_UTIL_H__
#define __WEAR_RESOURCES_UTIL_H__
#include <core/context.h>
namespace cdroid{

/** Utility methods to help with resource calculations.
 *  androidx.wear.internal.widget.ResourcesUtil.java (lines 29-56). */
class ResourcesUtil {
private:
    ResourcesUtil() = default; // static utility; not instantiable upstream either
public:
    /** Returns the screen width in pixels. */
    static int getScreenWidthPx(Context& context);

    /** Returns the screen height in pixels. */
    static int getScreenHeightPx(Context& context);

    /** Returns the number of pixels equivalent to the percentage of {@code resId} to the
        current screen. */
    static int getFractionOfScreenPx(Context& context, int screenPx, int resId);
};

}/*endof namespace*/
#endif/*__WEAR_RESOURCES_UTIL_H__*/
