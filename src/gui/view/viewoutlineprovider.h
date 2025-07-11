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
#ifndef __VIEW_OUTLINE_PROVIDER_H__
#define __VIEW_OUTLINE_PROVIDER_H__
#include <functional>
namespace cdroid{
    class View;
    class Outline;
    using ViewOutlineProvider = std::function<void(View& view, Outline& outline)>;
    namespace OutlineProvider{
    extern const ViewOutlineProvider BACKGROUND;

    /**
     * Maintains the outline of the View to match its rectangular bounds,
     * at <code>1.0f</code> alpha.
     *
     * This can be used to enable Views that are opaque but lacking a background cast a shadow.
     */
    extern const ViewOutlineProvider BOUNDS;
    /**
     * Maintains the outline of the View to match its rectangular padded bounds,
     * at <code>1.0f</code> alpha.
     *
     * This can be used to enable Views that are opaque but lacking a background cast a shadow.
     */
    extern const ViewOutlineProvider PADDED_BOUNDS;
    }
}/*endof namespace*/
#endif
