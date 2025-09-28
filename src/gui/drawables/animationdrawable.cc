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
    if(state) setFrame(0,true,false);
    mRunnable = std::bind(&AnimationDrawable::run,this);
}

AnimationDrawable::~AnimationDrawable(){
    mRunnable = nullptr;
}

void AnimationDrawable::setConstantState(std::shared_ptr<DrawableContainerState>state){
    DrawableContainer::setConstantState(state);
    mAnimationState = std::dynamic_pointer_cast<AnimationState>(state);
}

bool AnimationDrawable::setVisible(bool visible,bool restart){
    const bool changed = DrawableContainer::setVisible(visible,restart);
    if(visible){
        if(restart||changed){
            const bool startFromZero = restart ||(!mRunning&&!mAnimationState->mOneShot)||
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
        setFrame(0,false,(mAnimationState->getChildCount()>1)||!mAnimationState->mOneShot);
}

void AnimationDrawable::stop(){
    mAnimating = false;
    if(isRunning()){
        mCurFrame = 0 ;
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

int64_t AnimationDrawable::getDuration(int i)const{
    return mAnimationState->mDurations[i];
}

int64_t AnimationDrawable::getTotalDuration()const{
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
    int nextFrame = mCurFrame + 1;
    const int numFrames = mAnimationState->getChildCount();
    const bool isLastFrame = mAnimationState->mOneShot && (nextFrame >= numFrames-1);
    if(!mAnimationState->mOneShot && (nextFrame >= numFrames) )
        nextFrame = 0;
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

void AnimationDrawable::inflate(XmlPullParser& parser,const AttributeSet& atts){
    DrawableContainer::inflateWithAttributes(parser,atts);
    updateStateFromTypedArray(atts);

    //updateDensity();
    inflateChildElements(parser,atts);
    setFrame(0,true,false);
}

void AnimationDrawable::updateStateFromTypedArray(const AttributeSet&atts){
    auto state = mAnimationState;
    state->mVariablePadding = atts.getBoolean("variablePadding", state->mVariablePadding);
    state->mOneShot = atts.getBoolean("oneshot", state->mOneShot);
}

void AnimationDrawable::inflateChildElements(XmlPullParser& parser,const AttributeSet& atts){
    int type,depth;
    const int innerDepth = parser.getDepth()+1;
    while ((type=parser.next()) != XmlPullParser::END_DOCUMENT
            && ((depth=parser.getDepth()) >= innerDepth || type != XmlPullParser::END_TAG)) {
        if (type != XmlPullParser::START_TAG) {
            continue;
        }

        if ((depth > innerDepth) || parser.getName().compare("item")) {
            continue;
        }
        const int duration = atts.getInt("duration", -1);
        if (duration < 0) {
            throw std::logic_error(parser.getPositionDescription()+": <item> tag requires a 'duration' attribute");
        }

        Drawable* dr = atts.getDrawable("drawable");

        if (dr == nullptr) {
            while ((type=parser.next()) == XmlPullParser::TEXT) {
                // Empty
            }
            if (type != XmlPullParser::START_TAG) {
                throw std::logic_error(parser.getPositionDescription()+
                        ": <item> tag requires a 'drawable' attribute or child tag"
                        " defining a drawable");
            }
            dr = Drawable::createFromXmlInner(parser, atts);
        }

        mAnimationState->addFrame(dr, duration);
        if (dr != nullptr) {
            dr->setCallback(mCallback);
        }
    }
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

int64_t AnimationDrawable::AnimationState::getTotalDuration()const{
    int64_t total = 0;
    for (int dur : mDurations) {
        total += dur;
    }
    return total;
}

#pragma GCC pop_options
}
