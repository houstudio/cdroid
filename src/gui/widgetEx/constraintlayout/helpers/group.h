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

/*
 * Ported to C++ for CDROID from androidx.constraintlayout.widget.Group.
 *
 * Controls the visibility of a set of referenced widgets. Unlike Barrier, Group owns no core
 * helper widget — it propagates its own visibility to the referenced views via applyLayoutFeatures.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_WIDGET_GROUP_H
#define CDROID_CONSTRAINTLAYOUT_WIDGET_GROUP_H

#include <widgetEx/constraintlayout/helpers/constrainthelper.h>

namespace cdroid {

class Group : public ConstraintHelper {
  public:
    Group(Context* ctx, const AttributeSet& attrs);
    explicit Group(int width, int height);

    void setVisibility(int visibility) override;
    void onAttachedToWindow() override;

    void updatePostLayout(ConstraintLayout* container) override;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_WIDGET_GROUP_H
