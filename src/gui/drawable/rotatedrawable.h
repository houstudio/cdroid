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
#ifndef __ROTATE_DRAWABLE_H__
#define __ROTATE_DRAWABLE_H__
#include <drawable/drawablewrapper.h>
namespace cdroid{

class RotateDrawable:public DrawableWrapper{
private:
    static constexpr int MAX_LEVEL = 10000;
private:
    class RotateState:public DrawableWrapperState{
    public:
       float mFromDegrees;
       float mToDegrees;
       float mCurrentDegrees;
       float mPivotX;
       float mPivotY;
       bool mPivotXRel;
       bool mPivotYRel;
       RotateState();
       RotateState(const RotateState& orig);
       RotateDrawable*newDrawable()override;
    };
    std::shared_ptr<RotateState>mState;
    RotateDrawable(std::shared_ptr<RotateState>state);
    void updateStateFromTypedArray(const AttributeSet&atts);
protected:
    bool onLevelChange(int level)override;
    std::shared_ptr<DrawableWrapperState> mutateConstantState()override;
public:
    RotateDrawable(Drawable*d=nullptr);
    float getFromDegrees()const;
    float getToDegrees()const;
    void setFromDegrees(float fromDegrees);
    void setToDegrees(float toDegrees);

    float getPivotX()const;
    float getPivotY()const;
    void setPivotX(float pivotX);
    void setPivotY(float pivotX);

    bool isPivotXRelative()const;
    void setPivotXRelative(bool relative);
    bool isPivotYRelative()const;
    void setPivotYRelative(bool relative);
    std::shared_ptr<ConstantState>getConstantState()override;
    void draw(Canvas& canvas)override;
    void inflate(XmlPullParser&,const AttributeSet&atts)override;
};

}
#endif

