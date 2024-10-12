#include <stdlib.h>
#include <view/viewgroup.h>
#include <animation/animationutils.h>
#include <animation/layoutanimationcontroller.h>
#include <random>

namespace cdroid{

LayoutAnimationController::LayoutAnimationController(Context* context, const AttributeSet& attrs){
    mDelay = attrs.getFloat("delay");
    mOrder = attrs.getInt("animationOrder",std::map<const std::string,int>{
       {"normal" ,(int)ORDER_NORMAL},
       {"reverse",(int)ORDER_REVERSE},
       {"random" ,(int)ORDER_RANDOM}
    },ORDER_NORMAL);
    std::string resource = attrs.getString("animation");
    mAnimation    = nullptr;
    mInterpolator = nullptr;
    mMaxDelay  = LONG_MIN;
    setAnimation(context,resource);
    resource   = attrs.getString("interpolator");
    setInterpolator(context,resource);
}

LayoutAnimationController::LayoutAnimationController(Animation* animation,float delay){
    mDelay = delay;
    mOrder =0;
    mInterpolator= nullptr;
    mAnimation   = nullptr;
    mMaxDelay    = LONG_MIN;
    setAnimation(animation);
}

LayoutAnimationController::LayoutAnimationController(Animation* animation)
 :LayoutAnimationController(animation,.5f){
}

LayoutAnimationController::~LayoutAnimationController(){
    delete mAnimation;
    delete mInterpolator;
}

int LayoutAnimationController::getOrder()const{
    return mOrder;
}

void LayoutAnimationController::setOrder(int order){
    mOrder = order;
}

void LayoutAnimationController::setAnimation(Context* context,const std::string&resourceID){
    setAnimation(AnimationUtils::loadAnimation(context,resourceID));
}

void LayoutAnimationController::setAnimation(Animation* animation){
    if(mAnimation)delete mAnimation;
    mAnimation = animation;
    mAnimation->setFillBefore(true);
}

Animation* LayoutAnimationController::getAnimation(){
    return mAnimation;
}

void LayoutAnimationController::setInterpolator(Context* context,const std::string&resourceID){
    setInterpolator(AnimationUtils::loadInterpolator(context,resourceID));
}

void LayoutAnimationController::setInterpolator(Interpolator* interpolator){
    if(mInterpolator)delete mInterpolator;
    mInterpolator = interpolator;
}

Interpolator* LayoutAnimationController::getInterpolator(){
    return mInterpolator;
}

float LayoutAnimationController::getDelay()const{
    return mDelay;
}

void LayoutAnimationController::setDelay(float delay){
    mDelay= delay;
}

bool LayoutAnimationController::willOverlap(){
    return mDelay < 1.0f;
}

void LayoutAnimationController::start(){
    mDuration = mAnimation->getDuration();
    mMaxDelay = LONG_MIN;//Long.MIN_VALUE;
    mAnimation->setStartTime(-1);
}

Animation* LayoutAnimationController::getAnimationForView(View* view){
    long delay = getDelayForView(view) + mAnimation->getStartOffset();
    mMaxDelay = std::max(mMaxDelay, delay);

    Animation* animation = mAnimation->clone();
    animation->setStartOffset(delay);
    return animation;
}

bool LayoutAnimationController::isDone()const{
    return AnimationUtils::currentAnimationTimeMillis() >
                mAnimation->getStartTime() + mMaxDelay + mDuration;
}

long LayoutAnimationController::getDelayForView(View* view){
    ViewGroup::LayoutParams* lp = view->getLayoutParams();
    AnimationParameters* params = lp->layoutAnimationParameters;

    if (params == nullptr) return 0;

    float delay = mDelay * (float)mAnimation->getDuration();
    long viewDelay = (long) (getTransformedIndex(params) * delay);
    float totalDelay = delay * params->count;

    if (mInterpolator == nullptr) {
        mInterpolator = new LinearInterpolator();
    }

    float normalizedDelay = viewDelay / totalDelay;
    normalizedDelay = mInterpolator->getInterpolation(normalizedDelay);
    LOGV("%p:%d totalDelay=%.2f %d/%d mDelay=%.2f dur=%d",view,view->getId(),totalDelay,params->index,params->count,mDelay,mAnimation->getDuration());
    return (long) (normalizedDelay * totalDelay);
}

int LayoutAnimationController::getTransformedIndex(const AnimationParameters* params){
    switch (getOrder()) {
    case ORDER_REVERSE: return params->count - 1 - params->index;
    case ORDER_RANDOM :
#if defined(__linux__)||defined(__unix__)
        return static_cast<int> (params->count * drand48());
#else
        {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> dis(0.0, 1.0);
            return static_cast<int>(params->count * dis(gen));
        }
#endif
    case ORDER_NORMAL : default: return params->index;
    }
}

}
