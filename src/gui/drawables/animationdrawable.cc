#include <drawables/animationdrawable.h>
#include <core/systemclock.h>
#include <cdlog.h>
namespace cdroid{
#pragma GCC push_options
#pragma GCC optimize("O0")
AnimationDrawable::AnimationDrawable():AnimationDrawable(nullptr){
}

AnimationDrawable::AnimationDrawable(std::shared_ptr<AnimationDrawable::AnimationState>state){
    std::shared_ptr<AnimationState>as =std::make_shared<AnimationState>(state.get(),this);
    setConstantState(as);
    mRunning  = false;
    mCurFrame = 0;
    mMutated  = false;
    mAnimating= false;
    if(state)setFrame(0,true,false);
    mRunnable=std::bind(&AnimationDrawable::run,this);
}

AnimationDrawable::AnimationDrawable(Context*ctx,const AttributeSet&atts)
    :AnimationDrawable(){
    mAnimationState->setConstantSize(atts.getBoolean("constantSize"));
    mAnimationState->setVariablePadding(atts.getBoolean("variablePadding"));
    mAnimationState->setEnterFadeDuration(atts.getInt("enterFadeDuration"));
    mAnimationState->setExitFadeDuration(atts.getInt("exitFadeDuration"));
    mAnimationState->mOneShot = atts.getBoolean("oneshot",false);
}

AnimationDrawable::~AnimationDrawable(){
    mRunnable.reset();
}

void AnimationDrawable::setConstantState(std::shared_ptr<DrawableContainerState>state){
    DrawableContainer::setConstantState(state);
    mAnimationState = std::dynamic_pointer_cast<AnimationState>(state);
}

bool AnimationDrawable::setVisible(bool visible,bool restart){
    const bool changed=DrawableContainer::setVisible(visible,restart);
    if(visible){
        if(restart||changed){
            const bool startFromZero =restart||(!mRunning&&!mAnimationState->mOneShot)||
                (mCurFrame >= mAnimationState->getChildCount() );
            setFrame((startFromZero?0:mCurFrame),true,mAnimating);     
        }
    }else{
         unscheduleSelf(mRunnable);
    }
    return changed;
}

void AnimationDrawable::start(){
    mAnimating = true;
    if(!isRunning())
        setFrame(0,false,mAnimationState->getChildCount()>1||!mAnimationState->mOneShot);
}

void AnimationDrawable::stop(){
    mAnimating = false;
    if(isRunning()){
        mCurFrame =0 ;
        unscheduleSelf(mRunnable);
    }
}

bool AnimationDrawable::isRunning(){
    return mRunning;
}

void AnimationDrawable::run(){
    nextFrame(false);
}

void AnimationDrawable::unscheduleSelf(Runnable&what){
    mRunning = false;
    DrawableContainer::unscheduleSelf(what);
}

int AnimationDrawable::getNumberOfFrames()const{
    return mAnimationState->getChildCount();
}

Drawable* AnimationDrawable::getFrame(int index)const{
    return mAnimationState->getChild(index);
}

int AnimationDrawable::getDuration(int i)const{
    return mAnimationState->mDurations[i];
}

long AnimationDrawable::getTotalDuration()const{
    return mAnimationState->getTotalDuration();
}

bool AnimationDrawable::isOneShot()const{
    return mAnimationState->mOneShot;
}

void AnimationDrawable::setOneShot(bool oneShot){
    mAnimationState->mOneShot = oneShot;
}

void AnimationDrawable::addFrame(Drawable*frame,int duration){
    mAnimationState->addFrame(frame,duration);
    if(!mRunning) setFrame(0,true,false);
}

void AnimationDrawable::nextFrame(bool unschedule){
    int nextFrame =mCurFrame + 1;
    int numFrames =mAnimationState->getChildCount();
    bool isLastFrame =mAnimationState->mOneShot && nextFrame>=(numFrames-1);
    if(!mAnimationState->mOneShot && nextFrame >= numFrames)
        nextFrame=0;
    setFrame(nextFrame,unschedule,!isLastFrame);
}

void AnimationDrawable::setFrame(int frame,bool unschedule,bool animate){
    if(frame >= mAnimationState->getChildCount())
        return;
    mAnimating= animate;
    mCurFrame = frame;
    selectDrawable(frame);

    if(unschedule||animate)
        unscheduleSelf(mRunnable);
    if(animate){
        mCurFrame = frame;
        mRunning = true;
        scheduleSelf(mRunnable,SystemClock::uptimeMillis()+mAnimationState->mDurations[frame]);
    }
}

AnimationDrawable* AnimationDrawable::mutate(){
    if(!mMutated && DrawableContainer::mutate()==this){
        mAnimationState->mutate();
        mMutated = true; 
    }
    return this;
}

std::shared_ptr<DrawableContainer::DrawableContainerState> AnimationDrawable::cloneConstantState(){
    return std::make_shared<AnimationState>(mAnimationState.get(),this);
}

void AnimationDrawable::clearMutated(){
    DrawableContainer::clearMutated();
    mMutated = false;
}

Drawable*AnimationDrawable::inflate(Context*ctx,const AttributeSet&attrs){
    AnimationDrawable*d = new AnimationDrawable(ctx,attrs);
    return d;
}

////////////////////////////////////////////////////////////////////////////////////////////
AnimationDrawable::AnimationState::AnimationState(const AnimationState*orig,AnimationDrawable*owner)
    :DrawableContainer::DrawableContainerState(orig,owner){
    if(orig){
        mDurations= orig->mDurations;
        mOneShot  = orig->mOneShot;
    }else{
        mOneShot = false;
    }
}

void AnimationDrawable::AnimationState::mutate(){
}

AnimationDrawable*AnimationDrawable::AnimationState::newDrawable(){
    return new AnimationDrawable(std::dynamic_pointer_cast<AnimationState>(shared_from_this()));
}

void AnimationDrawable::AnimationState::addFrame(Drawable*dr,int dur){
    const int pos = DrawableContainer::DrawableContainerState::addChild(dr);
    if( mDurations.size() < getChildCount())
       mDurations.resize(getChildCount());
    mDurations[pos] = dur;
}

long AnimationDrawable::AnimationState::getTotalDuration()const{
    int total = 0;
    for (int dur : mDurations) {
        total += dur;
    }
    return total;
}

#pragma GCC pop_options
}
