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
#ifndef __BADGE_UTILS_H__
#define __BADGE_UTILS_H__
#include <drawable/badgedrawable.h>
namespace cdroid{
class Toolbar;
class BadgeUtils {
private:
    BadgeUtils()=default;
public:
    static constexpr bool USE_COMPAT_PARENT = false;
    static void attachBadgeDrawable(BadgeDrawable* badgeDrawable, View* anchor);
    static void updateBadgeBounds(Rect& rect, float centerX, float centerY, float halfWidth, float halfHeight);
    /**
     * Attaches a BadgeDrawable to its associated anchor and update the BadgeDrawable's coordinates
     * based on the anchor. For API 18+, the BadgeDrawable will be added as a view overlay. For
     * pre-API 18, the BadgeDrawable will be set as the foreground of a FrameLayout that is an
     * ancestor of the anchor.
     */
    static void attachBadgeDrawable(BadgeDrawable* badgeDrawable,
            View* anchor,FrameLayout* customBadgeParent);

    /**
     * A convenience method to attach a BadgeDrawable to the specified menu item on a toolbar, update
     * the BadgeDrawable's coordinates based on its anchor and adjust the BadgeDrawable's offset so it
     * is not clipped off by the toolbar.
     */
    static void attachBadgeDrawable(BadgeDrawable* badgeDrawable, Toolbar* toolbar,int menuItemId);

    /**
     * Attaches a BadgeDrawable to its associated action menu item on a toolbar, update the
     * BadgeDrawable's coordinates based on this anchor and adjust the BadgeDrawable's offset so it is
     * not clipped off by the toolbar. For API 18+, the BadgeDrawable will be added as a view overlay.
     * For pre-API 18, the BadgeDrawable will be set as the foreground of a FrameLayout that is an
     * ancestor of the anchor.
     */
    static void attachBadgeDrawable(BadgeDrawable* badgeDrawable,
          Toolbar* toolbar,int menuItemId,FrameLayout* customBadgeParent);
    /**
     * Detaches a BadgeDrawable from its associated anchor. For API 18+, the BadgeDrawable will be
     * removed from its anchor's ViewOverlay. For pre-API 18, the BadgeDrawable will be removed from
     * the foreground of a FrameLayout that is an ancestor of the anchor.
     */
    static void detachBadgeDrawable(BadgeDrawable* badgeDrawable, View* anchor);

    /**
     * Detaches a BadgeDrawable from its associated action menu item on a toolbar, For API 18+, the
     * BadgeDrawable will be removed from its anchor's ViewOverlay. For pre-API 18, the BadgeDrawable
     * will be removed from the foreground of a FrameLayout that is an ancestor of the anchor.
     */
    static void detachBadgeDrawable(BadgeDrawable* badgeDrawable, Toolbar* toolbar, int menuItemId);

    /**
     * Sets the bounds of a BadgeDrawable to match the bounds of its anchor (for API 18+) or its
     * anchor's FrameLayout ancestor (pre-API 18).
     */
    static void setBadgeDrawableBounds(BadgeDrawable* badgeDrawable, View* anchor,FrameLayout* compatBadgeParent);
};
}/*endof namespace*/
#endif
