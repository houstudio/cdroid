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
#ifndef __CDROID_WIDGET_ACCESSIBILITY_ITERATORS_H__
#define __CDROID_WIDGET_ACCESSIBILITY_ITERATORS_H__
#include <view/accessibilityiterators.h>
#include <core/rect.h>

namespace cdroid{

class Layout;
class TextView;

/**
 * Line-for-line port of android.widget.AccessibilityIterators (the widget
 * half of the text-segment iterators: Line and Page, which need Layout and
 * TextView). The view half (Character/Word/Paragraph) lives in
 * view/accessibilityiterators.h, mirroring the AOSP file split.
 */
class LineTextSegmentIterator : public AbstractTextSegmentIterator{
private:
    static LineTextSegmentIterator* sLineInstance;
protected:
    static constexpr int DIRECTION_START = -1;
    static constexpr int DIRECTION_END = 1;

    Layout* mLayout = nullptr;

    LineTextSegmentIterator() = default;
public:
    static LineTextSegmentIterator* getInstance();
    void initialize(const std::u16string& text, Layout* layout);
    int* following(int offset)override;
    int* preceding(int offset)override;
protected:
    int getLineEdgeIndex(int lineNumber, int direction);
};

class PageTextSegmentIterator : public LineTextSegmentIterator{
private:
    static PageTextSegmentIterator* sPageInstance;
    TextView* mView = nullptr;
    Rect mTempRect;

    PageTextSegmentIterator() = default;
public:
    static PageTextSegmentIterator* getInstance();
    void initialize(TextView* view);
    int* following(int offset)override;
    int* preceding(int offset)override;
};

}  // namespace cdroid
#endif  // __CDROID_WIDGET_ACCESSIBILITY_ITERATORS_H__
