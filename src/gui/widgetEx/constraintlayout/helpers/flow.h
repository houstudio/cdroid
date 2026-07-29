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
 * Ported to C++ for CDROID from androidx.constraintlayout.widget.Flow.
 *
 * Arranges referenced widgets in a flowing, wrapping sequence. This is the widget-layer View; it
 * owns a clcore::Flow that does the measure + constraint generation. Configure via the flow_*
 * XML attrs or the setters below; reference children via constraint_referenced_ids.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_WIDGET_FLOW_H
#define CDROID_CONSTRAINTLAYOUT_WIDGET_FLOW_H

#include <widgetEx/constraintlayout/helpers/constrainthelper.h>
#include <widgetEx/constraintlayout/core/widgets/flow.h>

namespace cdroid {

class Flow : public ConstraintHelper {
  public:
    // wrap mode (mirrors clcore::Flow)
    static constexpr int WRAP_NONE      = clcore::Flow::WRAP_NONE;
    static constexpr int WRAP_CHAIN     = clcore::Flow::WRAP_CHAIN;
    static constexpr int WRAP_ALIGNED   = clcore::Flow::WRAP_ALIGNED;
    static constexpr int WRAP_CHAIN_NEW = clcore::Flow::WRAP_CHAIN_NEW;
    // horizontal align
    static constexpr int HORIZONTAL_ALIGN_START  = clcore::Flow::HORIZONTAL_ALIGN_START;
    static constexpr int HORIZONTAL_ALIGN_END    = clcore::Flow::HORIZONTAL_ALIGN_END;
    static constexpr int HORIZONTAL_ALIGN_CENTER = clcore::Flow::HORIZONTAL_ALIGN_CENTER;
    // vertical align
    static constexpr int VERTICAL_ALIGN_TOP      = clcore::Flow::VERTICAL_ALIGN_TOP;
    static constexpr int VERTICAL_ALIGN_BOTTOM   = clcore::Flow::VERTICAL_ALIGN_BOTTOM;
    static constexpr int VERTICAL_ALIGN_CENTER   = clcore::Flow::VERTICAL_ALIGN_CENTER;
    static constexpr int VERTICAL_ALIGN_BASELINE = clcore::Flow::VERTICAL_ALIGN_BASELINE;

    Flow(Context* ctx, const AttributeSet& attrs);
    explicit Flow(int width, int height);

    // configuration (delegate to the core Flow)
    void setWrapMode(int wrapMode);
    void setMaxElementsWrap(int max);
    void setOrientation(int orientation);
    void setHorizontalAlign(int align);
    void setVerticalAlign(int align);
    void setHorizontalGap(int gap);
    void setVerticalGap(int gap);
    void setHorizontalStyle(int style);
    void setVerticalStyle(int style);
    void setHorizontalBias(float bias);
    void setVerticalBias(float bias);
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_WIDGET_FLOW_H
