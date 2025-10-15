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
#include <drawable/levellistdrawable.h>
#include <cdlog.h>

namespace cdroid{

LevelListDrawable::LevelListState::LevelListState(const LevelListState*orig,LevelListDrawable*own)
    :DrawableContainerState(orig,own){
    if(orig!=nullptr){
        mLows = orig->mLows;
        mHighs= orig->mHighs;
    }
}

void LevelListDrawable::LevelListState::mutate(){
    //mLows = mLows.clone();
    //mHighs = mHighs.clone();
}

void LevelListDrawable::LevelListState::addLevel(int low,int high,Drawable*drawable){
    addChild(drawable);
    mLows.push_back(low);
    mHighs.push_back(high);
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
void LevelListDrawable::inflate(XmlPullParser& parser,const AttributeSet& atts){
    DrawableContainer::inflate(parser,atts);
    inflateChildElements(parser,atts);
}

void LevelListDrawable::inflateChildElements(XmlPullParser& parser,const AttributeSet& atts){
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
        low = atts.getInt("minLevel", 0);
        int high = atts.getInt("maxLevel", 0);
        Drawable*dr = atts.getDrawable("drawable");

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
            dr = Drawable::createFromXmlInner(parser,atts);
        }
        mLevelListState->addLevel(low, high, dr);
    }

    onLevelChange(getLevel());
}

}
