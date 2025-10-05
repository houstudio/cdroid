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
#ifndef __ANIMATION_HANDLER__
#define __ANIMATION_HANDLER__
#include <unordered_map>
#include <core/looper.h>
#include <utils/neverdestroyed.h>
#include <view/choreographer.h>

namespace cdroid{
class ObjectAnimator;
class AnimationHandler{
public:
    class AnimationFrameCallback{
    public:
        virtual bool doAnimationFrame(int64_t)=0;
        virtual void commitAnimationFrame(int64_t)=0;
    };
    class AnimationFrameCallbackProvider {
    public:
        virtual ~AnimationFrameCallbackProvider()=default;
        virtual void postFrameCallback(const Choreographer::FrameCallback& callback)=0;
        virtual void postCommitCallback(Runnable& runnable)=0;
        virtual int64_t getFrameTime()=0;
        virtual long getFrameDelay()=0;
        virtual void setFrameDelay(long delay)=0;
    };
private:
    class MyFrameCallbackProvider :public AnimationFrameCallbackProvider {
    public:
        void postFrameCallback(const Choreographer::FrameCallback& callback)override;
        void postCommitCallback(Runnable& runnable)override;
        int64_t getFrameTime()override;
        long getFrameDelay()override;
        void setFrameDelay(long delay)override;
    };
    bool mListDirty;
    AnimationFrameCallbackProvider* mProvider;
    Choreographer::FrameCallback mFrameCallback;
    std::list<AnimationFrameCallback*> mAnimationCallbacks;
    std::list<AnimationFrameCallback*> mCommitCallbacks;
    std::unordered_map<AnimationFrameCallback* ,long>mDelayedCallbackStartTime;
    friend NeverDestroyed<AnimationHandler>;
    AnimationHandler();
private:
    AnimationFrameCallbackProvider* getProvider();
    ~AnimationHandler();
    void doFrame(int64_t);
    void doAnimationFrame(int64_t frameTime);
    bool isCallbackDue(AnimationFrameCallback* callback, int64_t currentTime);
    void commitAnimationFrame(AnimationFrameCallback* callback, int64_t frameTime);
    void cleanUpList();
    int getCallbackSize()const;
public:
    static AnimationHandler& getInstance();
    void setProvider(const AnimationFrameCallbackProvider* provider);
    void addAnimationFrameCallback(AnimationFrameCallback* callback, long delay);
    void addOneShotCommitCallback(AnimationFrameCallback* callback);
    void removeCallback(AnimationFrameCallback* callback);
    static int getAnimationCount();
    static void setFrameDelay(long delay);
    static long getFrameDelay();
    void autoCancelBasedOn(ObjectAnimator* objectAnimator);
};


}/*endof namespace*/
#endif
