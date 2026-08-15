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
#ifndef __STATELIST_DRAWABLE_H__
#define __STATELIST_DRAWABLE_H__
#include <drawable/drawablecontainer.h>
#include <core/typedarray.h>
namespace cdroid{

class StateListDrawable:public DrawableContainer{
protected:
    class StateListState:public DrawableContainerState{
    public:
        // AOSP mThemeAttrs: ?attr ids captured at inflate, re-resolved by applyTheme.
        std::vector<int> mThemeAttrs;
        std::vector<std::vector<int>>mStateSets;
        StateListState(const StateListState*orig,StateListDrawable*own);
        void mutate()override;
        StateListDrawable*newDrawable()override;
        int addStateSet(const std::vector<int>&stateSet, Drawable*drawable);
        int indexOfStateSet(const std::vector<int>&stateSet);
        bool hasFocusStateSpecified()const;
    };
private:
    std::shared_ptr<StateListState>mStateListState;
    void updateStateFromTypedArray(const TypedArray& a);
protected:
    StateListDrawable(std::shared_ptr<StateListState>state);
    int indexOfStateSet(const std::vector<int>&states)const;
    bool onStateChange(const std::vector<int>&stateSet)override;
    std::shared_ptr<DrawableContainerState>cloneConstantState()override;
    void setConstantState(std::shared_ptr<DrawableContainerState>state)override;
public:
    StateListDrawable();
    StateListDrawable(const ColorStateList&);
    void addState(const std::vector<int>&stateSet,Drawable*drawable);
    bool isStateful()const override{return true;}
    bool hasFocusStateSpecified()const override;
    void inflate(Resources& r,XmlPullParser&,const AttributeSet&atts,const Resources::Theme* theme)override;
    bool canApplyTheme()override;
    void applyTheme(const Resources::Theme& t)override;
    void inflateChildElements(Resources& r,XmlPullParser&parser,const AttributeSet&atts,const Resources::Theme* theme);
    StateListDrawable*mutate()override;
    void clearMutated()override;
    int getStateCount()const;
    const std::vector<int>&getStateSet(int idx)const;
    Drawable*getStateDrawable(int index);
    int getStateDrawableIndex(const std::vector<int>&state)const;
};

}
#endif
