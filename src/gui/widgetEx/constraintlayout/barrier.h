/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.widget.Barrier.
 *
 * A Barrier references multiple widgets and aligns to the most extreme one on the chosen side
 * (left/right/top/bottom/start/end). This is the widget-layer View; it owns a clcore::Barrier that
 * does the actual solver positioning. The view itself is GONE and zero-sized.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_WIDGET_BARRIER_H
#define CDROID_CONSTRAINTLAYOUT_WIDGET_BARRIER_H

#include <widgetEx/constraintlayout/constrainthelper.h>
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
