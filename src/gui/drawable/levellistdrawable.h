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
#ifndef __LEVELLIST_DRAWABLE_H__
#define __LEVELLIST_DRAWABLE_H__
#include <drawable/drawablecontainer.h>
namespace cdroid{
class LevelListDrawable:public DrawableContainer{
private:
    class LevelListState:public DrawableContainerState{
    public:
        std::vector<int>mLows;
        std::vector<int>mHighs;
        LevelListState(const LevelListState*orig,LevelListDrawable*own);
        LevelListState(const LevelListState&state);
        void mutate()override;
        void addLevel(int low,int high,Drawable*drawable);
        int indexOfLevel(int level)const;
        LevelListDrawable*newDrawable()override;
    };
    bool mMutated;
    std::shared_ptr<LevelListState>mLevelListState;
    LevelListDrawable(std::shared_ptr<LevelListState>state);
private:
    void inflateChildElements(XmlPullParser& parser,const AttributeSet& atts);
protected:
    bool onLevelChange(int level)override;
    std::shared_ptr<DrawableContainerState> cloneConstantState()override;
    void setConstantState(std::shared_ptr<DrawableContainerState> state)override;
public:
    LevelListDrawable();
    void addLevel(int low, int high, Drawable* drawable);
    LevelListDrawable* mutate()override;
    void clearMutated()override;
    void inflate(XmlPullParser&parser,const AttributeSet&atts)override;
};
}/*endof namespace*/
#endif

