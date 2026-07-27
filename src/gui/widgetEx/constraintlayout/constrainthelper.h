/*
 * Copyright (C) 2021 The Android Open Source Project
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

#include <core/attributeset.h>
#include <view/view.h>
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
    void init(const AttributeSet& attrs);

    std::vector<int> mIds;
    std::unique_ptr<HelperWidget> mHelperWidget;
    bool mUseViewMeasure = false;
    std::string mReferenceIds;

  private:
    void addRscID(int id);
    void addID(const std::string& idString);
    void setIds(const std::string& idList);
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_WIDGET_CONSTRAINT_HELPER_H
