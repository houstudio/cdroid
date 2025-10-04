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
#include <drawables/animatedstatelistdrawable.h>
#include <drawables/animatedrotatedrawable.h>
#include <drawables/animatedimagedrawable.h>
#include <drawables/animatedvectordrawable.h>
#include <cdlog.h>

namespace cdroid{

AnimatedStateListDrawable::AnimatedStateListDrawable():StateListDrawable(){
    std::shared_ptr<AnimatedStateListState> newState = std::make_shared<AnimatedStateListState>(nullptr,this);
    setConstantState(newState);
    onStateChange(getState());
    mTransition = nullptr;
    jumpToCurrentState();
}

AnimatedStateListDrawable::AnimatedStateListDrawable(std::shared_ptr<AnimatedStateListDrawable::AnimatedStateListState> state)
  :StateListDrawable(state){
    std::shared_ptr<AnimatedStateListState> newState = std::make_shared<AnimatedStateListState>(state.get(), this);
    mTransition = nullptr;
    setConstantState(newState);
    onStateChange(getState());
    jumpToCurrentState();
}

bool AnimatedStateListDrawable::setVisible(bool visible, bool restart){
    const bool changed = StateListDrawable::setVisible(visible, restart);

    if (mTransition && (changed || restart)) {
        if (visible) {
            mTransition->start();
        } else {
            // Ensure we're showing the correct state when visible.
            jumpToCurrentState();
        }
    }
    return changed;
}

void AnimatedStateListDrawable::addState(const std::vector<int>&stateSet,Drawable* drawable, int id){
    FATAL_IF(drawable==nullptr,"Drawable must not be null");
    mState->addStateSet(stateSet, drawable, id);
    onStateChange(getState());
}

void AnimatedStateListDrawable::addTransition(int fromId, int toId,Drawable* transition, bool reversible){
    FATAL_IF(transition==nullptr,"Transition drawable must not be null");

    mState->addTransition(fromId, toId,(Drawable*)transition, reversible);
}

bool AnimatedStateListDrawable::isStateful()const {
    return true;
}

bool AnimatedStateListDrawable::onStateChange(const std::vector<int>&stateSet){
    const int targetIndex = mState->indexOfKeyframe(stateSet);
    bool changed = targetIndex != getCurrentIndex()
            && (selectTransition(targetIndex) || selectDrawable(targetIndex));

    // We need to propagate the state change to the current drawable, but
    // we can't call StateListDrawable.onStateChange() without changing the
    // current drawable.
    Drawable* current = getCurrent();
    if (current != nullptr) {
        changed |= current->setState(stateSet);
    }

    return changed;
}

bool AnimatedStateListDrawable::selectTransition(int toIndex){
    int fromIndex;
    std::shared_ptr<Transition> currentTransition = mTransition;
    if (currentTransition != nullptr) {
        if (toIndex == mTransitionToIndex) {
            // Already animating to that keyframe.
            return true;
        } else if (toIndex == mTransitionFromIndex && currentTransition->canReverse()) {
            // Reverse the current animation.
            currentTransition->reverse();
            mTransitionToIndex = mTransitionFromIndex;
            mTransitionFromIndex = toIndex;
            return true;
        }

        // Start the next transition from the end of the current one.
        fromIndex = mTransitionToIndex;

        // Changing animation, end the current animation.
        currentTransition->stop();
    } else {
        fromIndex = getCurrentIndex();
    }

    // Reset state.
    mTransition = nullptr;
    mTransitionFromIndex = -1;
    mTransitionToIndex = -1;

    const int fromId = mState->getKeyframeIdAt(fromIndex);
    const int toId = mState->getKeyframeIdAt(toIndex);
    if ((toId == 0) || (fromId == 0)) {
        // Missing a keyframe ID.
        return false;
    }

    const int transitionIndex = mState->indexOfTransition(fromId, toId);
    if (transitionIndex < 0) {
        // Couldn't select a transition.
        return false;
    }

    const bool hasReversibleFlag = mState->transitionHasReversibleFlag(fromId, toId);

    // This may fail if we're already on the transition, but that's okay!
    selectDrawable(transitionIndex);

    std::shared_ptr<Transition> transition = nullptr;
    Drawable* d = getCurrent();
    if (dynamic_cast<AnimationDrawable*>(d)) {
        const bool reversed = mState->isTransitionReversed(fromId, toId);
        transition = std::make_shared<AnimationDrawableTransition>((AnimationDrawable*) d,reversed, hasReversibleFlag);
    } else if (dynamic_cast<AnimatedVectorDrawable*>(d)) {
        const bool reversed = mState->isTransitionReversed(fromId, toId);
        transition = std::make_shared<AnimatedVectorDrawableTransition>((AnimatedVectorDrawable*) d, reversed, hasReversibleFlag);
    } else if (dynamic_cast<Animatable*>(d)) {
        transition = std::make_shared<AnimatableTransition>(d);
    } else {
        // We don't know how to animate this transition.
        return false;
    }

    transition->start();
    mTransition = transition;
    mTransitionFromIndex = fromIndex;
    mTransitionToIndex = toIndex;
    return true;
}

void AnimatedStateListDrawable::jumpToCurrentState(){
    StateListDrawable::jumpToCurrentState();

    if (mTransition != nullptr) {
        mTransition->stop();
        mTransition = nullptr;

        selectDrawable(mTransitionToIndex);
        mTransitionToIndex = -1;
        mTransitionFromIndex = -1;
    }
}

AnimatedStateListDrawable* AnimatedStateListDrawable::mutate() {
    if (!mMutated && StateListDrawable::mutate() == this) {
        mState->mutate();
        mMutated = true;
    }
    return this;
}

void AnimatedStateListDrawable::clearMutated(){
    StateListDrawable::clearMutated();
    mMutated = false;
}

std::shared_ptr<DrawableContainer::DrawableContainerState> AnimatedStateListDrawable::cloneConstantState(){
    return std::make_shared<AnimatedStateListState>(mState.get(), this);
}

void AnimatedStateListDrawable::setConstantState(std::shared_ptr<DrawableContainerState> state){
    StateListDrawable::setConstantState(state);

    if (dynamic_cast<AnimatedStateListState*>(state.get())) {
        mState = std::dynamic_pointer_cast<AnimatedStateListState>(state);
    }
}

void AnimatedStateListDrawable::inflate(XmlPullParser&parser,const AttributeSet&atts){
    StateListDrawable::inflateWithAttributes(parser,atts);

    updateStateFromTypedArray(atts);
    //updateDensity();
    inflateChildElement(parser,atts);
    init();
}

void AnimatedStateListDrawable::updateStateFromTypedArray(const AttributeSet&atts) {
    auto state = mState;

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

void AnimatedStateListDrawable::init(){
    onStateChange(getState());
}

void AnimatedStateListDrawable::inflateChildElement(XmlPullParser&parser,const AttributeSet&atts){
    int type,depth;
    const int innerDepth = parser.getDepth()+1;
    while (((type = parser.next()) != XmlPullParser::END_DOCUMENT)
            && ((depth=parser.getDepth()) >= innerDepth || type != XmlPullParser::END_TAG)) {
        if ( (type != XmlPullParser::START_TAG) || (depth > innerDepth) ) {
            continue;
        }
        const std::string tagName = parser.getName();
        if (tagName.compare(ELEMENT_ITEM)==0) {
            parseItem(parser, atts);
        } else if (tagName.compare(ELEMENT_TRANSITION)==0) {
            parseTransition(parser, atts);
        }
    }
}

int AnimatedStateListDrawable::parseItem(XmlPullParser&parser,const AttributeSet&atts){
    const int keyframeId = atts.getResourceId("id", 0);
    Drawable* dr = atts.getDrawable("drawable");

    std::vector<int> states;
    StateSet::parseState(states,atts);

    // Loading child elements modifies the state of the AttributeSet's
    // underlying parser, so it needs to happen after obtaining
    // attributes and extracting states.
    if (dr == nullptr) {
        int type;
        while ((type = parser.next()) == XmlPullParser::TEXT) {
        }
        if (type != XmlPullParser::START_TAG) {
            throw std::logic_error(parser.getPositionDescription()+
                    ": <item> tag requires a 'drawable' attribute or child tag defining a drawable");
        }
        dr = Drawable::createFromXmlInner(parser,atts);
    }

    return mState->addStateSet(states, dr, keyframeId);
}

int AnimatedStateListDrawable::parseTransition(XmlPullParser&parser,const AttributeSet&atts){
    const int fromId = atts.getResourceId("fromId", 0);
    const int toId = atts.getResourceId("toId", 0);
    const bool reversible = atts.getBoolean("reversible", false);
    Drawable* dr = atts.getDrawable("drawable");

    // Loading child elements modifies the state of the AttributeSet's
    // underlying parser, so it needs to happen after obtaining
    // attributes and extracting states.
    if (dr == nullptr) {
        int type;
        while ((type = parser.next()) == XmlPullParser::TEXT) {
        }
        if (type != XmlPullParser::START_TAG) {
            throw std::logic_error(parser.getPositionDescription()+
                            ": <transition> tag requires a 'drawable' attribute or child tag defining a drawable");
        }
        dr = Drawable::createFromXmlInner(parser, atts);
    }

    return mState->addTransition(fromId, toId, dr, reversible);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
AnimatedStateListDrawable::AnimatedStateListState::AnimatedStateListState(const AnimatedStateListDrawable::AnimatedStateListState* orig,AnimatedStateListDrawable* owner)
  :StateListState(orig,owner){
}

void AnimatedStateListDrawable::AnimatedStateListState::mutate() {
    //mTransitions = mTransitions->clone();
    //mStateIds = mStateIds.clone();
}

int AnimatedStateListDrawable::AnimatedStateListState::addTransition(int fromId, int toId,Drawable* anim, bool reversible){
    const int pos = addChild(anim);
    const int64_t keyFromTo = generateTransitionKey(fromId, toId);
    const int64_t reversibleBit = reversible ? REVERSIBLE_FLAG_BIT : 0;

    mTransitions.put(keyFromTo, pos | reversibleBit);//append

    if (reversible) {
        const int64_t keyToFrom = generateTransitionKey(toId, fromId);
        mTransitions.put(keyToFrom, pos | REVERSED_BIT | reversibleBit);//append
    }
    return pos;
}

int AnimatedStateListDrawable::AnimatedStateListState::addStateSet(std::vector<int> stateSet,Drawable*drawable, int id){
    const int index = StateListDrawable::StateListState::addStateSet(stateSet, drawable);
    mStateIds.put(index, id);
    return index;
}

int AnimatedStateListDrawable::AnimatedStateListState::indexOfKeyframe(const std::vector<int>&stateSet) {
    const int index = StateListDrawable::StateListState::indexOfStateSet(stateSet);
    if (index >= 0) {
        return index;
    }

    return StateListDrawable::StateListState::indexOfStateSet(StateSet::WILD_CARD);
}

int AnimatedStateListDrawable::AnimatedStateListState::getKeyframeIdAt(int index) {
    return index < 0 ? 0 : mStateIds.get(index, 0);
}

int AnimatedStateListDrawable::AnimatedStateListState::indexOfTransition(int fromId, int toId) {
    const int64_t keyFromTo = generateTransitionKey(fromId, toId);
    return (int) mTransitions.get(keyFromTo, -1);
}

bool AnimatedStateListDrawable::AnimatedStateListState::isTransitionReversed(int fromId, int toId) {
    const int64_t keyFromTo = generateTransitionKey(fromId, toId);
    return (mTransitions.get(keyFromTo, -1) & REVERSED_BIT) != 0;
}

bool AnimatedStateListDrawable::AnimatedStateListState::transitionHasReversibleFlag(int fromId, int toId) {
    const int64_t keyFromTo = generateTransitionKey(fromId, toId);
    return (mTransitions.get(keyFromTo, -1) & REVERSIBLE_FLAG_BIT) != 0;
}

//bool canApplyTheme() {return mAnimThemeAttrs != null || super.canApplyTheme();}
AnimatedStateListDrawable* AnimatedStateListDrawable::AnimatedStateListState::newDrawable(){
    return new AnimatedStateListDrawable(std::dynamic_pointer_cast<AnimatedStateListState>(shared_from_this()));
}

int64_t AnimatedStateListDrawable::AnimatedStateListState::generateTransitionKey(int fromId, int toId) {
    return (int64_t) fromId << 32 | toId;
}

//---------------------------------------------------------------------------------------------------------------
AnimatedStateListDrawable::FrameInterpolator::FrameInterpolator(AnimationDrawable* d, bool reversed){
     updateFrames(d, reversed);
}

int  AnimatedStateListDrawable::FrameInterpolator::updateFrames(AnimationDrawable* d, bool reversed){
    int N = d->getNumberOfFrames();
    mFrames = N;

    if (mFrameTimes.size() < N) {
        mFrameTimes.resize(N);
    }

    std::vector<int>&frameTimes = mFrameTimes;
    int totalDuration = 0;
    for (int i = 0; i < N; i++) {
        const int duration = d->getDuration(reversed ? N - i - 1 : i);
        frameTimes[i] = duration;
        totalDuration += duration;
    }

    mTotalDuration = totalDuration;
    return totalDuration;
}

int  AnimatedStateListDrawable::FrameInterpolator::getTotalDuration(){
    return mTotalDuration;
}

float AnimatedStateListDrawable::FrameInterpolator::getInterpolation(float input)const{
    const int elapsed = (int) (input * mTotalDuration + 0.5f);
    const int N = mFrames;
    const std::vector<int>& frameTimes = mFrameTimes;

    // Find the current frame and remaining time within that frame.
    int remaining = elapsed;
    int i = 0;
    while (i < N && remaining >= frameTimes[i]) {
        remaining -= frameTimes[i];
        i++;
    }

    // Remaining time is relative of total duration.
    const float frameElapsed= (i<N)?(remaining / (float) mTotalDuration):.0f;
    return i / (float) N + frameElapsed;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AnimatedStateListDrawable::Transition::reverse(){
    //NOTHING
}

bool AnimatedStateListDrawable::Transition::canReverse(){
    return false;
}

AnimatedStateListDrawable::AnimatableTransition::AnimatableTransition(Drawable* a) {
    mDrawable = dynamic_cast<Drawable*>(a);
}

void AnimatedStateListDrawable::AnimatableTransition::start() {
    if(dynamic_cast<AnimatedRotateDrawable*>(mDrawable)){
        AnimatedRotateDrawable*ard=(AnimatedRotateDrawable*)mDrawable;
        ard->start();
    }else if(dynamic_cast<AnimatedImageDrawable*>(mDrawable)){
        AnimatedImageDrawable*aid=(AnimatedImageDrawable*)mDrawable;
        aid->start();
    }
}

void AnimatedStateListDrawable::AnimatableTransition::stop() {
    if(dynamic_cast<AnimatedRotateDrawable*>(mDrawable)){
        AnimatedRotateDrawable*ard=(AnimatedRotateDrawable*)mDrawable;
        ard->stop();
    }else if(dynamic_cast<AnimatedImageDrawable*>(mDrawable)){
        AnimatedImageDrawable*aid=(AnimatedImageDrawable*)mDrawable;
        aid->stop();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace{
    class PROP_CURRENT_INDEX:public Property{
    public:
        PROP_CURRENT_INDEX():Property("currentIndex"){
        }
        AnimateValue get(void* object){
            AnimateValue v = ((AnimationDrawable*)object)->getCurrentIndex();
            return v;
        }
        void set(void* object,const AnimateValue& value){
            AnimationDrawable*ad=(AnimationDrawable*)object;
            ad->setCurrentIndex(GET_VARIANT(value,int));
        }
    };
}

static const std::shared_ptr<Property>CURRENT_INDEX=std::make_shared<PROP_CURRENT_INDEX>();
AnimatedStateListDrawable::AnimationDrawableTransition::AnimationDrawableTransition(AnimationDrawable* ad, bool reversed, bool hasReversibleFlag){
    const int frameCount = ad->getNumberOfFrames();
    const int fromFrame = reversed ? frameCount - 1 : 0;
    const int toFrame = reversed ? 0 : frameCount - 1;
    mFrameInterpolator = new FrameInterpolator(ad, reversed);
    mProperty = CURRENT_INDEX.get();
    ObjectAnimator* anim = ObjectAnimator::ofInt(ad, mProperty,{fromFrame, toFrame});
    anim->setAutoCancel(true);
    anim->setDuration(mFrameInterpolator->getTotalDuration());
    anim->setInterpolator(mFrameInterpolator);
    mHasReversibleFlag = hasReversibleFlag;
    mAnim = anim;
    mDrawable = ad;
}

AnimatedStateListDrawable::AnimationDrawableTransition::~AnimationDrawableTransition(){
    delete mFrameInterpolator;
	delete mAnim;
	delete mDrawable;
}

bool AnimatedStateListDrawable::AnimationDrawableTransition::canReverse() {
    return mHasReversibleFlag;
}

void AnimatedStateListDrawable::AnimationDrawableTransition::start() {
    mAnim->start();
}

void AnimatedStateListDrawable::AnimationDrawableTransition::reverse() {
    mAnim->reverse();
}

void AnimatedStateListDrawable::AnimationDrawableTransition::stop() {
    mAnim->cancel();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AnimatedStateListDrawable::AnimatedVectorDrawableTransition::AnimatedVectorDrawableTransition(AnimatedVectorDrawable* avd,bool reversed, bool hasReversibleFlag){
    mAvd = avd;
    mDrawable = mAvd;
    mReversed = reversed;
    mHasReversibleFlag = hasReversibleFlag;
}

bool AnimatedStateListDrawable::AnimatedVectorDrawableTransition::canReverse(){
    return mHasReversibleFlag && mAvd->canReverse();
}

void AnimatedStateListDrawable::AnimatedVectorDrawableTransition::start(){
    if (mReversed) {
        reverse();
    } else {
        mAvd->start();
    }
}

void AnimatedStateListDrawable::AnimatedVectorDrawableTransition::reverse(){
    if (canReverse()) {
        mAvd->reverse();
    } else {
        LOGW("Can't reverse, either the reversible is set to false, or the AnimatedVectorDrawable can't reverse");
    }
}

void AnimatedStateListDrawable::AnimatedVectorDrawableTransition::stop(){
    mAvd->stop();
}

}//endof namespace
