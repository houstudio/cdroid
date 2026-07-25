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
 * Ported to C++ for CDROID from androidx.constraintlayout.widget.Placeholder.
 *
 * A virtual object that positions an existing content view. When a content id is set, the
 * placeholder takes the content's size and the content view is drawn at the placeholder's frame
 * (treated as gone at its original location). The placeholder itself is otherwise invisible.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_WIDGET_PLACEHOLDER_H
#define CDROID_CONSTRAINTLAYOUT_WIDGET_PLACEHOLDER_H

#include <view/view.h>
#include <core/attributeset.h>

namespace cdroid {

class ConstraintLayout;

class Placeholder : public View {
public:
    Placeholder(Context* ctx, const AttributeSet& attrs);
    explicit Placeholder(int width, int height);

    int  getEmptyVisibility() const;
    void setEmptyVisibility(int visibility);
    View* getContent() const { return mContent; }

    // Set the content view by id (the content is looked up among the placeholder's siblings).
    void setContentId(int id);
    int  getContentId() const { return mContentId; }

    // --- lifecycle hooks (invoked by ConstraintLayout) ---
    void updatePreLayout(ConstraintLayout* container);
    void updatePostMeasure(ConstraintLayout* container);

private:
    void init(const AttributeSet& attrs);

    int   mContentId = -1;
    View* mContent = nullptr;
    int   mEmptyVisibility = View::INVISIBLE;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_WIDGET_PLACEHOLDER_H
