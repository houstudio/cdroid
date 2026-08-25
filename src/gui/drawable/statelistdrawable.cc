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
#include <drawable/statelistdrawable.h>
#include <drawable/colordrawable.h>
#include <core/context.h>
#include <core/typedarray.h>
#include <widget/framework_styleable.h>
#include <porting/cdtypes.h>
#include <porting/cdlog.h>
namespace cdroid{
using namespace cdroid::internal;

StateListDrawable::StateListState::StateListState(const StateListState*orig,StateListDrawable*own)
    :DrawableContainerState(orig,own){
    if(orig){
        mStateSets = orig->mStateSets;
    }
}

StateListDrawable*StateListDrawable::StateListState::newDrawable(){
    // AOSP newDrawable() → ctor → createConstantState() → state copy ctor
    // (children re-created from their ConstantStates as futures). Adopting the
    // shared state shared the children across every clone from the drawable
    // cache — nested inside a LayerDrawable layer this leaked one view's
    // bounds/level into every other view of the same resource.
    StateListDrawable* dr = new StateListDrawable();
    dr->setConstantState(std::make_shared<StateListState>(this, dr));
    dr->onStateChange(dr->getState());
    return dr;
}

void StateListDrawable::StateListState::mutate(){
    // AOSP runs super.mutate() (mutates every child) before cloning mStateSets;
    // an empty override suppressed the base chain entirely.
    DrawableContainerState::mutate();
}

int StateListDrawable::StateListState::addStateSet(const std::vector<int>&stateSet, Drawable*drawable){
    const int pos = addChild(drawable);
    // addChild dedupes a re-added Drawable* (returns the existing index) —
    // keep mStateSets in lockstep so later indices stay aligned (AOSP always
    // appends; the unconditional push desynced the parallel arrays).
    if (pos == (int)mStateSets.size()) mStateSets.push_back(stateSet);
    else mStateSets[pos] = stateSet;
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
    return StateSet::containsAttribute(mStateSets,(int)cdroid::internal::R::attr::state_focused);
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
    return StateSet::containsAttribute(mStateListState->mStateSets,(int)cdroid::internal::R::attr::state_focused);
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

// AOSP StateListDrawable.canApplyTheme/applyTheme.
bool StateListDrawable::canApplyTheme(){
    return (mStateListState && !mStateListState->mThemeAttrs.empty()) || Drawable::canApplyTheme();
}

void StateListDrawable::applyTheme(const Resources::Theme& t){
    Drawable::applyTheme(t);
    if (mStateListState && !mStateListState->mThemeAttrs.empty()) {
        auto a = t.resolveAttributes(mStateListState->mThemeAttrs, R::styleable::StateListDrawable);
        if (a) updateStateFromTypedArray(*a);
        mStateListState->mThemeAttrs.clear();
    }
    onStateChange(getState());
}

void StateListDrawable::inflate(Resources&r,XmlPullParser&parser,const AttributeSet&atts,const Resources::Theme* theme){
    (void)r;
    Drawable::inflateWithAttributes(parser,atts);
    Context* ctx = atts.getContext();
    auto ta = obtainAttributes(r, theme, atts, R::styleable::StateListDrawable);
    if (ta) updateStateFromTypedArray(*ta);
    inflateChildElements(r,parser,atts, theme);
    onStateChange(getState());
}

void StateListDrawable::updateStateFromTypedArray(const TypedArray& a) {
    auto state = mStateListState;

    // Account for any configuration changes.
    //state->mChangingConfigurations |= a.getChangingConfigurations();
    // Extract the theme attributes, if any.
    state->mThemeAttrs = a.extractThemeAttrs();

    state->mVariablePadding = a.getBoolean(R::styleable::StateListDrawable_variablePadding, state->mVariablePadding);
    state->mConstantSize = a.getBoolean(R::styleable::StateListDrawable_constantSize, state->mConstantSize);
    state->mEnterFadeDuration = a.getInt(R::styleable::StateListDrawable_enterFadeDuration, state->mEnterFadeDuration);
    state->mExitFadeDuration = a.getInt(R::styleable::StateListDrawable_exitFadeDuration, state->mExitFadeDuration);
    state->mDither = a.getBoolean(R::styleable::StateListDrawable_dither, state->mDither);
    state->mAutoMirrored = a.getBoolean(R::styleable::StateListDrawable_autoMirrored, state->mAutoMirrored);
}

void StateListDrawable::inflateChildElements(Resources&r,XmlPullParser&parser,const AttributeSet&atts,const Resources::Theme* theme){
    int type,depth;
    const int innerDepth = parser.getDepth()+1;
    // AOSP StateListDrawable.inflateChildElements: the loop must stop at the
    // selector's own END_TAG (depth < innerDepth). The old
    // `(next()!=END_DOCUMENT && depth>=innerDepth) || type==END_TAG` grouped
    // as (A && B) || C, so ANY end tag kept it alive: the loop swallowed the
    // enclosing tags and only stopped on the NEXT SIBLING's START_TAG, which
    // the parent loop then never saw (a <selector> followed by a sibling
    // element lost that sibling — e.g. seekbar_track_material dropped its
    // progress layer and inflated with 2 layers).
    while( ((type=parser.next())!=XmlPullParser::END_DOCUMENT)
            &&(((depth=parser.getDepth())>=innerDepth)||(type!=XmlPullParser::END_TAG))){
        if(type!=XmlPullParser::START_TAG)continue;
        if((depth>innerDepth)||parser.getName().compare("item"))continue;

        std::vector<int>states;
        Context* ctx = atts.getContext();
        auto ta = obtainAttributes(r, theme, atts, R::styleable::StateListDrawableItem);
        Drawable*dr = ta ? ta->getDrawable(R::styleable::StateListDrawableItem_drawable) : nullptr;
        StateSet::parseState(states,atts);
        if(dr==nullptr){
            while((type=parser.next())==XmlPullParser::TEXT){}
            if(type!=XmlPullParser::START_TAG)
                throw std::logic_error("<item> tag requires a 'drawable' attribute or child tag defining a drawable");
            dr = Drawable::createFromXmlInner(r,parser,atts, theme);
        }
        mStateListState->addStateSet(states,dr);
    }
}

}
