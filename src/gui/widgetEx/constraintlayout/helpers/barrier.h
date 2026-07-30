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
 * Ported to C++ for CDROID from androidx.constraintlayout.widget.Barrier.
 *
 * A Barrier references multiple widgets and aligns to the most extreme one on the chosen side
 * (left/right/top/bottom/start/end). This is the widget-layer View; it owns a clcore::Barrier that
 * does the actual solver positioning. The view itself is GONE and zero-sized.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_WIDGET_BARRIER_H
#define CDROID_CONSTRAINTLAYOUT_WIDGET_BARRIER_H

#include <widgetEx/constraintlayout/helpers/constrainthelper.h>
#include <widgetEx/constraintlayout/core/widgets/barrier.h>

namespace cdroid {

class Barrier : public ConstraintHelper {
  public:
    // Direction constants (mirrors clcore::Barrier + START/END for RTL resolution).
    static constexpr int LEFT   = clcore::Barrier::LEFT;
    static constexpr int TOP    = clcore::Barrier::TOP;
    static constexpr int RIGHT  = clcore::Barrier::RIGHT;
    static constexpr int BOTTOM = clcore::Barrier::BOTTOM;
    static constexpr int START  = BOTTOM + 2;
    static constexpr int END    = START + 1;

    Barrier(Context* ctx, const AttributeSet& attrs);
    explicit Barrier(int width, int height);

    int  getType() const;
    void setType(int type);

    bool getAllowsGoneWidget() const;
    void setAllowsGoneWidget(bool supportGone);
    int  getMargin() const;
    void setMargin(int margin);
    void setDpMargin(int margin);

    void resolveRtl(ConstraintWidget* widget, bool isRtl) override;

  private:
    void updateType(ConstraintWidget* widget, int type, bool isRtl);

    int mIndicatedType = LEFT;
    int mResolvedType  = LEFT;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_WIDGET_BARRIER_H
