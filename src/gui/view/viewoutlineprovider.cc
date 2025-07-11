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
#include <view/view.h>
#include <core/outline.h>
#include <view/viewoutlineprovider.h>
namespace cdroid{
namespace OutlineProvider{
    const ViewOutlineProvider BACKGROUND =[](View& view, Outline& outline) {
        Drawable* background = view.getBackground();
        if (background != nullptr) {
            background->getOutline(outline);
        } else {
            outline.setRect(0, 0, view.getWidth(), view.getHeight());
            outline.setAlpha(0.0f);
        }
    };

    /**
     * Maintains the outline of the View to match its rectangular bounds,
     * at <code>1.0f</code> alpha.
     *
     * This can be used to enable Views that are opaque but lacking a background cast a shadow.
     */
    const ViewOutlineProvider BOUNDS = [](View& view, Outline& outline) {
         outline.setRect(0, 0, view.getWidth(), view.getHeight());
    };

    /**
     * Maintains the outline of the View to match its rectangular padded bounds,
     * at <code>1.0f</code> alpha.
     *
     * This can be used to enable Views that are opaque but lacking a background cast a shadow.
     */
    const ViewOutlineProvider PADDED_BOUNDS = [](View& view, Outline& outline) {
          outline.setRect(view.getPaddingLeft(),  view.getPaddingTop(),
                   view.getWidth() - view.getPaddingRight(),
                   view.getHeight() - view.getPaddingBottom());
    };
}
}/*endof namespace*/
