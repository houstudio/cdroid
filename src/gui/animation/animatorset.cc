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
#include <animation/animatorset.h>
#include <animation/objectanimator.h>
#include <porting/cdlog.h>
#include <algorithm>

namespace cdroid{

AnimatorSet::AnimatorSet():Animator(){
    mDelayAnim = ValueAnimator::ofFloat({0.f, 1.f});
    mDelayAnim->setDuration(0);
    mRootNode  = new Node(mDelayAnim);
    mNodeMap.insert({mDelayAnim, mRootNode});
    mNodes.push_back(mRootNode);
    // Set the flag to ignore calling end() without start() for pre-N releases
    //VERSION_CODES.N=24,VERSION_CODES.O=26,Android9(Pie)=28;
    mShouldIgnoreEndWithoutStart = false;
    const bool isPreO = false;//app.getApplicationInfo().targetSdkVersion < Build.VERSION_CODES.O
    mShouldResetValuesAtStart = !isPreO;
    mEndCanBeCalled = !isPreO;
    mTotalDuration = 0;
    mSeekState = new SeekState(this);
    mDummyListener.onAnimationEnd=[this](Animator&animation,bool isReverse){
        auto it = mNodeMap.find(&animation);
        if(it==mNodeMap.end()){
            throw std::runtime_error("Error: animation ended is not in the node map");
        }
        it->second->mEnded = true;
    };
}

AnimatorSet::AnimatorSet(const AnimatorSet&other){
    const int nodeCount = other.mNodes.size();
    mStarted = false;
    mLastFrameTime = -1;
    mFirstFrame = -1;
    mLastEventId = -1;
    mPaused = false;
    mPauseTime = -1;
    mSeekState = new SeekState(this);
    mSelfPulse = true;
    mStartListenersCalled = false;
    mReversing = false;
    mShouldResetValuesAtStart = other.mShouldResetValuesAtStart;
    mShouldIgnoreEndWithoutStart = other.mShouldIgnoreEndWithoutStart;
    mDependencyDirty = true;

    mDummyListener.onAnimationEnd=[this](Animator&animation,bool isReverse){
        auto it = mNodeMap.find(&animation);
        if(it==mNodeMap.end()){
            throw std::runtime_error("Error: animation ended is not in the node map");
        }
        it->second->mEnded = true;
    };

    // Walk through the old nodes list, cloning each node and adding it to the new nodemap.
    // One problem is that the old node dependencies point to nodes in the old AnimatorSet.
    // We need to track the old/new nodes in order to reconstruct the dependencies in the clone.

    std::unordered_map<Node*,Node*>clonesMap;
    for (Node*node:other.mNodes){
        Node* nodeClone = node->clone();
        // Remove the old internal listener from the cloned child
        nodeClone->mAnimation->removeListener(mDummyListener);
        clonesMap.insert({node, nodeClone});
        mNodes.push_back(nodeClone);
        mNodeMap.insert({nodeClone->mAnimation, nodeClone});
    }

    mRootNode = clonesMap.find(other.mRootNode)->second;
    mDelayAnim = (ValueAnimator*) this->mRootNode->mAnimation;

    // Now that we've cloned all of the nodes, we're ready to walk through their
    // dependencies, mapping the old dependencies to the new nodes
    for (int i = 0; i < nodeCount; i++) {
        Node* node = other.mNodes.at(i);
        // Update dependencies for node's clone
        Node* nodeClone = clonesMap.find(node)->second;
        nodeClone->mLatestParent = node->mLatestParent == nullptr
                ? nullptr : clonesMap.find(node->mLatestParent)->second;
        int size = node->mChildNodes.size();
        bool found=false;
        for (int j = 0; j < size; j++) {
            auto it = clonesMap.find(node->mChildNodes.at(j));
            found= it!=clonesMap.end();
            if(found)nodeClone->mChildNodes.push_back(it->second);
        }
        size = node->mSiblings.size();
        for (int j = 0; j < size; j++) {
            auto it = clonesMap.find(node->mSiblings.at(j));
            found= it!=clonesMap.end();
            if(found)nodeClone->mSiblings.push_back(it->second);
        }
        size = node->mParents.size();
        for (int j = 0; j < size; j++) {
            auto it =clonesMap.find(node->mParents.at(j));
            found= it!=clonesMap.end();
            if(found)nodeClone->mParents.push_back(it->second);
        }
    }
}

AnimatorSet::~AnimatorSet(){
    for(auto nd:mNodeMap){
        delete nd.first;
    }
    for(auto nd:mNodes)
        delete nd;
    for(auto e:mEvents)delete e;
    mNodeMap.clear();
    mNodes.clear();
    delete mSeekState;
}

void AnimatorSet::playTogether(const std::vector<Animator*>&items){
    const int size = (int)items.size();
    if(size){
        Builder builder(this,items[0]);
        for (int i = 1; i < size; ++i) {
            builder.with(items[i]);
        }
    }
}

void AnimatorSet::playSequentially(const std::vector<Animator*>&items){
    const size_t size = items.size();
    if(size==1){
        play(items[0]);
    } else {
        for (int i = 0; i < int(size - 1); ++i) {
            play(items[i])->before(items[i + 1]);
        }
    }
}

std::vector<Animator*> AnimatorSet::getChildAnimations()const{
    std::vector<Animator*> childList;
    size_t size = mNodes.size();
    for (size_t i = 0; i < size; i++) {
        Node* node = mNodes.at(i);
        if (node != mRootNode) {
            childList.push_back(node->mAnimation);
        }
    }
    return childList;
}

void AnimatorSet::setTarget(void*target){
    for (auto node:mNodes){//int i = 0; i < size; i++) {
        Animator* animation = node->mAnimation;
        if (dynamic_cast<AnimatorSet*>(animation)) {
            ((AnimatorSet*)animation)->setTarget(target);
        } else if (dynamic_cast<ObjectAnimator*>(animation)) {
            ((ObjectAnimator*)animation)->setTarget(target);
        }
    }
}

int AnimatorSet::getChangingConfigurations() {
    int conf = Animator::getChangingConfigurations();
    size_t nodeCount = mNodes.size();
    for (int i = 0; i < nodeCount; i ++) {
        conf |= mNodes.at(i)->mAnimation->getChangingConfigurations();
    }
    return conf;
}

void AnimatorSet::setInterpolator(const TimeInterpolator* interpolator) {
    mInterpolator = interpolator;
}

const TimeInterpolator* AnimatorSet::getInterpolator() const{
    return mInterpolator;
}

AnimatorSet::Builder* AnimatorSet::play(Animator* anim){
    if(anim!=nullptr)
        return new Builder(this,anim);
    return nullptr;
}

void AnimatorSet::cancel(){
    if (isStarted()|| mStartListenersCalled) {
        std::vector<AnimatorListener>tmpListener = mListeners;
        for (auto ls:tmpListener) {
            if(ls.onAnimationCancel)ls.onAnimationCancel(*this);
        }
        for (auto node:mPlayingSet){//int i = 0; i < setSize; i++) {
            node->mAnimation->cancel();
        }
        mPlayingSet.clear();
        endAnimation();
    }
}

void AnimatorSet::forceToEnd() {
    if (mEndCanBeCalled) {
        end();
        return;
    }

    // Note: we don't want to combine this case with the end() method below because in
    // the case of developer calling end(), we still need to make sure end() is explicitly
    // called on the child animators to maintain the old behavior.
    if (mReversing) {
        handleAnimationEvents(mLastEventId, 0, getTotalDuration());
    } else {
        int64_t zeroScalePlayTime = getTotalDuration();
        if (zeroScalePlayTime == DURATION_INFINITE) {
            // Use a large number for the play time.
            zeroScalePlayTime = INT_MAX;//Integer.MAX_VALUE;
        }
        handleAnimationEvents(mLastEventId, int(mEvents.size() - 1), zeroScalePlayTime);
    }
    mPlayingSet.clear();
    endAnimation();
}

void AnimatorSet::end() {
    if (mShouldIgnoreEndWithoutStart && !isStarted()) {
        return;
    }
    if (isStarted()) {
        // Iterate the animations that haven't finished or haven't started, and end them.
        if (mReversing) {
            // Between start() and first frame, mLastEventId would be unset (i.e. -1)
            mLastEventId = (mLastEventId == -1 )? int(mEvents.size()) : mLastEventId;
            while (mLastEventId > 0) {
                mLastEventId = mLastEventId - 1;
                AnimationEvent* event = mEvents.at(mLastEventId);
                Animator* anim = event->mNode->mAnimation;
                auto it=mNodeMap.find(anim);
                if (it->second->mEnded) {
                    continue;
                }
                if (event->mEvent == AnimationEvent::ANIMATION_END) {
                    anim->reverse();
                } else if (event->mEvent == AnimationEvent::ANIMATION_DELAY_ENDED
                        && anim->isStarted()) {
                    // Make sure anim hasn't finished before calling end() so that we don't end
                    // already ended animations, which will cause start and end callbacks to be
                    // triggered again.
                    anim->end();
                }
            }
        } else {
            while (mLastEventId < mEvents.size() - 1) {
                // Avoid potential reentrant loop caused by child animators manipulating
                // AnimatorSet's lifecycle (i.e. not a recommended approach).
                mLastEventId = mLastEventId + 1;
                AnimationEvent* event = mEvents.at(mLastEventId);
                Animator* anim = event->mNode->mAnimation;
                auto it = mNodeMap.find(anim);
                if (it->second->mEnded) {
                    continue;
                }
                if (event->mEvent == AnimationEvent::ANIMATION_START) {
                    anim->start();
                } else if ((event->mEvent == AnimationEvent::ANIMATION_END) && anim->isStarted()) {
                    // Make sure anim hasn't finished before calling end() so that we don't end
                    // already ended animations, which will cause start and end callbacks to be
                    // triggered again.
                    anim->end();
                }
            }
        }
        mPlayingSet.clear();
    }
    endAnimation();
}

bool AnimatorSet::isRunning() {
    if (mStartDelay == 0) {
        return mStarted;
    }
    return mLastFrameTime > 0;
}

bool AnimatorSet::isStarted() {
    return mStarted;
}

int64_t AnimatorSet::getStartDelay() {
    return mStartDelay;
}

void AnimatorSet::setStartDelay(int64_t startDelay) {
    // Clamp start delay to non-negative range.
    if (startDelay < 0) {
        LOGW("Start delay should always be non-negative");
        startDelay = 0;
    }
    int64_t delta = startDelay - mStartDelay;
    if (delta == 0) {
        return;
    }
    mStartDelay = startDelay;
    if (!mDependencyDirty) {
        // Dependency graph already constructed, update all the nodes' start/end time
        for (Node*node:mNodes){//int i = 0; i < size; i++) {
            if (node == mRootNode) {
                node->mEndTime = mStartDelay;
            } else {
                node->mStartTime = (node->mStartTime == DURATION_INFINITE) ?
                        DURATION_INFINITE : node->mStartTime + delta;
                node->mEndTime = (node->mEndTime == DURATION_INFINITE) ?
                        DURATION_INFINITE : node->mEndTime + delta;
            }
        }
        // Update total duration, if necessary.
        if (mTotalDuration != DURATION_INFINITE) {
            mTotalDuration += delta;
        }
    }
}

int64_t AnimatorSet::getDuration()const{
    return mDuration;
}

Animator& AnimatorSet::setDuration(int64_t duration) {
    if (duration < 0) {
        std::logic_error("duration must be a value of zero or greater");
    }
    mDependencyDirty = true;
    // Just record the value for now - it will be used later when the AnimatorSet starts
    mDuration = duration;
    return *this;
}

void AnimatorSet::setupStartValues() {
    LOGD("%d nodes",mNodes.size());
    for (Node*node:mNodes) {
        if (node != mRootNode) {
            node->mAnimation->setupStartValues();
        }
    }
}

void AnimatorSet::setupEndValues() {
    for (Node*node:mNodes){
        if (node != mRootNode) {
            node->mAnimation->setupEndValues();
        }
    }
}

void AnimatorSet::pause() {
    const bool previouslyPaused = mPaused;
    Animator::pause();
    if (!previouslyPaused && mPaused) {
        mPauseTime = -1;
    }
}

void AnimatorSet::resume() {
    const bool previouslyPaused = mPaused;
    Animator::resume();
    if (previouslyPaused && !mPaused) {
        if (mPauseTime >= 0) {
            addAnimationCallback(0);
        }
    }
}

void AnimatorSet::start() {
    start(false, true);
}

void AnimatorSet::startWithoutPulsing(bool inReverse) {
    start(inReverse, false);
}

void AnimatorSet::initAnimation() {
    if (mInterpolator != nullptr) {
        for (int i = 0; i < mNodes.size(); i++) {
            Node* node = mNodes.at(i);
            node->mAnimation->setInterpolator(mInterpolator);
        }
    }
    updateAnimatorsDuration();
    createDependencyGraph();
}

void AnimatorSet::start(bool inReverse, bool selfPulse) {
    mStarted = true;
    mSelfPulse = selfPulse;
    mPaused = false;
    mPauseTime = -1;

    for (Node*node:mNodes) {
        node->mEnded = false;
        node->mAnimation->setAllowRunningAsynchronously(false);
    }

    initAnimation();
    if (inReverse && !canReverse()) {
        throw std::runtime_error("Cannot reverse infinite AnimatorSet");
    }

    mReversing = inReverse;

    // Now that all dependencies are set up, start the animations that should be started.
    const bool IsEmptySet = isEmptySet(this);
    if (!IsEmptySet) {
        startAnimation();
    }

    std::vector<AnimatorListener>tmpListeners = mListeners;
    for (AnimatorListener&ls:tmpListeners) {
        if(ls.onAnimationStart)ls.onAnimationStart(*this, inReverse);
    }
    if (IsEmptySet) {
        // In the case of empty AnimatorSet, or 0 duration scale, we will trigger the
        // onAnimationEnd() right away.
        end();
    }
}

bool AnimatorSet::isEmptySet(AnimatorSet* set) {
    if (set->getStartDelay() > 0) {
        return false;
    }
    std::vector<Animator*>childAnimators = set->getChildAnimations();
    for (Animator*anim:childAnimators) {
        if (!(dynamic_cast<AnimatorSet*>(anim))) {
            // Contains non-AnimatorSet, not empty.
            return false;
        } else {
            if (!isEmptySet((AnimatorSet*) anim)) {
                return false;
            }
        }
    }
    return true;
}

void AnimatorSet::updateAnimatorsDuration() {
    if (mDuration >= 0) {
        // If the duration was set on this AnimatorSet, pass it along to all child animations
        for (Node*node:mNodes) {
            // TODO: don't set the duration of the timing-only nodes created by AnimatorSet to
            // insert "play-after" delays
            node->mAnimation->setDuration(mDuration);
        }
    }
    mDelayAnim->setDuration(mStartDelay);
}

void AnimatorSet::skipToEndValue(bool inReverse) {
    if (!isInitialized()) {
        throw std::runtime_error("Children must be initialized.");
    }

    // This makes sure the animation events are sorted an up to date.
    initAnimation();

    // Calling skip to the end in the sequence that they would be called in a forward/reverse
    // run, such that the sequential animations modifying the same property would have
    // the right value in the end.
    if (inReverse) {
        for (int i = int(mEvents.size() - 1); i >= 0; i--) {
            if (mEvents.at(i)->mEvent == AnimationEvent::ANIMATION_DELAY_ENDED) {
                mEvents.at(i)->mNode->mAnimation->skipToEndValue(true);
            }
        }
    } else {
        for (int i = 0; i < mEvents.size(); i++) {
            if (mEvents.at(i)->mEvent == AnimationEvent::ANIMATION_END) {
                mEvents.at(i)->mNode->mAnimation->skipToEndValue(false);
            }
        }
    }
}

void AnimatorSet::animateBasedOnPlayTime(int64_t currentPlayTime, int64_t lastPlayTime, bool inReverse){
    if( (currentPlayTime<0) || (lastPlayTime<0)){
        throw std::logic_error("Error: Play time should never be negative.");
    }
    if (inReverse) {
        if (getTotalDuration() == DURATION_INFINITE) {
            throw std::logic_error("Cannot reverse AnimatorSet with infinite duration");
        }
        int64_t duration = getTotalDuration() - mStartDelay;
        currentPlayTime = std::min(currentPlayTime, duration);
        currentPlayTime = duration - currentPlayTime;
        lastPlayTime = duration - lastPlayTime;
        inReverse = false;
    }
    // Skip all values to start, and iterate mEvents to get animations to the right fraction.
    skipToStartValue(false);

    std::vector<Node*> unfinishedNodes;
    // Assumes forward playing from here on.
    for (AnimationEvent* event:mEvents){
        if (event->getTime() > currentPlayTime) {
            break;
        }

        // This animation started prior to the current play time, and won't finish before the
        // play time, add to the unfinished list.
        if (event->mEvent == AnimationEvent::ANIMATION_DELAY_ENDED) {
            if (event->mNode->mEndTime == DURATION_INFINITE
                    || event->mNode->mEndTime > currentPlayTime) {
                unfinishedNodes.push_back(event->mNode);
            }
        }
        // For animations that do finish before the play time, end them in the sequence that
        // they would in a normal run.
        if (event->mEvent == AnimationEvent::ANIMATION_END) {
            // Skip to the end of the animation.
            event->mNode->mAnimation->skipToEndValue(false);
        }
    }

    // Seek unfinished animation to the right time.
    for (Node*node:unfinishedNodes) {
        int64_t playTime = getPlayTimeForNode(currentPlayTime, node, inReverse);
        if (!inReverse) {
            playTime -= node->mAnimation->getStartDelay();
        }
        node->mAnimation->animateBasedOnPlayTime(playTime, lastPlayTime, inReverse);
    }
}

bool AnimatorSet::isInitialized() {
    if (mChildrenInitialized) {
        return true;
    }

    bool allInitialized = true;
    for (Node*node:mNodes){//int i = 0; i < mNodes.size(); i++) {
        if (!node->mAnimation->isInitialized()) {
            allInitialized = false;
            break;
        }
    }
    mChildrenInitialized = allInitialized;
    return mChildrenInitialized;
}

void AnimatorSet::skipToStartValue(bool inReverse){
     skipToEndValue(!inReverse);
}

void AnimatorSet::setCurrentPlayTime(int64_t playTime){
    if (mReversing && (getTotalDuration() == DURATION_INFINITE)) {
        // Should never get here
        throw std::logic_error("Error: Cannot seek in reverse in an infinite AnimatorSet");
    }

    if (((getTotalDuration() != DURATION_INFINITE) && (playTime > getTotalDuration() - mStartDelay))
            || (playTime < 0)) {
        throw std::logic_error("Error: Play time should always be in between 0 and duration.");
    }

    initAnimation();

    if (!isStarted()) {
        if (mReversing) {
            throw std::logic_error("Error: Something went wrong. mReversing should not be set when AnimatorSet is not started.");
        }
        if (!mSeekState->isActive()) {
            findLatestEventIdForTime(0);
            // Set all the values to start values.
            initChildren();
            skipToStartValue(mReversing);
            mSeekState->setPlayTime(0, mReversing);
        }
        animateBasedOnPlayTime(playTime, 0, mReversing);
        mSeekState->setPlayTime(playTime, mReversing);
    } else {
        // If the animation is running, just set the seek time and wait until the next frame
        // (i.e. doAnimationFrame(...)) to advance the animation.
        mSeekState->setPlayTime(playTime, mReversing);
    }
}

int64_t AnimatorSet::getCurrentPlayTime() {
    if (mSeekState->isActive()) {
        return mSeekState->getPlayTime();
    }
    if (mLastFrameTime == -1) {
        // Not yet started or during start delay
        return 0;
    }
    float durationScale = ValueAnimator::getDurationScale();
    durationScale = (durationScale == 0) ? 1 : durationScale;
    if (mReversing) {
        return int64_t((mLastFrameTime - mFirstFrame) / durationScale);
    } else {
        return int64_t((mLastFrameTime - mFirstFrame - mStartDelay) / durationScale);
    }
}

void AnimatorSet::initChildren(){
    if (!isInitialized()) {
        mChildrenInitialized = true;
        // Forcefully initialize all children based on their end time, so that if the start
        // value of a child is dependent on a previous animation, the animation will be
        // initialized after the the previous animations have been advanced to the end.
        skipToEndValue(false);
    }
}

bool AnimatorSet::doAnimationFrame(int64_t frameTime){
    const float durationScale = ValueAnimator::getDurationScale();
    if (durationScale == 0.f) {
        // Duration scale is 0, end the animation right away.
        forceToEnd();
        return true;
    }
    // After the first frame comes in, we need to wait for start delay to pass before updating
    // any animation values.
    if (mFirstFrame < 0) {
        mFirstFrame = frameTime;
    }

    LOGV("%p frameTime=(%lld-%lld)=%d",this,frameTime,mFirstFrame,int(frameTime-mFirstFrame));
    // Handle pause/resume
    if (mPaused) {
        // Note: Child animations don't receive pause events. Since it's never a contract that
        // the child animators will be paused when set is paused, this is unlikely to be an
        // issue.
        mPauseTime = frameTime;
        removeAnimationCallback();
        return false;
    } else if (mPauseTime > 0) {
            // Offset by the duration that the animation was paused
        mFirstFrame += (frameTime - mPauseTime);
        mPauseTime = -1;
    }

    // Continue at seeked position
    if (mSeekState->isActive()) {
        mSeekState->updateSeekDirection(mReversing);
        if (mReversing) {
            mFirstFrame = int64_t(frameTime - mSeekState->getPlayTime() * durationScale);
        } else {
            mFirstFrame = int64_t(frameTime - (mSeekState->getPlayTime() + mStartDelay) * durationScale);
        }
        mSeekState->reset();
    }

    if (!mReversing && (frameTime < (mFirstFrame + mStartDelay * durationScale))) {
        // Still during start delay in a forward playing case.
        return false;
    }

    // From here on, we always use unscaled play time. Note this unscaled playtime includes
    // the start delay.
    const int64_t unscaledPlayTime = int64_t((frameTime - mFirstFrame) / durationScale);
    mLastFrameTime = frameTime;

    // 1. Pulse the animators that will start or end in this frame
    // 2. Pulse the animators that will finish in a later frame
    const int latestId = findLatestEventIdForTime(unscaledPlayTime);
    const int startId = mLastEventId;

    LOGV("%p startId=%d latestId=%d mEvents.size=%d",this,startId,latestId,mEvents.size());
    handleAnimationEvents(startId, latestId, unscaledPlayTime);

    mLastEventId = latestId;

    // Pump a frame to the on-going animators
    for (Node*node:mPlayingSet){
        if (!node->mEnded) {
            pulseFrame(node, getPlayTimeForNode(unscaledPlayTime, node));
        }
    }

    // Remove all the finished anims
    for (int i = int(mPlayingSet.size()-1);i>=0;i--) {
        if (mPlayingSet.at(i)->mEnded) {
            mPlayingSet.erase(mPlayingSet.begin()+i);
        }
    }

    bool finished = false;
    if (mReversing) {
        if ((mPlayingSet.size() == 1) && (mPlayingSet.at(0) == mRootNode)) {
            // The only animation that is running is the delay animation.
            finished = true;
        } else if ( mPlayingSet.empty() && (mLastEventId < 3)) {
            // The only remaining animation is the delay animation
            finished = true;
        }
    } else {
        finished = mPlayingSet.empty() && (mLastEventId == mEvents.size() - 1);
    }

    if (finished) {
        endAnimation();
        return true;
    }
    return false;
}

void AnimatorSet::commitAnimationFrame(int64_t frameTime) {
    // No op.
}

bool AnimatorSet::pulseAnimationFrame(int64_t frameTime) {
    return doAnimationFrame(frameTime);
}

void AnimatorSet::handleAnimationEvents(int startId, int latestId, int64_t playTime) {
    if (mReversing) {
        startId = startId == -1 ? int(mEvents.size()) : startId;
        for (int i = startId - 1; i >= latestId; i--) {
            AnimationEvent* event = mEvents.at(i);
            Node* node = event->mNode;
            if (event->mEvent == AnimationEvent::ANIMATION_END) {
                if (node->mAnimation->isStarted()) {
                    // If the animation has already been started before its due time (i.e.
                    // the child animator is being manipulated outside of the AnimatorSet), we
                    // need to cancel the animation to reset the internal state (e.g. frame
                    // time tracking) and remove the self pulsing callbacks
                    node->mAnimation->cancel();
                }
                node->mEnded = false;
                mPlayingSet.push_back(event->mNode);
                node->mAnimation->startWithoutPulsing(true);
                pulseFrame(node, 0);
            } else if (event->mEvent == AnimationEvent::ANIMATION_DELAY_ENDED && !node->mEnded) {
                // end event:
                pulseFrame(node, getPlayTimeForNode(playTime, node));
            }
        }
    } else {
        for (int i = startId + 1; i <= latestId; i++) {
            AnimationEvent* event = mEvents.at(i);
            Node* node = event->mNode;
            if (event->mEvent == AnimationEvent::ANIMATION_START) {
                mPlayingSet.push_back(event->mNode);
                if (node->mAnimation->isStarted()) {
                    // If the animation has already been started before its due time (i.e.
                    // the child animator is being manipulated outside of the AnimatorSet), we
                    // need to cancel the animation to reset the internal state (e.g. frame
                    // time tracking) and remove the self pulsing callbacks
                    node->mAnimation->cancel();
                }
                node->mEnded = false;
                node->mAnimation->startWithoutPulsing(false);
                pulseFrame(node, 0);
            } else if ((event->mEvent == AnimationEvent::ANIMATION_END) && !node->mEnded) {
                // start event:
                pulseFrame(node, getPlayTimeForNode(playTime, node));
            }
        }
    }
}

void AnimatorSet::pulseFrame(AnimatorSet::Node* node, int64_t animPlayTime) {
    if (!node->mEnded) {
        float durationScale = ValueAnimator::getDurationScale();
        durationScale = (durationScale == 0) ? 1 : durationScale;
        node->mEnded = node->mAnimation->pulseAnimationFrame((animPlayTime * durationScale));
    }
}

int64_t AnimatorSet::getPlayTimeForNode(int64_t overallPlayTime, AnimatorSet::Node* node) {
    return getPlayTimeForNode(overallPlayTime, node, mReversing);
}

int64_t AnimatorSet::getPlayTimeForNode(int64_t overallPlayTime, AnimatorSet::Node* node, bool inReverse) {
    if (inReverse) {
        overallPlayTime = getTotalDuration() - overallPlayTime;
        return node->mEndTime - overallPlayTime;
    } else {
        return overallPlayTime - node->mStartTime;
    }
}

void AnimatorSet::startAnimation() {
    addDummyListener();

    // Register animation callback
    addAnimationCallback(0);

    if ((mSeekState->getPlayTimeNormalized() == 0) && mReversing) {
        // Maintain old behavior, if seeked to 0 then call reverse, we'll treat the case
        // the same as no seeking at all.
        mSeekState->reset();
    }
    // Set the child animators to the right end:
    if (mShouldResetValuesAtStart) {
        if (isInitialized()) {
            skipToEndValue(!mReversing);
        } else if (mReversing) {
            // Reversing but haven't initialized all the children yet.
            initChildren();
            skipToEndValue(!mReversing);
        } else {
            // If not all children are initialized and play direction is forward
            for (int i = int(mEvents.size() - 1); i >= 0; i--) {
                if (mEvents.at(i)->mEvent == AnimationEvent::ANIMATION_DELAY_ENDED) {
                    Animator* anim = mEvents.at(i)->mNode->mAnimation;
                    // Only reset the animations that have been initialized to start value,
                    // so that if they are defined without a start value, they will get the
                    // values set at the right time (i.e. the next animation run)
                    if (anim->isInitialized()) {
                        anim->skipToEndValue(true);
                    }
                }
            }
        }
    }

    if (mReversing || (mStartDelay == 0) || mSeekState->isActive()) {
        int64_t playTime = 0;
        // If no delay, we need to call start on the first animations to be consistent with old
        // behavior.
        if (mSeekState->isActive()) {
            mSeekState->updateSeekDirection(mReversing);
            playTime = mSeekState->getPlayTime();
        }
        int toId = findLatestEventIdForTime(playTime);
        handleAnimationEvents(-1, toId, playTime);
        for (int i = int(mPlayingSet.size()-1);i>=0;i--){
            if (mPlayingSet.at(i)->mEnded) {
                mPlayingSet.erase(mPlayingSet.begin()+i);
            }
        }
        mLastEventId = toId;
    }
}

void AnimatorSet::addDummyListener() {
    for (int i = 1; i < mNodes.size(); i++) {
        mNodes.at(i)->mAnimation->addListener(mDummyListener);
    }
}

void AnimatorSet::removeDummyListener() {
    for (int i = 1; i < mNodes.size(); i++) {
        mNodes.at(i)->mAnimation->removeListener(mDummyListener);
    }
}

int AnimatorSet::findLatestEventIdForTime(int64_t currentPlayTime) {
    const int size = (int)mEvents.size();
    int latestId = mLastEventId;
    // Call start on the first animations now to be consistent with the old behavior
    if (mReversing) {
        currentPlayTime = getTotalDuration() - currentPlayTime;
        mLastEventId = (mLastEventId == -1) ? size : mLastEventId;
        for (int j = (mLastEventId - 1); j >= 0; j--) {
            AnimationEvent* event = mEvents.at(j);
            if (event->getTime() >= currentPlayTime) {
                latestId = j;
            }
        }
    } else {
        for (int i = (mLastEventId + 1); i < size; i++) {
            AnimationEvent* event = mEvents.at(i);
            if ((event->getTime() != DURATION_INFINITE)&&(event->getTime() <= currentPlayTime)) {
                latestId = i;
            }
        }
    }
    return latestId;
}

void AnimatorSet::endAnimation() {
    mStarted = false;
    mLastFrameTime = -1;
    mFirstFrame = -1;
    mLastEventId = -1;
    mPaused = false;
    mPauseTime = -1;
    mSeekState->reset();
    mPlayingSet.clear();

    // No longer receive callbacks
    removeAnimationCallback();
    // Call end listener
    std::vector<AnimatorListener>tmpListeners = mListeners;
    for (Animator::AnimatorListener& ls:tmpListeners){
        if(ls.onAnimationEnd)ls.onAnimationEnd(*this, mReversing);
    }
    removeDummyListener();
    mSelfPulse = true;
    mReversing = false;
}

void AnimatorSet::removeAnimationCallback() {
    if (!mSelfPulse) {
        return;
    }
    AnimationHandler::getInstance().removeCallback(this);
}

void AnimatorSet::addAnimationCallback(int64_t delay) {
    if (!mSelfPulse) {
        return;
    }
    AnimationHandler::getInstance().addAnimationFrameCallback(this, delay);
}

AnimatorSet*AnimatorSet::clone()const{
    return new AnimatorSet(*this);
}

bool AnimatorSet::canReverse() {
    return getTotalDuration() != DURATION_INFINITE;
}

void AnimatorSet::reverse() {
    start(true, true);
}

void AnimatorSet::createDependencyGraph(){
    if (!mDependencyDirty) {
        // Check whether any duration of the child animations has changed
        bool durationChanged = false;
        for (int i = 0; i < mNodes.size(); i++) {
            Animator* anim = mNodes.at(i)->mAnimation;
            if (mNodes.at(i)->mTotalDuration != anim->getTotalDuration()) {
                durationChanged = true;
                break;
            }
        }
        if (!durationChanged) {
            return;
        }
    }

    mDependencyDirty = false;
    // Traverse all the siblings and make sure they have all the parents
    const size_t size = mNodes.size();
    for (size_t i = 0; i < size; i++) {
        mNodes.at(i)->mParentsAdded = false;
    }
    for (size_t i = 0; i < size; i++) {
        Node* node = mNodes.at(i);
        if (node->mParentsAdded) {
            continue;
        }

        node->mParentsAdded = true;
        if (node->mSiblings.empty()){// == nullptr) {
            continue;
        }

        // Find all the siblings
        findSiblings(node, node->mSiblings);
        auto it =std::find(node->mSiblings.begin(),node->mSiblings.end(),node);
        if(it!=node->mSiblings.end())node->mSiblings.erase(it);

        // Get parents from all siblings
        size_t siblingSize = node->mSiblings.size();
        for (int j = 0; j < siblingSize; j++) {
            node->addParents(node->mSiblings.at(j)->mParents);
        }

        // Now make sure all siblings share the same set of parents
        for (size_t j = 0; j < siblingSize; j++) {
            Node* sibling = node->mSiblings.at(j);
            sibling->addParents(node->mParents);
            sibling->mParentsAdded = true;
        }
    }

    for (size_t i = 0; i < size; i++) {
        Node* node = mNodes.at(i);
        if ((node != mRootNode) && node->mParents.empty()){// == nullptr)) {
            node->addParent(mRootNode);
        }
    }

    // Do a DFS on the tree
    std::vector<Node*> visited;//(mNodes.size());
    // Assign start/end time
    mRootNode->mStartTime = 0;
    mRootNode->mEndTime = mDelayAnim->getDuration();
    updatePlayTime(mRootNode, visited);

    sortAnimationEvents();
    mTotalDuration = mEvents.at(mEvents.size() - 1)->getTime();
}

bool AnimatorSet::AnimationEventCompare(const AnimationEvent* e1, const AnimationEvent* e2){
    const auto t1 = e1->getTime();
    const auto t2 = e2->getTime();
    //LOGD("t1=%lld t2=%lld events=%d/%d",t1,t2,e1->mEvent,e2->mEvent);
    if (t1 == t2) {
        // For events that happen at the same time, we need them to be in the sequence
        // (end, start, start delay ended)
        if ((e2->mEvent + e1->mEvent) == (AnimationEvent::ANIMATION_START
                + AnimationEvent::ANIMATION_DELAY_ENDED)) {
            // Ensure start delay happens after start
            return e1->mEvent < e2->mEvent;
        } else {
            return e2->mEvent < e1->mEvent;
        }
    }
    if (t2 == DURATION_INFINITE) {
        return true;
    }
    if (t1 == DURATION_INFINITE) {
        return false;
    }
    // When neither event happens at INFINITE time:
    return t1 < t2;
}

void AnimatorSet::sortAnimationEvents(){
    // Sort the list of events in ascending order of their time
    // Create the list including the delay animation.
    for(auto e:mEvents)delete e;
    mEvents.clear();
    for (int i = 1; i < mNodes.size(); i++) {
        Node* node = mNodes.at(i);
        mEvents.push_back(new AnimationEvent(node, AnimationEvent::ANIMATION_START));
        mEvents.push_back(new AnimationEvent(node, AnimationEvent::ANIMATION_DELAY_ENDED));
        mEvents.push_back(new AnimationEvent(node, AnimationEvent::ANIMATION_END));
    }
    std::sort(mEvents.begin(),mEvents.end(),AnimationEventCompare);

    const size_t eventSize = mEvents.size();
    // For the same animation, start event has to happen before end.
    for (size_t i = 0; i < eventSize;) {
        AnimationEvent* event = mEvents.at(i);
        if (event->mEvent == AnimationEvent::ANIMATION_END) {
            bool needToSwapStart = false;
            if (event->mNode->mStartTime == event->mNode->mEndTime) {
                needToSwapStart = true;
            } else if (event->mNode->mEndTime == event->mNode->mStartTime
                    + event->mNode->mAnimation->getStartDelay()) {
                // Swapping start delay
                needToSwapStart = false;
            } else {
                i++;
                continue;
            }

            int startEventId = eventSize;
            int startDelayEndId = eventSize;
            for (size_t j = i + 1; j < eventSize; j++) {
                if ((startEventId < eventSize) && (startDelayEndId < eventSize)) {
                    break;
                }
                if (mEvents.at(j)->mNode == event->mNode) {
                    if (mEvents.at(j)->mEvent == AnimationEvent::ANIMATION_START) {
                        // Found start event
                        startEventId = j;
                    } else if (mEvents.at(j)->mEvent == AnimationEvent::ANIMATION_DELAY_ENDED) {
                        startDelayEndId = j;
                    }
                }

            }
            if (needToSwapStart && (startEventId == mEvents.size())) {
                throw std::logic_error("Something went wrong, no start is found after "
                        "stop for an animation that has the same start and end time.");

            }
            if (startDelayEndId == mEvents.size()) {
                throw std::logic_error("Something went wrong, no start"
                        "delay end is found after stop for an animation");

            }

            // We need to make sure start is inserted before start delay ended event,
            // because otherwise inserting start delay ended events first would change
            // the start event index.
            if (needToSwapStart) {
                auto it = mEvents.begin()+startEventId;
                AnimationEvent* startEvent = *it;
                mEvents.erase(it);//remove(startEventId);
                mEvents.insert(mEvents.begin()+i, startEvent);
                i++;
            }
            auto it = mEvents.begin()+startDelayEndId;
            AnimationEvent* startDelayEndEvent = *it;
            mEvents.erase(it);//remove(startDelayEndId);
            mEvents.insert(mEvents.begin()+i, startDelayEndEvent);
            i += 2;
        } else {
            i++;
        }
    }

    if (!mEvents.empty() && (mEvents.at(0)->mEvent != AnimationEvent::ANIMATION_START)) {
        throw std::logic_error("Sorting went bad, the start event should always be at index 0");
    }

    // Add AnimatorSet's start delay node to the beginning
    mEvents.insert(mEvents.begin(), new AnimationEvent(mRootNode, AnimationEvent::ANIMATION_END));
    mEvents.insert(mEvents.begin(), new AnimationEvent(mRootNode, AnimationEvent::ANIMATION_DELAY_ENDED));
    mEvents.insert(mEvents.begin(), new AnimationEvent(mRootNode, AnimationEvent::ANIMATION_START));

    if ((mEvents.at(mEvents.size() - 1)->mEvent == AnimationEvent::ANIMATION_START)
            || (mEvents.at(mEvents.size() - 1)->mEvent == AnimationEvent::ANIMATION_DELAY_ENDED)) {
        throw std::logic_error("Something went wrong, the last event is not an end event");
    }
}

void AnimatorSet::updatePlayTime(AnimatorSet::Node* parent,std::vector<AnimatorSet::Node*>& visited){
    if (parent->mChildNodes.empty()) {
        if (parent == mRootNode) {
            // All the animators are in a cycle
            for (Node*node: mNodes) {
                if (node != mRootNode) {
                    node->mStartTime = DURATION_INFINITE;
                    node->mEndTime = DURATION_INFINITE;
                }
            }
        }
        return;
    }

    visited.push_back(parent);
    const size_t childrenSize = parent->mChildNodes.size();
    for (size_t i = 0;i < childrenSize;i++){
        Node*child = parent->mChildNodes.at(i);
        child->mTotalDuration = child->mAnimation->getTotalDuration();// Update cached duration.
        auto it = std::find(visited.begin(),visited.end(),child);
        if (it!=visited.end()) {
            const int index = it-visited.begin();
            // Child has been visited, cycle found. Mark all the nodes in the cycle.
            for (int j=index; j<visited.size();j++) {
                visited.at(j)->mLatestParent = nullptr;
                visited.at(j)->mStartTime = DURATION_INFINITE;
                visited.at(j)->mEndTime = DURATION_INFINITE;
            }
            child->mStartTime = DURATION_INFINITE;
            child->mEndTime = DURATION_INFINITE;
            child->mLatestParent = nullptr;
            LOGW("Cycle found in AnimatorSet: %p", this);
            continue;
        }

        if (child->mStartTime != DURATION_INFINITE) {
            if (parent->mEndTime == DURATION_INFINITE) {
                child->mLatestParent = parent;
                child->mStartTime = DURATION_INFINITE;
                child->mEndTime = DURATION_INFINITE;
            } else {
                if (parent->mEndTime >= child->mStartTime) {
                    child->mLatestParent = parent;
                    child->mStartTime = parent->mEndTime;
                }

                child->mEndTime = (child->mTotalDuration == DURATION_INFINITE) ?
                        DURATION_INFINITE : (child->mStartTime + child->mTotalDuration);
            }
        }
        updatePlayTime(child, visited);
    }
    auto it = std::find(visited.begin(),visited.end(),parent);
    if(it!=visited.end()) visited.erase(it);
}

void AnimatorSet::findSiblings(AnimatorSet::Node* node,std::vector<AnimatorSet::Node*>& siblings){
    auto it = std::find(siblings.begin(),siblings.end(),node);
    if (it== siblings.end()){//!contains(node)) {
        siblings.push_back(node);
        if(node->mSiblings.empty()){
            return ;
        }
        for (Node*sibling:node->mSiblings) {
            findSiblings(sibling, siblings);
        }
    }
}

bool AnimatorSet::shouldPlayTogether() {
    updateAnimatorsDuration();
    createDependencyGraph();
    // All the child nodes are set out to play right after the delay animation
    return mRootNode->mChildNodes.empty() || (mRootNode->mChildNodes.size() == mNodes.size() - 1);
}

int64_t AnimatorSet::getTotalDuration() {
    updateAnimatorsDuration();
    createDependencyGraph();
    return mTotalDuration;
}

AnimatorSet::Node* AnimatorSet::getNodeForAnimation(Animator* anim){
    auto it = mNodeMap.find(anim);
    Node* node = nullptr;
    if (it==mNodeMap.end()) {
        node = new Node(anim);
        mNodeMap.insert({anim, node});
        mNodes.push_back(node);
    }else {
        node= it->second;
    }
    return node;
}

std::string AnimatorSet::toString()const{
    std::string str ="AnimatorSet@";
    str+=std::to_string((long)this);
    return str;
}

//////////////////////////////////////////////////////////////////////////////////////////////

AnimatorSet::Node::Node(Animator* animation){
    mAnimation = animation;
    mEnded = mParentsAdded = false;
    mStartTime = mEndTime = 0;
    mTotalDuration = 0;
}

AnimatorSet::Node* AnimatorSet::Node::clone(){
    Node* node = new Node(nullptr);
    node->mAnimation = mAnimation->clone();
    node->mChildNodes= mChildNodes;
    node->mSiblings  = mSiblings;
    node->mParents   = mParents;
    
    node->mEnded = false;
    return node;
}

void AnimatorSet::Node::addChild(AnimatorSet::Node* node){
    auto it = std::find(mChildNodes.begin(),mChildNodes.end(),node);
    if(it==mChildNodes.end()){
        mChildNodes.push_back(node);
        node->addParent(this);
    }
}

void AnimatorSet::Node::addSibling(AnimatorSet::Node* node){
    if(std::find(mSiblings.begin(),mSiblings.end(),node)==mSiblings.end()){
        mSiblings.push_back(node);
        node->addSibling(this);
    }
}

void AnimatorSet::Node::addParent(AnimatorSet::Node* node){
    if(std::find(mParents.begin(),mParents.end(),node)==mParents.end()){
        mParents.push_back(node);
        node->addChild(this);
    }
}

void AnimatorSet::Node::addParents(const std::vector<AnimatorSet::Node*>& parents){
    for (auto p:parents) {
        addParent(p);
    }
}

AnimatorSet::AnimationEvent::AnimationEvent(Node* node, int event)
    :mNode(node),mEvent(event){
}

int64_t AnimatorSet::AnimationEvent::getTime()const {
    if (mEvent == ANIMATION_START) {
        return mNode->mStartTime;
    } else if (mEvent == ANIMATION_DELAY_ENDED) {
        return (mNode->mStartTime == DURATION_INFINITE)
                ? DURATION_INFINITE : (mNode->mStartTime + mNode->mAnimation->getStartDelay());
    } else {
        return mNode->mEndTime;
    }
}

AnimatorSet::SeekState::SeekState(AnimatorSet*set){
    mAnimSet = set;
    reset();
}

void AnimatorSet::SeekState::reset() {
    mPlayTime = -1;
    mSeekingInReverse = false;
}

void AnimatorSet::SeekState::setPlayTime(int64_t playTime, bool inReverse) {
    // TODO: This can be simplified.

    // Clamp the play time
    if (mAnimSet->getTotalDuration() != DURATION_INFINITE) {
        mPlayTime = std::min(playTime, int64_t(mAnimSet->getTotalDuration() - mAnimSet->mStartDelay));
    }
    mPlayTime = std::max(int64_t(0), mPlayTime);
    mSeekingInReverse = inReverse;
}

void AnimatorSet::SeekState::updateSeekDirection(bool inReverse){
    // Change seek direction without changing the overall fraction
    if (inReverse && (mAnimSet->getTotalDuration() == DURATION_INFINITE)) {
         throw std::logic_error("Error: Cannot reverse infinite animator set");
    }
    if (mPlayTime >= 0) {
        if (inReverse != mSeekingInReverse) {
            mPlayTime = mAnimSet->getTotalDuration() - mAnimSet->mStartDelay - mPlayTime;
            mSeekingInReverse = inReverse;
        }
    }
}

int64_t AnimatorSet::SeekState::getPlayTime()const{
    return mPlayTime;
}

int64_t AnimatorSet::SeekState::getPlayTimeNormalized()const{
    if (mAnimSet->mReversing) {
        return mAnimSet->getTotalDuration() - mAnimSet->mStartDelay - mPlayTime;
    }
    return mPlayTime;
}

bool AnimatorSet::SeekState::isActive()const{
    return mPlayTime != -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////
AnimatorSet::Builder::Builder(AnimatorSet*set,Animator* anim) {
    mAnimSet = set;
    mAnimSet->mDependencyDirty = true;
    mCurrentNode = mAnimSet->getNodeForAnimation(anim);
}

AnimatorSet::Builder& AnimatorSet::Builder::with(Animator* anim) {
    Node* node = mAnimSet->getNodeForAnimation(anim);
    mCurrentNode->addSibling(node);
    return *this;
}

AnimatorSet::Builder& AnimatorSet::Builder::before(Animator* anim) {
    Node* node = mAnimSet->getNodeForAnimation(anim);
    mCurrentNode->addChild(node);
    return *this;
}

AnimatorSet::Builder& AnimatorSet::Builder::after(Animator* anim) {
    Node* node = mAnimSet->getNodeForAnimation(anim);
    mCurrentNode->addParent(node);
    return *this;
}

AnimatorSet::Builder& AnimatorSet::Builder::after(int64_t delay) {
    // setup dummy ValueAnimator just to run the clock
    ValueAnimator* anim = ValueAnimator::ofFloat({0.f, 1.f});
    anim->setDuration(delay);
    after(anim);
    return *this;
}

}//endof namespace
