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
#ifndef __CLIP_DRAWABLE_H__
#define __CLIP_DRAWABLE_H__
#include <drawable/drawablewrapper.h>
namespace cdroid{

class ClipDrawable:public DrawableWrapper{
private:
    static constexpr int MAX_LEVEL = 10000;
public:
    static constexpr int HORIZONTAL = 1;
    static constexpr int VERTICAL = 2;
private:
    class ClipState:public DrawableWrapperState{
    public:
        int mGravity;
        int mOrientation;
        ClipState();
        ClipState(const ClipState& state);
        ClipDrawable*newDrawable()override;
    };
    std::shared_ptr<ClipState>mState;
    ClipDrawable(std::shared_ptr<ClipState>state);
    void updateStateFromTypedArray(const AttributeSet&atts);
protected:
    bool onLevelChange(int level)override;
    std::shared_ptr<DrawableWrapperState> mutateConstantState()override;
public:
    ClipDrawable();
    ClipDrawable(Drawable* drawable, int gravity,int orientation);
    int getOpacity()override;
    int getGravity()const;
    int getOrientation()const;
    void draw(Canvas& canvas)override;
    void inflate(XmlPullParser&,const AttributeSet&)override;
};

}
#endif
