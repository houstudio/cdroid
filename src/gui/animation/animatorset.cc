#include <animation/animatorset.h>
#include <animation/objectanimator.h>
#include <cdlog.h>

namespace cdroid{

AnimatorSet::AnimatorSet(){
    mDelayAnim = ValueAnimator::ofFloat({.0f, 1.f});
    mDelayAnim->setDuration(0);
    mRootNode  = new Node(mDelayAnim);
    mNodeMap.insert(std::pair<Animator*,Node*>(mDelayAnim, mRootNode));
    mNodes.push_back(mRootNode);
    bool isPreO;
    // Set the flag to ignore calling end() without start() for pre-N releases
    mShouldIgnoreEndWithoutStart = true;
    isPreO = true;
    mShouldResetValuesAtStart = !isPreO;
    mEndCanBeCalled = !isPreO;
    mSeekState = new SeekState(this);
}

AnimatorSet::~AnimatorSet(){
    for(auto nd:mNodeMap){
        delete nd.first;
    }
    for(auto nd:mNodes)
        delete nd;
    mNodeMap.clear();
    mNodes.clear();
    delete mSeekState;
}

void AnimatorSet::playTogether(const std::vector<Animator*>&items){
    if(items.size()){
        Builder builder(this,items[0]);
        for (int i = 1; i < items.size(); ++i) {
            builder.with(items[i]);
        }
    }
}

void AnimatorSet::playSequentially(const std::vector<Animator*>&items){
    if(items.size()){
       if(items.size()==1){
           play(items[0]);
       } else {
           for (int i = 0; i < items.size() - 1; ++i) {
               play(items[i])->before(items[i + 1]);
           }
       }
    }
}

std::vector<Animator*> AnimatorSet::getChildAnimations()const{
    std::vector<Animator*> childList;
    int size = (int)mNodes.size();
    for (int i = 0; i < size; i++) {
        Node* node = mNodes.at(i);
        if (node != mRootNode) {
            childList.push_back(node->mAnimation);
        }
    }
    return childList;
}

void AnimatorSet::setTarget(void*target){
    for (auto node:mNodes){//int i = 0; i < size; i++) {
        //Node* node = mNodes.get(i);
        Animator* animation = node->mAnimation;
        if (dynamic_cast<AnimatorSet*>(animation)) {
            ((AnimatorSet*)animation)->setTarget(target);
        } else if (dynamic_cast<ObjectAnimator*>(animation)) {
            ((ObjectAnimator*)animation)->setTarget(target);
        }
    }
}

void AnimatorSet::setInterpolator(TimeInterpolator* interpolator) {
    mInterpolator = interpolator;
}

TimeInterpolator* AnimatorSet::getInterpolator() {
    return mInterpolator;
}

AnimatorSet::Builder* AnimatorSet::play(Animator* anim){
    if(anim==nullptr)return nullptr;
    return new Builder(this,anim);
}

void AnimatorSet::cancel(){
    if (isStarted()) {
        for (auto ls:mListeners) {
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
        long zeroScalePlayTime = getTotalDuration();
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
                auto it=mNodeMap.find(anim);
                if (it->second->mEnded) {
                    continue;
                }
                if (event->mEvent == AnimationEvent::ANIMATION_START) {
                    anim->start();
                } else if (event->mEvent == AnimationEvent::ANIMATION_END && anim->isStarted()) {
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

long AnimatorSet::getStartDelay() {
    return mStartDelay;
}

void AnimatorSet::setStartDelay(long startDelay) {
    // Clamp start delay to non-negative range.
    if (startDelay < 0) {
        LOGW("Start delay should always be non-negative");
        startDelay = 0;
    }
    long delta = startDelay - mStartDelay;
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
                node->mStartTime = node->mStartTime == DURATION_INFINITE ?
                        DURATION_INFINITE : node->mStartTime + delta;
                node->mEndTime = node->mEndTime == DURATION_INFINITE ?
                        DURATION_INFINITE : node->mEndTime + delta;
            }
        }
        // Update total duration, if necessary.
        if (mTotalDuration != DURATION_INFINITE) {
            mTotalDuration += delta;
        }
    }
}

long AnimatorSet::getDuration(){
    return mDuration;
}

Animator& AnimatorSet::setDuration(long duration) {
    if (duration < 0) {
        LOGE("duration must be a value of zero or greater");
    }
    mDependencyDirty = true;
    // Just record the value for now - it will be used later when the AnimatorSet starts
    mDuration = duration;
    return *this;
}

void AnimatorSet::setupStartValues() {
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
    bool previouslyPaused = mPaused;
    Animator::pause();
    if (!previouslyPaused && mPaused) {
        mPauseTime = -1;
    }
}

void AnimatorSet::resume() {
    bool previouslyPaused = mPaused;
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
        //node->mAnimation->setAllowRunningAsynchronously(false);
    }

    initAnimation();
    if (inReverse && !canReverse()) {
        throw "Cannot reverse infinite AnimatorSet";
    }

    mReversing = inReverse;

    // Now that all dependencies are set up, start the animations that should be started.
    bool bisEmptySet = isEmptySet(this);
    if (!bisEmptySet) {
        startAnimation();
    }

    for (AnimatorListener&ls:mListeners) {
        ls.onAnimationStart(*this, inReverse);
    }
    if (bisEmptySet) {
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
        throw ("Children must be initialized.");
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

void AnimatorSet::animateBasedOnPlayTime(long currentPlayTime, long lastPlayTime, bool inReverse){
    if (inReverse) {
        if (getTotalDuration() == DURATION_INFINITE) {
            LOGE("Cannot reverse AnimatorSet with infinite duration");
        }
        long duration = getTotalDuration() - mStartDelay;
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
        long playTime = getPlayTimeForNode(currentPlayTime, node, inReverse);
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

void AnimatorSet::setCurrentPlayTime(long playTime){
    initAnimation();

    if (!isStarted()) {
        if (mReversing) {
            throw "Error: Something went wrong. mReversing should not be set when AnimatorSet is not started.";
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

long AnimatorSet::getCurrentPlayTime() {
    if (mSeekState->isActive()) {
        return mSeekState->getPlayTime();
    }
    if (mLastFrameTime == -1) {
        // Not yet started or during start delay
        return 0;
    }
    float durationScale = ValueAnimator::getDurationScale();
    durationScale = durationScale == 0 ? 1 : durationScale;
    if (mReversing) {
        return (long) ((mLastFrameTime - mFirstFrame) / durationScale);
    } else {
        return (long) ((mLastFrameTime - mFirstFrame - mStartDelay) / durationScale);
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

bool AnimatorSet::doAnimationFrame(long frameTime){
    float durationScale = ValueAnimator::getDurationScale();
    if (durationScale == .0f) {
        // Duration scale is 0, end the animation right away.
        forceToEnd();
        return true;
    }

    // After the first frame comes in, we need to wait for start delay to pass before updating
    // any animation values.
    if (mFirstFrame < 0) {
        mFirstFrame = frameTime;
    }

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
            mFirstFrame = (long) (frameTime - mSeekState->getPlayTime() * durationScale);
        } else {
            mFirstFrame = (long) (frameTime - (mSeekState->getPlayTime() + mStartDelay)
                    * durationScale);
        }
        mSeekState->reset();
    }

    if (!mReversing && frameTime < mFirstFrame + mStartDelay * durationScale) {
        // Still during start delay in a forward playing case.
        return false;
    }

    // From here on, we always use unscaled play time. Note this unscaled playtime includes
    // the start delay.
    long unscaledPlayTime = (long) ((frameTime - mFirstFrame) / durationScale);
    mLastFrameTime = frameTime;

    // 1. Pulse the animators that will start or end in this frame
    // 2. Pulse the animators that will finish in a later frame
    int latestId = findLatestEventIdForTime(unscaledPlayTime);
    int startId = mLastEventId;

    handleAnimationEvents(startId, latestId, unscaledPlayTime);

    mLastEventId = latestId;

    // Pump a frame to the on-going animators
    for (Node*node:mPlayingSet){
        if (!node->mEnded) {
            pulseFrame(node, getPlayTimeForNode(unscaledPlayTime, node));
        }
    }

    // Remove all the finished anims
    for (size_t i = mPlayingSet.size() - 1; i >= 0; i--) {
        if (mPlayingSet.at(i)->mEnded) {
            //mPlayingSet.remove(i);
        }
    }

    bool finished = false;
    if (mReversing) {
        if (mPlayingSet.size() == 1 && mPlayingSet.at(0) == mRootNode) {
            // The only animation that is running is the delay animation.
            finished = true;
        } else if ( mPlayingSet.empty() && (mLastEventId < 3)) {
            // The only remaining animation is the delay animation
            finished = true;
        }
    } else {
        finished = mPlayingSet.empty() && mLastEventId == mEvents.size() - 1;
    }

    if (finished) {
        endAnimation();
        return true;
    }
    return false;
}

void AnimatorSet::commitAnimationFrame(long frameTime) {
    // No op.
}

bool AnimatorSet::pulseAnimationFrame(long frameTime) {
    return doAnimationFrame(frameTime);
}

void AnimatorSet::handleAnimationEvents(int startId, int latestId, long playTime) {
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
            } else if (event->mEvent == AnimationEvent::ANIMATION_END && !node->mEnded) {
                // start event:
                pulseFrame(node, getPlayTimeForNode(playTime, node));
            }
        }
    }
}

void AnimatorSet::pulseFrame(AnimatorSet::Node* node, long animPlayTime) {
    if (!node->mEnded) {
        float durationScale = ValueAnimator::getDurationScale();
        durationScale = durationScale == 0  ? 1 : durationScale;
        node->mEnded = node->mAnimation->pulseAnimationFrame(
                (long) (animPlayTime * durationScale));
    }
}

long AnimatorSet::getPlayTimeForNode(long overallPlayTime, AnimatorSet::Node* node) {
    return getPlayTimeForNode(overallPlayTime, node, mReversing);
}

long AnimatorSet::getPlayTimeForNode(long overallPlayTime, AnimatorSet::Node* node, bool inReverse) {
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

    if (mSeekState->getPlayTimeNormalized() == 0 && mReversing) {
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
            for (size_t i = mEvents.size() - 1; i >= 0; i--) {
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

    if (mReversing || mStartDelay == 0 || mSeekState->isActive()) {
        long playTime;
        // If no delay, we need to call start on the first animations to be consistent with old
        // behavior.
        if (mSeekState->isActive()) {
            mSeekState->updateSeekDirection(mReversing);
            playTime = mSeekState->getPlayTime();
        } else {
            playTime = 0;
        }
        int toId = findLatestEventIdForTime(playTime);
        handleAnimationEvents(-1, toId, playTime);
        for (int i = int(mPlayingSet.size() - 1); i >= 0; i--) {
            if (mPlayingSet.at(i)->mEnded) {
                //mPlayingSet.remove(i);
            }
        }
        mLastEventId = toId;
    }
}

void AnimatorSet::addDummyListener() {
    for (int i = 1; i < mNodes.size(); i++) {
        //mNodes.at(i)->mAnimation->addListener(mDummyListener);
    }
}

void AnimatorSet::removeDummyListener() {
    for (int i = 1; i < mNodes.size(); i++) {
        //mNodes.at(i)->mAnimation->removeListener(mDummyListener);
    }
}

int AnimatorSet::findLatestEventIdForTime(long currentPlayTime) {
    const int size = (int)mEvents.size();
    int latestId = mLastEventId;
    // Call start on the first animations now to be consistent with the old behavior
    if (mReversing) {
        currentPlayTime = getTotalDuration() - currentPlayTime;
        mLastEventId = mLastEventId == -1 ? size : mLastEventId;
        for (int j = mLastEventId - 1; j >= 0; j--) {
            AnimationEvent* event = mEvents.at(j);
            if (event->getTime() >= currentPlayTime) {
                latestId = j;
            }
        }
    } else {
        for (int i = mLastEventId + 1; i < size; i++) {
            AnimationEvent* event = mEvents.at(i);
            if (event->getTime() <= currentPlayTime) {
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
    for (Animator::AnimatorListener& ls:mListeners){
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

void AnimatorSet::addAnimationCallback(long delay) {
    if (!mSelfPulse) {
        return;
    }
    AnimationHandler::getInstance().addAnimationFrameCallback(this, delay);
}

Animator*AnimatorSet::clone()const{
    return nullptr;
}

bool AnimatorSet::canReverse() {
    return getTotalDuration() != DURATION_INFINITE;
}

void AnimatorSet::reverse() {
    start(true, true);
}

void AnimatorSet::createDependencyGraph(){

}

void AnimatorSet::sortAnimationEvents(){
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
    for (Node*child:parent->mChildNodes) {
        auto it = std::find(visited.begin(),visited.end(),child);
        if (it!=visited.end()) {
            // Child has been visited, cycle found. Mark all the nodes in the cycle.
            for (; it!=visited.end();it++) {
                (*it)->mLatestParent = nullptr;
                (*it)->mStartTime = DURATION_INFINITE;
                (*it)->mEndTime = DURATION_INFINITE;
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

                long duration = child->mAnimation->getTotalDuration();
                child->mEndTime = duration == DURATION_INFINITE ?
                        DURATION_INFINITE : child->mStartTime + duration;
            }
        }
        updatePlayTime(child, visited);
    }
    //visited.remove(parent);
}

void AnimatorSet::findSiblings(AnimatorSet::Node* node,std::vector<AnimatorSet::Node*>& siblings){
    auto it=std::find(siblings.begin(),siblings.end(),node);
    if (it== siblings.end()){//contains(node)) {
        siblings.push_back(node);
        for (Node*sibling:node->mSiblings) {
            findSiblings(sibling, siblings);
        }
    }
}

bool AnimatorSet::shouldPlayTogether() {
    updateAnimatorsDuration();
    createDependencyGraph();
    // All the child nodes are set out to play right after the delay animation
    return mRootNode->mChildNodes.empty() || mRootNode->mChildNodes.size() == mNodes.size() - 1;
}

long AnimatorSet::getTotalDuration() {
    updateAnimatorsDuration();
    createDependencyGraph();
    return mTotalDuration;
}

AnimatorSet::Node* AnimatorSet::getNodeForAnimation(Animator* anim){
    auto it= mNodeMap.find(anim);
    Node* node = nullptr;
    if (it==mNodeMap.end()) {
        node = new Node(anim);
        mNodeMap.insert(std::pair<Animator*,Node*>(anim, node));
        mNodes.push_back(node);
    }
    return node;
}

//////////////////////////////////////////////////////////////////////////////////////////////

AnimatorSet::Node::Node(Animator* animation){
    mAnimation = animation;
}

AnimatorSet::Node* AnimatorSet::Node::clone(){
    Node* node =new Node(nullptr);
    node->mAnimation  = mAnimation->clone();
    node->mChildNodes= mChildNodes;
    node->mSiblings  = mSiblings;
    node->mParents   = mParents;
    
    node->mEnded = false;
    return node;
}

void AnimatorSet::Node::addChild(AnimatorSet::Node* node){
    mChildNodes.push_back(node);
    node->addParent(this);
}

void AnimatorSet::Node::addSibling(AnimatorSet::Node* node){
    mSiblings.push_back(node);
    node->addSibling(this);
}

void AnimatorSet::Node::addParent(AnimatorSet::Node* node){
    mParents.push_back(node);
    node->addChild(this);
}

void AnimatorSet::Node::addParents(const std::vector<AnimatorSet::Node*>& parents){
    for (auto p:parents) {
         addParent(p);
    }
}

AnimatorSet::AnimationEvent::AnimationEvent(Node* node, int event) {
    mNode = node;
    mEvent = event;
}

long AnimatorSet::AnimationEvent::getTime()const {
    if (mEvent == ANIMATION_START) {
        return mNode->mStartTime;
    } else if (mEvent == ANIMATION_DELAY_ENDED) {
        return mNode->mStartTime == DURATION_INFINITE
                ? DURATION_INFINITE : mNode->mStartTime + mNode->mAnimation->getStartDelay();
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

void AnimatorSet::SeekState::setPlayTime(long playTime, bool inReverse) {
    // TODO: This can be simplified.

    // Clamp the play time
    if (mAnimSet->getTotalDuration() != DURATION_INFINITE) {
        mPlayTime = std::min(playTime, mAnimSet->getTotalDuration() - mAnimSet->mStartDelay);
    }
    mPlayTime = std::max(0L, mPlayTime);
    mSeekingInReverse = inReverse;
}

void AnimatorSet::SeekState::updateSeekDirection(bool inReverse){
    // Change seek direction without changing the overall fraction
    if (inReverse && mAnimSet->getTotalDuration() == DURATION_INFINITE) {
        LOGE("Error: Cannot reverse infinite animator set");
    }
    if (mPlayTime >= 0) {
        if (inReverse != mSeekingInReverse) {
            mPlayTime = mAnimSet->getTotalDuration() - mAnimSet->mStartDelay - mPlayTime;
            mSeekingInReverse = inReverse;
        }
    }
}

long AnimatorSet::SeekState::getPlayTime()const{
    return mPlayTime;
}

long AnimatorSet::SeekState::getPlayTimeNormalized()const{
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
    mAnimSet =set;
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

AnimatorSet::Builder& AnimatorSet::Builder::after(long delay) {
    // setup dummy ValueAnimator just to run the clock
    ValueAnimator* anim = ValueAnimator::ofFloat({.0f, 1.f});
    anim->setDuration(delay);
    after(anim);
    return *this;
}

}//endof namespace
