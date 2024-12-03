#include <animation/statelistanimator.h>
namespace cdroid{

StateListAnimator::StateListAnimator(){
    mAnimationListener.onAnimationEnd=[this](Animator& animator,bool){
        if (mRunningAnimator == &animator) {
            mRunningAnimator = nullptr;
        }
    };
}

void StateListAnimator::addState(const std::vector<int>&specs, Animator* animator){
    Tuple tuple =Tuple(specs, animator);
    animator->addListener(mAnimationListener);
    mTuples.push_back(tuple);
    mChangingConfigurations |= animator->getChangingConfigurations();
}

void StateListAnimator::setState(const std::vector<int>&state){
    Tuple* match = nullptr;
    const size_t count = mTuples.size();
    for (size_t i = 0; i < count; i++) {
        Tuple& tuple = mTuples.at(i);
        if (StateSet::stateSetMatches(tuple.mSpecs, state)) {
            match = &tuple;
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
        mTuples.at(i).mAnimator->setTarget(nullptr);
    }
    mView = nullptr;
    mLastMatch = nullptr;
    mRunningAnimator = nullptr;
}

void StateListAnimator::start(Tuple* match) {
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
    if (mRunningAnimator != nullptr) {
        mRunningAnimator->end();
        mRunningAnimator = nullptr;
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

std::shared_ptr<ConstantState<StateListAnimator*>>StateListAnimator::createConstantState(){
    return mConstantState;
}

/////////////////////////////////////////////////////////////////
StateListAnimator::StateListAnimatorConstantState::StateListAnimatorConstantState(StateListAnimator* animator) {
    mAnimator = animator;
    mAnimator->mConstantState = shared_from_this();
    mChangingConf = mAnimator->getChangingConfigurations();
}

int StateListAnimator::StateListAnimatorConstantState::getChangingConfigurations()const{
    return mChangingConf;
}

StateListAnimator* StateListAnimator::StateListAnimatorConstantState::newInstance() {
    StateListAnimator* clone = nullptr;//mAnimatorclone();
    clone->mConstantState = shared_from_this();
    return clone;
}

}
