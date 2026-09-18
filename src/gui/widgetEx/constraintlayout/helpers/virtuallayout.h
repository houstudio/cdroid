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
 * Ported to C++ for CDROID from androidx.constraintlayout.widget.VirtualLayout.
 *
 * View-layer base for helper layouts that own a core VirtualLayout (Flow). The BasicMeasure
 * Measurer routes these children through onMeasure(VirtualLayout*, ...) — the measurement
 * entry ConstraintLayout.java:853-860 prefers over View::measure for virtual layouts.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_WIDGET_VIRTUAL_LAYOUT_H
#define CDROID_CONSTRAINTLAYOUT_WIDGET_VIRTUAL_LAYOUT_H

#include <widgetEx/constraintlayout/helpers/constrainthelper.h>
#include <widgetEx/constraintlayout/core/widgets/virtuallayout.h>

namespace cdroid {

class ConstraintLayout;

class VirtualLayout : public ConstraintHelper {
  public:
    VirtualLayout(Context* ctx);
    VirtualLayout(Context* ctx, const AttributeSet* attrs);
    VirtualLayout(Context* ctx, const AttributeSet* attrs, int defStyleAttr);

    // Called to measure the layout (VirtualLayout.java:71-75; Flow overrides). The specs are
    // View MeasureSpecs — the core VirtualLayout consumes the same mode encoding.
    virtual void onMeasure(clcore::VirtualLayout* layout,
                           int widthMeasureSpec, int heightMeasureSpec);
    // The 3-arg overload would hide the View onMeasure(int,int) — keep both visible.
    using ConstraintHelper::onMeasure;

  protected:
    void init(const AttributeSet* attrs) override;
    void onAttachedToWindow() override;
    void setVisibility(int visibility) override;
    void setElevation(float elevation) override;

  private:
    bool mApplyVisibilityOnAttach = false;
    bool mApplyElevationOnAttach = false;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_WIDGET_VIRTUAL_LAYOUT_H
