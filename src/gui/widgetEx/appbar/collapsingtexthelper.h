/*********************************************************************************
 * Copyright (C) [2026] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *********************************************************************************/
#ifndef __COLLAPSING_TEXT_HELPER_H__
#define __COLLAPSING_TEXT_HELPER_H__
#include <core/rect.h>
#include <text/textpaint.h>
#include <string>

namespace cdroid {
class Canvas;
class View;

class CollapsingTextHelper {
private:
    View* mView;
    TextPaint mTextPaint;
    std::u16string mText;
    Rect mExpandedBounds;
    Rect mCollapsedBounds;
    float mExpandedTextSize = 20.0f;
    float mCollapsedTextSize = 16.0f;
    float mExpandedFraction = 0.0f;
    float mCurrentTextSize = 20.0f;
    float mCurrentX = 0.0f;
    float mCurrentY = 0.0f;
    int mExpandedTextColor = 0xFFFFFFFF;
    int mCollapsedTextColor = 0xFFFFFFFF;
    bool mDrawEnabled = true;
    void recalculate();
public:
    explicit CollapsingTextHelper(View* view);
    void setText(const std::u16string& text);
    const std::u16string& getText() const;
    void setExpandedBounds(int left, int top, int right, int bottom);
    void setCollapsedBounds(int left, int top, int right, int bottom);
    void setExpandedTextSize(float size);
    void setCollapsedTextSize(float size);
    void setExpandedTextColor(int color);
    void setCollapsedTextColor(int color);
    void setExpandedFraction(float fraction);
    float getExpandedFraction() const;
    void setDrawEnabled(bool enabled);
    void draw(Canvas& canvas);
};
}
#endif
