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
#include <drawable/statelistdrawable.h>
#include <drawable/colordrawable.h>
#include <porting/cdtypes.h>
#include <porting/cdlog.h>
namespace cdroid{

StateListDrawable::StateListState::StateListState(const StateListState*orig,StateListDrawable*own)
    :DrawableContainerState(orig,own){
    if(orig){
        mStateSets = orig->mStateSets;
    }
}

StateListDrawable*StateListDrawable::StateListState::newDrawable(){
    return new StateListDrawable(std::dynamic_pointer_cast<StateListState>(shared_from_this()));
}

void StateListDrawable::StateListState::mutate(){
}

int StateListDrawable::StateListState::addStateSet(const std::vector<int>&stateSet, Drawable*drawable){
    const int pos = addChild(drawable);
    mStateSets.push_back(stateSet);
    return pos;
}

int StateListDrawable::StateListState::indexOfStateSet(const std::vector<int>&stateSet){
    const int N = getChildCount();
    for (int i = 0; i < N; i++) {
        if (StateSet::stateSetMatches(mStateSets[i], stateSet)) {
            return i;
        }
    }
    return -1;
}

bool StateListDrawable::StateListState::hasFocusStateSpecified()const{
    return StateSet::containsAttribute(mStateSets,StateSet::FOCUSED);
}

StateListDrawable::StateListDrawable(){
    auto state = std::make_shared<StateListState>(nullptr,this);
    setConstantState(state);
}

StateListDrawable::StateListDrawable(const ColorStateList&cls){
    auto state = std::make_shared<StateListState>(nullptr,this);
    setConstantState(state);
    const std::vector<int>&colors = cls.getColors();
    const std::vector<std::vector<int>>& states = cls.getStates();
    for(int i=0;i<states.size();i++){
        addState(states[i],new ColorDrawable(colors[i]));
    }
}

StateListDrawable::StateListDrawable(std::shared_ptr<StateListState>state){
    std::shared_ptr<StateListState>newState = std::make_shared<StateListState>(state.get(), this);
    setConstantState(newState);
    onStateChange(getState());
}

std::shared_ptr<DrawableContainer::DrawableContainerState>StateListDrawable::cloneConstantState(){
    return std::make_shared<StateListState>(mStateListState.get(),this);
}

StateListDrawable*StateListDrawable::mutate(){
    if (!mMutated && DrawableContainer::mutate() == this) {
        mStateListState->mutate();
        mMutated = true;
    }
    return this;
}

void StateListDrawable::clearMutated(){
    DrawableContainer::clearMutated();
    mMutated = false;
}

void StateListDrawable::setConstantState(std::shared_ptr<DrawableContainerState>state){
    DrawableContainer::setConstantState(state);
    mStateListState = std::dynamic_pointer_cast<StateListState>(state);
}

int StateListDrawable::indexOfStateSet(const std::vector<int>&stateSet)const{
    for (int i = 0; i < mStateListState->mStateSets.size(); i++) {
        if (StateSet::stateSetMatches(mStateListState->mStateSets[i], stateSet)) {
            return i;
        }
    }
    return -1;
}

void StateListDrawable::addState(const std::vector<int>&stateSet, Drawable* drawable){
    if(drawable){
        mStateListState->addStateSet(stateSet,drawable);
        onStateChange(getState());
    }
}

bool StateListDrawable::hasFocusStateSpecified()const{
    return StateSet::containsAttribute(mStateListState->mStateSets,StateSet::FOCUSED);
}

int StateListDrawable::getStateCount()const{
    return getChildCount();
}

const std::vector<int>& StateListDrawable::getStateSet(int idx)const{
    return mStateListState->mStateSets[idx];
}

Drawable*StateListDrawable::getStateDrawable(int index){
    return getChild(index);
}

int StateListDrawable::getStateDrawableIndex(const std::vector<int>&stateSet)const{
    return indexOfStateSet(stateSet); 
}

bool StateListDrawable::onStateChange(const std::vector<int>&stateSet){
    const bool changed = DrawableContainer::onStateChange(stateSet);
    int  idx = mStateListState->indexOfStateSet(stateSet);
    if(idx<0)idx = mStateListState->indexOfStateSet(StateSet::WILD_CARD);
    LOGV("%p set stateIndex[%d/%d]=%p",this,idx,getChildCount(),getChild(idx));
    return selectDrawable(idx)||changed;
}

void StateListDrawable::inflate(XmlPullParser&parser,const AttributeSet&atts){
    Drawable::inflateWithAttributes(parser,atts);
    updateStateFromTypedArray(atts);
    inflateChildElements(parser,atts);
    onStateChange(getState());
}

void StateListDrawable::updateStateFromTypedArray(const AttributeSet&atts) {
    auto state = mStateListState;

    // Account for any configuration changes.
    //state->mChangingConfigurations |= a.getChangingConfigurations();
    // Extract the theme attributes, if any.
    //state->mThemeAttrs = a.extractThemeAttrs();

    state->mVariablePadding = atts.getBoolean("variablePadding", state->mVariablePadding);
    state->mConstantSize = atts.getBoolean("constantSize", state->mConstantSize);
    state->mEnterFadeDuration = atts.getInt("enterFadeDuration", state->mEnterFadeDuration);
    state->mExitFadeDuration = atts.getInt("exitFadeDuration", state->mExitFadeDuration);
    state->mDither = atts.getBoolean("dither", state->mDither);
    state->mAutoMirrored = atts.getBoolean("autoMirrored", state->mAutoMirrored);
}

void StateListDrawable::inflateChildElements(XmlPullParser&parser,const AttributeSet&atts){
    int type,depth;
    const int innerDepth = parser.getDepth()+1;
    while( ((type=parser.next())!=XmlPullParser::END_DOCUMENT)
            &&((depth=parser.getDepth())>=innerDepth)||(type==XmlPullParser::END_TAG)){
        if(type!=XmlPullParser::START_TAG)continue;
        if((depth>innerDepth)||parser.getName().compare("item"))continue;

        std::vector<int>states;
        Drawable*dr = atts.getDrawable("drawable");
        StateSet::parseState(states,atts);
        if(dr==nullptr){
            while((type=parser.next())==XmlPullParser::TEXT){}
            if(type!=XmlPullParser::START_TAG)
                throw std::logic_error("<item> tag requires a 'drawable' attribute or child tag defining a drawable");
            dr = Drawable::createFromXmlInner(parser,atts);
        }
        mStateListState->addStateSet(states,dr);
    }
}

}
