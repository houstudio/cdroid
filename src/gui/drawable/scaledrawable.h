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
#ifndef __SCALE_DRAWABLE_H__
#define __SCALE_DRAWABLE_H__
#include <drawable/drawablewrapper.h>
namespace cdroid{

class ScaleDrawable:public DrawableWrapper{
private:
    static constexpr int MAX_LEVEL = 10000;
private:
    class ScaleState:public DrawableWrapperState{
    private:
        static constexpr float DO_NOT_SCALE = -1.f;
    public:
        float mScaleWidth;
        float mScaleHeight;
        int mGravity;
        int mInitialLevel;
        bool mUseIntrinsicSizeAsMin;
        ScaleState();
        ScaleState(const ScaleState& orig);
        ScaleDrawable* newDrawable()override;
    };
    std::shared_ptr<ScaleState>mState;
    ScaleDrawable(std::shared_ptr<ScaleState> state);
    void updateStateFromTypedArray(const AttributeSet&atts);
protected:
    void onBoundsChange(const Rect& bounds)override;
    bool onLevelChange(int level)override;
    std::shared_ptr<DrawableWrapperState> mutateConstantState()override;
public:
    ScaleDrawable();
    ScaleDrawable(Drawable* drawable, int gravity,float scaleWidth,float scaleHeight);
    std::shared_ptr<ConstantState>getConstantState()override;
    void draw(Canvas& canvas)override;
    int getOpacity()override;
    int getGravity()const;
    void inflate(XmlPullParser&,const AttributeSet&atts)override;
};

}
#endif

