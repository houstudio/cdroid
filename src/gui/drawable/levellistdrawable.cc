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
#include <widget/internal_R.h>
#include <drawable/levellistdrawable.h>
#include <core/typedarray.h>
#include <widget/framework_styleable.h>
#include <cdlog.h>

namespace cdroid{
using namespace cdroid::internal;

LevelListDrawable::LevelListState::LevelListState(const LevelListState*orig,LevelListDrawable*own)
    :DrawableContainerState(orig,own){
    if(orig!=nullptr){
        mLows = orig->mLows;
        mHighs= orig->mHighs;
    }
}

void LevelListDrawable::LevelListState::mutate(){
    // AOSP runs super.mutate() (mutates every child) before cloning the arrays;
    // an empty override suppressed the base chain entirely.
    DrawableContainerState::mutate();
    //mLows = mLows.clone();
    //mHighs = mHighs.clone();
}

void LevelListDrawable::LevelListState::addLevel(int low,int high,Drawable*drawable){
    const int pos = addChild(drawable);
    // Keep the parallel arrays in lockstep with addChild's dedupe (see
    // StateListState::addStateSet).
    if (pos == (int)mLows.size()) { mLows.push_back(low); mHighs.push_back(high); }
    else { mLows[pos] = low; mHighs[pos] = high; }
}

int LevelListDrawable::LevelListState::indexOfLevel(int level)const{
    const int N = getChildCount();
    for (int i = 0; i < N; i++) {
       if (level >= mLows[i] && level <= mHighs[i]) {
           return i;
       }
    }
    return -1;
}

LevelListDrawable*LevelListDrawable::LevelListState::newDrawable(){
    return new LevelListDrawable(std::dynamic_pointer_cast<LevelListState>(shared_from_this()));
}

LevelListDrawable::LevelListDrawable():DrawableContainer(){
    mMutated = false;
    auto state = std::make_shared<LevelListState>(nullptr,this);
    setConstantState(state);
    onLevelChange(getLevel());
}

LevelListDrawable::LevelListDrawable(std::shared_ptr<LevelListState>state){
    auto newState = std::make_shared<LevelListState>(state.get(),this);
    mMutated = false;
    setConstantState(newState);
    onLevelChange(getLevel());
}

bool LevelListDrawable::onLevelChange(int level){
    const int idx = mLevelListState->indexOfLevel(level);
    LOGV("%p level %d.index=%d",this,level,idx);
    if (selectDrawable(idx)) {
        return true;
    }
    return DrawableContainer::onLevelChange(level);
}

std::shared_ptr<DrawableContainer::DrawableContainerState> LevelListDrawable::cloneConstantState(){
    return std::make_shared<LevelListState>(mLevelListState.get(),this);
}

void LevelListDrawable::setConstantState(std::shared_ptr<DrawableContainerState> state){
    DrawableContainer::setConstantState(state);
    mLevelListState = std::dynamic_pointer_cast<LevelListState>(state);
}

LevelListDrawable*LevelListDrawable::mutate(){
    if (!mMutated && (DrawableContainer::mutate() == this)) {
        mLevelListState->mutate();
        mMutated = true;
    }
    return this; 
}

void LevelListDrawable::clearMutated(){
    DrawableContainer::clearMutated();
    mMutated = false;
}

void LevelListDrawable::addLevel(int low,int high,Drawable* drawable) {
    if(drawable){
        mLevelListState->addLevel(low,high,drawable);
        onLevelChange(getLevel());
    }
}
void LevelListDrawable::inflate(Resources& r,XmlPullParser& parser,const AttributeSet& atts, const Resources::Theme* theme){
    DrawableContainer::inflate(r,parser,atts, theme);
    inflateChildElements(r,parser,atts);
}

void LevelListDrawable::inflateChildElements(Resources& r,XmlPullParser& parser,const AttributeSet& atts){
    int type,depth,low = 0;
    const int innerDepth = parser.getDepth()+1;
    while (((type = parser.next()) != XmlPullParser::END_DOCUMENT)
            && ((depth=parser.getDepth()) >= innerDepth || type != XmlPullParser::END_TAG)) {
        if (type != XmlPullParser::START_TAG) {
            continue;
        }

        if ((depth > innerDepth) || parser.getName().compare("item")) {
            continue;
        }
        auto ta = r.obtainStyledAttributes(&atts, R::styleable::LevelListDrawableItem);
        low = ta ? ta->getInt(R::styleable::LevelListDrawableItem_minLevel, 0) : 0;
        int high = ta ? ta->getInt(R::styleable::LevelListDrawableItem_maxLevel, 0) : 0;
        Drawable* dr = ta ? ta->getDrawable(R::styleable::LevelListDrawableItem_drawable) : nullptr;

        if (high < 0) {
            throw std::logic_error(parser.getPositionDescription()+
                ": <item> tag requires a 'maxLevel' attribute");
        }

        if (dr==nullptr) {
            while ((type = parser.next()) == XmlPullParser::TEXT) {}
            if (type != XmlPullParser::START_TAG) {
                throw std::logic_error(parser.getPositionDescription()+
                                ": <item> tag requires a 'drawable' attribute or "
                                "child tag defining a drawable");
            }
            dr = Drawable::createFromXmlInner(r,parser,atts);
        }
        mLevelListState->addLevel(low, high, dr);
    }

    onLevelChange(getLevel());
}

}
