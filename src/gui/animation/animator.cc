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
#include <animation/animator.h>
#include <algorithm>
#include <stdexcept>

namespace cdroid{
Animator::~Animator(){
}

Animator*Animator::clone()const{
    return nullptr;
}

void Animator::start() {
}

void Animator::cancel() {
}

void Animator::end() {
}

void Animator::pause() {
    if (isStarted() && !mPaused) {
        mPaused = true;
        for (auto l:mPauseListeners) {
            if(l.onAnimationPause)l.onAnimationPause(*this);
        }
    }
}

void Animator::resume() {
    if (mPaused) {
        mPaused = false;
        for (auto l:mPauseListeners)
            if(l.onAnimationResume)l.onAnimationResume(*this);
    }
}

bool Animator::isPaused() {
   return mPaused;
}

int64_t Animator::getTotalDuration() {
    const int64_t duration = getDuration();
    if (duration == DURATION_INFINITE) {
        return DURATION_INFINITE;
    } else {
        return getStartDelay() + duration;
    }
}

const TimeInterpolator* Animator::getInterpolator()const{
    return nullptr;
}

bool Animator::isStarted() {
    // Default method returns value for isRunning(). Subclasses should override to return a
    // real value.
    return isRunning();
}

std::shared_ptr<Animator::AnimatorConstantState> Animator::createConstantState(){
    return std::make_shared<AnimatorConstantState>(this);
}

void Animator::addListener(const AnimatorListener& listener) {
    mListeners.push_back(listener);
}

void Animator::removeListener(const AnimatorListener& listener){
    auto it = std::find(mListeners.begin(), mListeners.end(),listener);
    if( it!=mListeners.end() ) mListeners.erase(it);
}

std::vector<Animator::AnimatorListener> Animator::getListeners() {
    return mListeners;
}

void Animator::addPauseListener(const AnimatorPauseListener& listener) {
    mPauseListeners.push_back(listener);
}

void Animator::removePauseListener(const AnimatorPauseListener& listener){
    auto it = std::find(mPauseListeners.begin(),mPauseListeners.end(),listener);
    if( it != mPauseListeners.end() ) mPauseListeners.erase(it);
}

void Animator::removeAllListeners() {
    mListeners.clear();
    mPauseListeners.clear();
}

int Animator::getChangingConfigurations() {
    return mChangingConfigurations;
}

void Animator::setChangingConfigurations(int configs) {
    mChangingConfigurations = configs;
}

void Animator::appendChangingConfigurations(int configs) {
    mChangingConfigurations |= configs;
}

void Animator::setupStartValues() {
    //NOTHING
}

void Animator::setupEndValues() {
    //NOTHING
}

void Animator::setTarget(void*target){
    //NOTHING
}

bool Animator::canReverse() {
    return false;
}

void Animator::reverse(){
    throw std::runtime_error("Reverse is not supported");
}

void Animator::setAllowRunningAsynchronously(bool){
    //NOTHING
}

bool Animator::pulseAnimationFrame(int64_t frameTime) {
    // TODO: Need to find a better signal than this. There's a bug in SystemUI that's preventing
    // returning !isStarted() from working.
    return false;
}

void Animator::startWithoutPulsing(bool inReverse) {
    if (inReverse) {
        reverse();
    } else {
        start();
    }
}

void Animator::skipToEndValue(bool inReverse) {
    //NOTHING
}

bool Animator::isInitialized(){
    return true;
}

void Animator::animateBasedOnPlayTime(int64_t currentPlayTime, int64_t lastPlayTime, bool inReverse) {
    //NOTHING
}

AnimatorListenerAdapter::AnimatorListenerAdapter(){
    onAnimationCancel=onAnimationRepeat=onAnimationPause=onAnimationResume=[](Animator&anim){};
    onAnimationEnd=onAnimationStart=[](Animator&aim,bool reverse){};
}

Animator::AnimatorConstantState::AnimatorConstantState(Animator* animator)
    :mAnimator(animator){
    // ensure a reference back to here so that constante state is not gc'ed.
    mAnimator->mConstantState = shared_from_this();
    mChangingConf = mAnimator->getChangingConfigurations();
}

int Animator::AnimatorConstantState::getChangingConfigurations() {
    return mChangingConf;
}

Animator* Animator::AnimatorConstantState::newInstance() {
    Animator* clone = mAnimator->clone();
    clone->mConstantState = shared_from_this();
    return clone;
}

}//endof namespace
