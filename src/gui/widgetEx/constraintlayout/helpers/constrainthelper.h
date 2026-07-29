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
 * Ported to C++ for CDROID from androidx.constraintlayout.widget.ConstraintHelper.
 *
 * Base for virtual helpers (Barrier, Group, ...): a View that owns a core HelperWidget and a list
 * of referenced view ids. Multiple helpers can reference the same widgets. References are resolved
 * to ConstraintWidgets in updatePreLayout(), then the core helper participates in the solver pass.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_WIDGET_CONSTRAINT_HELPER_H
#define CDROID_CONSTRAINTLAYOUT_WIDGET_CONSTRAINT_HELPER_H

#include <memory>
#include <string>
#include <vector>

#include <view/view.h>
#include <core/attributeset.h>
#include <widgetEx/constraintlayout/core/widgets/helperwidget.h>

namespace cdroid {

class ConstraintLayout;

class ConstraintHelper : public View {
  public:
    ConstraintHelper(Context* ctx, const AttributeSet& attrs);
    explicit ConstraintHelper(int width, int height);

    // The owned core helper widget (Barrier/...). getViewWidget() returns this for helper children.
    HelperWidget* getHelperWidget() const {
        return mHelperWidget.get();
    }

    // --- referenced ids (programmatic API; XML uses constraint_referenced_ids) ---
    std::vector<int> getReferencedIds() const;
    void setReferencedIds(const std::vector<int>& ids);
    void addView(View* view);
    int  removeView(View* view);
    bool containsId(int id) const;
    int  indexFromId(int id) const;

    void setUseViewMeasure(bool useViewMeasure);

    // Replace the LayoutParams' default widget with this helper's core widget. In CDROID the
    // dispatch is done via getViewWidget() instead, so this is a no-op kept for API parity.
    void validateParams();

    // --- lifecycle hooks (invoked by ConstraintLayout) ---
    virtual void updatePreLayout(ConstraintLayout* container);
    virtual void updatePostLayout(ConstraintLayout* container);
    virtual void updatePostMeasure(ConstraintLayout* container);
    virtual void updatePreDraw(ConstraintLayout* container);
    virtual void resolveRtl(ConstraintWidget* widget, bool isRtl);

    // Propagate this helper's visibility to the referenced views (Group's behavior).
    void applyLayoutFeatures();
    void applyLayoutFeatures(ConstraintLayout* container);

  protected:
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec) override;
    virtual void init(const AttributeSet& attrs);

    std::vector<int> mIds;
    std::unique_ptr<HelperWidget> mHelperWidget;
    bool mUseViewMeasure = false;
    std::string mReferenceIds;
  private:
    void addRscID(int id);
    void addID(int id);
    void setIds(const AttributeSet&atts, const std::string& idList);
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_WIDGET_CONSTRAINT_HELPER_H
