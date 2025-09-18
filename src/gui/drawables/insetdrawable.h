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
#ifndef __INSET_DRAWABLE_H__
#define __INSET_DRAWABLE_H__
#include <drawables/drawablewrapper.h>

namespace cdroid{

class InsetDrawable:public DrawableWrapper{
private:
    class InsetValue{
    public:
        float mFraction;
        int mDimension;
        int getDimension(int boundSize)const;
        void set(float,int);
        void set(float);
    };
    class InsetState:public DrawableWrapper::DrawableWrapperState{
    private:
        void applyDensityScaling(int sourceDensity, int targetDensity);
    public:
        InsetValue mInsetLeft;
        InsetValue mInsetTop;
        InsetValue mInsetRight;
        InsetValue mInsetBottom;
        Insets mInset;
        InsetState();
        InsetState(const InsetState& orig);
        void onDensityChanged(int sourceDensity, int targetDensity)override;
        InsetDrawable*newDrawable()override;
    };
    std::shared_ptr<InsetState>mState;
    InsetDrawable(std::shared_ptr<InsetState>state);
    void getInsets(Rect& out);
    void verifyRequiredAttributes();
    void updateStateFromTypedArray(const AttributeSet&atts);
protected:
    void onBoundsChange(const Rect&)override;
    std::shared_ptr<DrawableWrapperState> mutateConstantState()override;
public:
    InsetDrawable();
    InsetDrawable(Drawable*drawable,int inset);
    InsetDrawable(Drawable* drawable,int insetLeft,int insetTop,int insetRight,int insetBottom);
    std::shared_ptr<ConstantState>getConstantState()override;
    int getIntrinsicWidth() override;
    int getIntrinsicHeight() override;
    void getOutline(Outline&) override;
    bool getPadding(Rect& padding)override;
    int getOpacity()override;
    Insets getOpticalInsets()override;
    void inflate(XmlPullParser&parser,const AttributeSet&atts)override;
};

}//namespace

#endif
