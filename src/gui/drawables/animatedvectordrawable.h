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
#ifndef __ANIMATED_VECTOR_DRAWABLE_H__
#define __ANIMATED_VECTOR_DRAWABLE_H__
#include <drawables/vectordrawable.h>
#include <animation/objectanimator.h>
namespace cdroid{
namespace hwui{
    class PropertyValuesAnimatorSet;
};
class AnimatedVectorDrawable: public Drawable{// implements Animatable2 {
public:
    class AnimatedVectorDrawableState;
private:
    static constexpr const char* ANIMATED_VECTOR = "animated-vector";
    static constexpr const char* TARGET = "target";
    static constexpr bool DBG_ANIMATION_VECTOR_DRAWABLE = false;
    class VectorDrawableAnimator;
    class VectorDrawableAnimatorUI;
    class VectorDrawableAnimatorRT;
    /** Local, mutable animator set. */
    VectorDrawableAnimator* mAnimatorSet;
    std::shared_ptr<AnimatedVectorDrawableState> mAnimatedVectorState;
    Drawable::Callback* mCallback;
    /** The animator set that is parsed from the xml. */
    AnimatorSet* mAnimatorSetFromXml = nullptr;

    bool mMutated;

    /** Use a internal AnimatorListener to support callbacks during animation events. */
    std::vector<Animatable2::AnimationCallback> mAnimationCallbacks;
    Animator::AnimatorListener mAnimatorListener;
private:
    static bool shouldIgnoreInvalidAnimation();
    static void updateAnimatorProperty(Animator* animator, const std::string& targetName,VectorDrawable* vectorDrawable, bool ignoreInvalidAnim);
    static bool containsSameValueType(const PropertyValuesHolder* holder,const Property* property);
    void fallbackOntoUI();
protected:
    void onBoundsChange(const Rect& bounds) override;
    bool onStateChange(const std::vector<int>& state)override;
    bool onLevelChange(int level)override;
public:
    AnimatedVectorDrawable();
    AnimatedVectorDrawable(std::shared_ptr<AnimatedVectorDrawableState> state);
    ~AnimatedVectorDrawable()override;
    AnimatedVectorDrawable*mutate()override;
    void clearMutated()override;
    std::shared_ptr<Drawable::ConstantState> getConstantState()override;
    int getChangingConfigurations()const override;
    void draw(Canvas&)override;
    bool onLayoutDirectionChanged(int layoutDirection)override;
    int getAlpha() const override;
    void setAlpha(int alpha)override;
    void setColorFilter(ColorFilter* colorFilter)override;
    ColorFilter* getColorFilter()override;
    void setTintList(const ColorStateList* tint)override;
    void setHotspot(float x, float y)override;
    void setHotspotBounds(int left, int top, int right, int bottom)override;
    void setTintMode(int tintMode)override;
    bool setVisible(bool visible, bool restart)override;
    bool isStateful() const override;
    int getOpacity()override;
    int getIntrinsicWidth()override;
    int getIntrinsicHeight()override;
    void getOutline(Outline& outline)override;
    Insets getOpticalInsets()override;
    void inflate(XmlPullParser&,const AttributeSet&)override;
    void forceAnimationOnUI();
    bool canApplyTheme()override;
    bool isRunning();
    void reset();
    void start();
    void ensureAnimatorSet();
    void stop();
    void reverse();
    bool canReverse();
    void registerAnimationCallback(const Animatable2::AnimationCallback& callback);
    bool unregisterAnimationCallback(const Animatable2::AnimationCallback& callback);
    void removeAnimatorSetListener();
    void clearAnimationCallbacks();
};

class AnimatedVectorDrawable::AnimatedVectorDrawableState:public ConstantState ,
    public std::enable_shared_from_this<AnimatedVectorDrawableState>{
private:
    friend AnimatedVectorDrawable;
    class PendingAnimator {
    public:
        std::string animResId;
        float pathErrorScale;
        std::string target;
        PendingAnimator(const std::string& animResId, float pathErrorScale, const std::string& target);
        Animator* newInstance(Context*);
    };
    int mChangingConfigurations;
    bool mShouldIgnoreInvalidAnim;
    Context*mContext;
    VectorDrawable* mVectorDrawable;
    /** Animators that require a theme before inflation. */
    std::vector<PendingAnimator*> mPendingAnims;
    /** Fully inflated animators awaiting cloning into an AnimatorSet. */
    std::vector<Animator*> mAnimators;
    /** Map of animators to their target object names */
    std::unordered_map<Animator*, std::string> mTargetNameMap;
public:
    AnimatedVectorDrawableState(std::shared_ptr<AnimatedVectorDrawableState>copy,Callback* owner);
    ~AnimatedVectorDrawableState()override;
    bool canApplyTheme();
    Drawable*newDrawable()override;
    int getChangingConfigurations() const override;
    void addPendingAnimator(const std::string& resId, float pathErrorScale, const std::string& target);
    void addTargetAnimator(const std::string& targetName, Animator* animator);
    void prepareLocalAnimators(AnimatorSet* animatorSet);
    Animator*prepareLocalAnimator(int index);
    void inflatePendingAnimators(/*Resources res,Theme t*/);
};

class AnimatedVectorDrawable::VectorDrawableAnimator {
public:
    virtual ~VectorDrawableAnimator()=default;
    virtual void init(AnimatorSet* set)=0;
	virtual void start()=0;
	virtual void end()=0;
	virtual void reset()=0;
	virtual void reverse()=0;
	virtual bool canReverse()=0;
	virtual void setListener(const Animator::AnimatorListener& listener)=0;
	virtual void removeListener(const Animator::AnimatorListener& listener)=0;
	virtual void onDraw(Canvas& canvas)=0;
	virtual bool isStarted()=0;
	virtual bool isRunning()=0;
	virtual bool isInfinite()=0;
	virtual void pause()=0;
	virtual void resume()=0;
};

class AnimatedVectorDrawable::VectorDrawableAnimatorUI:public VectorDrawableAnimator {
private:
    friend AnimatedVectorDrawable;
    AnimatorSet* mSet = nullptr;
    Drawable* mDrawable;
    // Caching the listener in the case when listener operation is called before the mSet is
    // setup by init().
    std::vector<Animator::AnimatorListener> mListenerArray;
    bool mIsInfinite = false;
private:
    void invalidateOwningView();
public:
    VectorDrawableAnimatorUI(AnimatedVectorDrawable* drawable);
    ~VectorDrawableAnimatorUI()override;
    void init(AnimatorSet* set)override;
    void start()override;
    void end()override;

    void reset()override;
    void reverse()override;
    bool canReverse()override;
    void setListener(const Animator::AnimatorListener& listener)override;
    void removeListener(const Animator::AnimatorListener& listener)override;
    void onDraw(Canvas& canvas)override;

    bool isStarted()override;
    bool isRunning()override;
    bool isInfinite()override;
    void pause()override;
    void resume()override;
};
#ifdef ENABLE_VECTOR_RENDER_THREAD
//It is now unusable in cdroid disable it 
class AnimatedVectorDrawable::VectorDrawableAnimatorRT:public VectorDrawableAnimator {
private:
    static constexpr int START_ANIMATION = 1;
    static constexpr int REVERSE_ANIMATION = 2;
    static constexpr int RESET_ANIMATION = 3;
    static constexpr int END_ANIMATION = 4;
    friend AnimatedVectorDrawable;
    // If the duration of an animation is more than 300 frames, we cap the sample size to 300.
    static constexpr int MAX_SAMPLE_POINTS = 300;
    Animator::AnimatorListener mListener;
    std::vector<int64_t> mStartDelays;
    PropertyValuesHolder::PropertyValues mTmpValues;// =  new PropertyValuesHolder.PropertyValues();
    hwui::PropertyValuesAnimatorSet* mSetPtr = nullptr;
    bool mContainsSequentialAnimators = false;
    bool mStarted = false;
    bool mInitialized = false;
    bool mIsReversible = false;
    bool mIsInfinite = false;
    // TODO: Consider using NativeAllocationRegistery to track native allocation
    hwui::Tree*/*VirtualRefBasePtr*/ mSetRefBasePtr;
    //WeakReference<RenderNode> mLastSeenTarget = null;
    int mLastListenerId = 0;
    std::vector<int> mPendingAnimationActions;
    AnimatedVectorDrawable* mDrawable;
private:
    void parseAnimatorSet(AnimatorSet* set, int64_t startTime);
    void createRTAnimator(ObjectAnimator* animator, int64_t startTime);
    void createRTAnimatorForGroup(const std::vector<PropertyValuesHolder*>&values,ObjectAnimator* animator, VectorDrawable::VGroup* target,int64_t startTime);
    void createRTAnimatorForPath( ObjectAnimator* animator, VectorDrawable::VPath* target,int64_t startTime);

    void createRTAnimatorForFullPath(ObjectAnimator* animator,VectorDrawable::VFullPath* target, int64_t startTime);

    void createRTAnimatorForRootGroup(const std::vector<PropertyValuesHolder*>& values,ObjectAnimator* animator, VectorDrawable::VectorDrawableState* target,int64_t startTime);
    static int getFrameCount(int64_t duration);

    static std::vector<float> createFloatDataPoints(PropertyValuesHolder::PropertyValues::DataSource dataSource, int64_t duration);
    static std::vector<int> createIntDataPoints(PropertyValuesHolder::PropertyValues::DataSource dataSource, int64_t duration);
    void createNativeChildAnimator(PropertyValuesHolder* holder, int64_t extraDelay,ObjectAnimator* animator);
    
    void handlePendingAction(int pendingAnimationAction);
    bool useLastSeenTarget();
    //bool useTarget(RenderNode target);

    void invalidateOwningView() ;
    void addPendingAction(int pendingAnimationAction);
    void startAnimation();
    void endAnimation();
    void resetAnimation();
    void reverseAnimation();
    void onAnimationEnd(int listenerId);

    // onFinished: should be called from native
    static void callOnFinished(VectorDrawableAnimatorRT* set, int id);
    void transferPendingActions(VectorDrawableAnimator* animatorSet);
protected:
    //void recordLastSeenTarget(DisplayListCanvas canvas);
public:
    VectorDrawableAnimatorRT(AnimatedVectorDrawable* drawable);
    ~VectorDrawableAnimatorRT()override;
    void init(AnimatorSet* set)override;
    void start()override;
    void end()override;
    void reset()override;
    void reverse()override;
    long getAnimatorNativePtr();
    bool canReverse()override;
    bool isStarted()override;
    bool isRunning()override;
    void setListener(const Animator::AnimatorListener& listener);
    void removeListener(const Animator::AnimatorListener& listener);
    void onDraw(Canvas& canvas);
    bool isInfinite();
    void pause()override;
    void resume()override;
};
#endif
}/*endof namespace*/
#endif/*__ANIMATED_VECTOR_DRAWABLE_H__*/
