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
#ifndef __SHAPE_DRAWABLE_H__
#define __SHAPE_DRAWABLE_H__
#include <drawable/drawable.h>
#include <drawable/shape.h>

namespace cdroid{

class ShapeDrawable:public Drawable{
private:
    class ShapeState:public std::enable_shared_from_this<ShapeState>,public ConstantState{
    public:
        Shape*mShape;
        const ColorStateList* mTint;
        int mTintMode;
        Rect mPadding;
        int mChangingConfigurations;
        int mIntrinsicWidth;
        int mIntrinsicHeight;
        int mAlpha;
        bool mDither;
        ShapeState();
        ShapeState(const ShapeState&orig);
        ~ShapeState();
        ShapeDrawable* newDrawable()override;
        int getChangingConfigurations()const override;
    };
    bool mMutated;
    PorterDuffColorFilter*mTintFilter;
    std::shared_ptr<ShapeState>mShapeState;
    void updateShape();
    ShapeDrawable(std::shared_ptr<ShapeState>state);
    void updateLocalState();
    void updateStateFromTypedArray(const AttributeSet&a);
    int inflateTag(const std::string&,XmlPullParser&,const AttributeSet&);
protected:
    void onBoundsChange(const Rect&bounds)override;
    bool onStateChange(const std::vector<int>&stateset)override;
public:
    ShapeDrawable();
    ~ShapeDrawable();
    void setShape(Shape*shape);
    Shape*getShape()const;
    bool getPadding(Rect&rect)override;
    void setPadding(const Rect& padding);
    void setPadding(int left, int top, int right, int bottom);
    void setAlpha(int alpha)override;
    int getAlpha()const override;
    int getOpacity()override;
    void setTintList(const ColorStateList*)override;
    void setTintMode(int tintMode)override;
    void setColorFilter(ColorFilter*colorFilter)override;
    int getIntrinsicWidth()const;
    int getIntrinsicHeight()const;
    void setIntrinsicWidth(int width);
    void setIntrinsicHeight(int height);
    bool isStateful()const override;
    bool hasFocusStateSpecified()const override;
    void getOutline(Outline&)override;
    std::shared_ptr<ConstantState>getConstantState()override;
    ShapeDrawable*mutate()override;
    void clearMutated()override;
    void draw(Canvas&canvas)override;
    void inflate(XmlPullParser&,const AttributeSet&)override;
};

}
#endif
