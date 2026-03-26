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
#include <drawable/badgeutils.h>
#include <widget/framelayout.h>
#include <widget/toolbar.h>
#include <gui_features.h>
namespace cdroid{

  /**
   * Updates a badge's bounds using its center coordinate, {@code halfWidth} and {@code halfHeight}.
   *
   * @param rect Holds rectangular coordinates of the badge's bounds.
   * @param centerX A badge's center x coordinate.
   * @param centerY A badge's center y coordinate.
   * @param halfWidth Half of a badge's width.
   * @param halfHeight Half of a badge's height.
   */
void BadgeUtils::updateBadgeBounds(Rect& rect, float centerX, float centerY, float halfWidth, float halfHeight) {
    rect.set( int(centerX - halfWidth), int(centerY - halfHeight),
        int(halfWidth*2.f), int(halfHeight*2.f));
}

void BadgeUtils::attachBadgeDrawable(BadgeDrawable* badgeDrawable, View* anchor) {
    attachBadgeDrawable(badgeDrawable, anchor, /* customBadgeParent */ nullptr);
}

/**
 * Attaches a BadgeDrawable to its associated anchor and update the BadgeDrawable's coordinates
 * based on the anchor. For API 18+, the BadgeDrawable will be added as a view overlay. For
 * pre-API 18, the BadgeDrawable will be set as the foreground of a FrameLayout that is an
 * ancestor of the anchor.
 */
void BadgeUtils::attachBadgeDrawable(BadgeDrawable* badgeDrawable,View* anchor,FrameLayout* customBadgeParent) {
    setBadgeDrawableBounds(badgeDrawable, anchor, customBadgeParent);

    if (badgeDrawable->getCustomBadgeParent() != nullptr) {
        badgeDrawable->getCustomBadgeParent()->setForeground(badgeDrawable);
    } else {
        if (USE_COMPAT_PARENT) {
            FATAL("Trying to reference null customBadgeParent");
        } else {
            anchor->getOverlay()->add(badgeDrawable);
        }
    }
}

  /**
   * A convenience method to attach a BadgeDrawable to the specified menu item on a toolbar, update
   * the BadgeDrawable's coordinates based on its anchor and adjust the BadgeDrawable's offset so it
   * is not clipped off by the toolbar.
   */
void BadgeUtils::attachBadgeDrawable(BadgeDrawable* badgeDrawable, Toolbar* toolbar,int menuItemId) {
    attachBadgeDrawable(badgeDrawable, toolbar, menuItemId, nullptr /*customBadgeParent */);
}

/**
 * Attaches a BadgeDrawable to its associated action menu item on a toolbar, update the
 * BadgeDrawable's coordinates based on this anchor and adjust the BadgeDrawable's offset so it is
 * not clipped off by the toolbar. For API 18+, the BadgeDrawable will be added as a view overlay.
 * For pre-API 18, the BadgeDrawable will be set as the foreground of a FrameLayout that is an
 * ancestor of the anchor.
 */
void BadgeUtils::attachBadgeDrawable(BadgeDrawable* badgeDrawable,
        Toolbar* toolbar,int menuItemId,FrameLayout* customBadgeParent) {
#if ENABLE(MENU)
    toolbar->post([badgeDrawable,toolbar,menuItemId,customBadgeParent](){
            ActionMenuItemView* menuItemView =ToolbarUtils::getActionMenuItemView(toolbar, menuItemId);
            if (menuItemView != nullptr) {
                badgeDrawable->setHorizontalOffset(
                    badgeDrawable->getHorizontalOffset()
                       + toolbar->getContext()
                       ->getDimensionPixelSize("cdroid:dimen/mtrl_badge_toolbar_action_menu_item_horizontal_offset"));
                badgeDrawable->setVerticalOffset(
                    badgeDrawable->getVerticalOffset()
                       + toolbar->getContext()
                       ->getDimensionPixelSize("cdroid:dimen/mtrl_badge_toolbar_action_menu_item_vertical_offset"));
                BadgeUtils::attachBadgeDrawable(badgeDrawable, menuItemView, customBadgeParent);
            }
        });
#endif
}

/**
 * Detaches a BadgeDrawable from its associated anchor. For API 18+, the BadgeDrawable will be
 * removed from its anchor's ViewOverlay. For pre-API 18, the BadgeDrawable will be removed from
 * the foreground of a FrameLayout that is an ancestor of the anchor.
 */
void BadgeUtils::detachBadgeDrawable(BadgeDrawable* badgeDrawable, View* anchor) {
    if (badgeDrawable == nullptr) {
        return;
    }
    if (USE_COMPAT_PARENT || badgeDrawable->getCustomBadgeParent() != nullptr) {
        badgeDrawable->getCustomBadgeParent()->setForeground(nullptr);
    } else {
        anchor->getOverlay()->remove(badgeDrawable);
    }
}

/**
 * Detaches a BadgeDrawable from its associated action menu item on a toolbar, For API 18+, the
 * BadgeDrawable will be removed from its anchor's ViewOverlay. For pre-API 18, the BadgeDrawable
 * will be removed from the foreground of a FrameLayout that is an ancestor of the anchor.
 */
void BadgeUtils::detachBadgeDrawable(BadgeDrawable* badgeDrawable, Toolbar* toolbar, int menuItemId) {
    if (badgeDrawable == nullptr) {
        return;
    }
#if ENABLE(MENU)
    ActionMenuItemView* menuItemView = ToolbarUtils::getActionMenuItemView(toolbar, menuItemId);
    if (menuItemView != nullptr) {
        detachBadgeDrawable(badgeDrawable, menuItemView);
    } else {
        LOGW("Trying to remove badge from a null menuItemView: %d" ,menuItemId);
    }
#endif
}

/**
 * Sets the bounds of a BadgeDrawable to match the bounds of its anchor (for API 18+) or its
 * anchor's FrameLayout ancestor (pre-API 18).
 */
void BadgeUtils::setBadgeDrawableBounds(BadgeDrawable* badgeDrawable, View* anchor,FrameLayout* compatBadgeParent) {
    Rect badgeBounds;
    anchor->getDrawingRect(badgeBounds);
    badgeDrawable->setBounds(badgeBounds);
    badgeDrawable->updateBadgeCoordinates(anchor, compatBadgeParent);
}

}
