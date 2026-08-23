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
#include <animation/statelistanimator.h>
#include <porting/cdlog.h>
namespace cdroid{

StateListAnimator::Tuple::Tuple(const std::vector<int>&specs, Animator* animator) {
    mSpecs = specs;
    mAnimator = animator;
}
StateListAnimator::Tuple::~Tuple(){
    delete mAnimator;
}

StateListAnimator::StateListAnimator(){
    mView = nullptr;
    mRunningAnimator=nullptr;
    mLastMatch = nullptr;
    initAnimatorListener();
}

void StateListAnimator::initAnimatorListener(){
    mAnimatorListener.onAnimationEnd=[this](Animator& animation,bool){
        animation.setTarget(nullptr);
        if (mRunningAnimator == &animation) {
            mRunningAnimator = nullptr;
        }
    };
}

StateListAnimator::StateListAnimator(const StateListAnimator&other)
  :StateListAnimator(){
    size_t tupleSize = other.mTuples.size();
    for (size_t i = 0; i < tupleSize; i++) {
        Tuple* tuple = other.mTuples.at(i);
        Animator* animatorClone = tuple->mAnimator->clone();
        // AOSP clone(): animatorClone.removeListener(mAnimatorListener) — the
        // cloned child must not carry the SOURCE's state-change listener (it
        // closes over `other`; with a cached source that is a stale capture).
        // CDROID's value-copied listener lists can't be matched by
        // removeListener, and SLA children only ever carry the source's
        // listener (added in addState), so clearing all is equivalent.
        animatorClone->removeAllListeners();
        this->addState(tuple->mSpecs, animatorClone);
    }
    this->setChangingConfigurations(other.getChangingConfigurations());
}

StateListAnimator::~StateListAnimator(){
    for(Tuple*tuple:mTuples){
        delete tuple;
    }
    mTuples.clear();
}

void StateListAnimator::addState(const std::vector<int>&specs, Animator* animator){
    animator->addListener(mAnimatorListener);
    mTuples.push_back(new Tuple(specs, animator));
    mChangingConfigurations |= animator->getChangingConfigurations();
}

void StateListAnimator::setState(const std::vector<int>&state){
    Tuple* match = nullptr;
    const size_t count = mTuples.size();
    for (size_t i = 0; i < count; i++) {
        Tuple* tuple = mTuples.at(i);
        if (StateSet::stateSetMatches(tuple->mSpecs, state)) {
            match = tuple;
            break;
        }
    }
    if (match == mLastMatch) {
        return;
    }
    if (mLastMatch != nullptr) {
        cancel();
    }

    mLastMatch = match;

    if (match != nullptr) {
        start(match);
    }
}

Animator* StateListAnimator::getRunningAnimator() {
    return mRunningAnimator;
}

View* StateListAnimator::getTarget(){
    return mView;
}

void StateListAnimator::setTarget(View* view) {
    View* current = getTarget();
    if (current == view) {
        return;
    }
    if (current != nullptr) {
        clearTarget();
    }
    if (view != nullptr) {
        mView = view;
    }
}

void StateListAnimator::clearTarget() {
    const size_t size = mTuples.size();
    for (size_t i = 0; i < size; i++) {
        mTuples.at(i)->mAnimator->setTarget(nullptr);
    }
    mView = nullptr;
    mLastMatch = nullptr;
    mRunningAnimator = nullptr;
}

void StateListAnimator::start(Tuple* match) {
    match->mAnimator->setTarget(getTarget());
    mRunningAnimator = match->mAnimator;
    mRunningAnimator->start();
}

void StateListAnimator::cancel() {
    if (mRunningAnimator != nullptr) {
        mRunningAnimator->cancel();
        mRunningAnimator = nullptr;
    }
}

void StateListAnimator::jumpToCurrentState(){
    // AOSP: only end() — mRunningAnimator is NOT cleared (getRunningAnimator
    // keeps reporting the jumped-to-end animator until the next setState).
    if (mRunningAnimator != nullptr) {
        mRunningAnimator->end();
    }
}

int StateListAnimator::getChangingConfigurations()const{
    return mChangingConfigurations;
}

void StateListAnimator::setChangingConfigurations(int configs) {
    mChangingConfigurations = configs;
}

void StateListAnimator::appendChangingConfigurations(int configs) {
    mChangingConfigurations |= configs;
}

std::shared_ptr<ConstantState<StateListAnimator*>> StateListAnimator::createConstantState(){
    // Ownership transfer + post-construction back-ref — see Animator::
    // createConstantState (shared_from_this() inside the ctor throws).
    std::shared_ptr<StateListAnimatorConstantState> cs = std::make_shared<StateListAnimatorConstantState>(this);
    mConstantState = cs;
    return cs;
}

/////////////////////////////////////////////////////////////////
StateListAnimator::StateListAnimatorConstantState::StateListAnimatorConstantState(StateListAnimator* animator) {
    mAnimator.reset(animator);
    mChangingConf = mAnimator->getChangingConfigurations();
}

int StateListAnimator::StateListAnimatorConstantState::getChangingConfigurations(){
    return mChangingConf;
}

StateListAnimator* StateListAnimator::StateListAnimatorConstantState::newInstance() {
    StateListAnimator* clone = new StateListAnimator(*mAnimator);
    clone->mConstantState = shared_from_this();
    return clone;
}

}
