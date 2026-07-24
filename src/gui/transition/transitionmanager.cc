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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
 *********************************************************************************/
#include <transition/transitionmanager.h>

#include <algorithm>
#include <memory>

#include <porting/cdlog.h>
#include <view/view.h>
#include <view/viewgroup.h>
#include <view/viewtreeobserver.h>

#include <transition/fade.h>
#include <transition/autotransition.h>
#include <transition/transitionlisteneradapter.h>

namespace cdroid{

namespace {

// ---- file-local statics (android: static fields; single UI thread → process-local) ----
ArrayMap<ViewGroup*, std::vector<Transition*>>& sRunningTransitionsRef(){
    static ArrayMap<ViewGroup*, std::vector<Transition*>> sRunningTransitions;
    return sRunningTransitions;
}
std::vector<ViewGroup*>& sPendingTransitionsRef(){
    static std::vector<ViewGroup*> sPendingTransitions;
    return sPendingTransitions;
}
Transition*& sDefaultTransitionRef(){
    static Transition* sDefaultTransition = nullptr;
    return sDefaultTransition;
}

// android: private static nested MultiListener implementing OnPreDrawListener +
// OnAttachStateChangeListener. CDROID listener callbacks are callback-member (EventSet)
// values, not virtual overrides, so MultiListener stores an OnPreDrawListener and an
// OnAttachStateChangeListener and wires their callbacks to its methods.
//
// Lifetime: MultiListener is held alive by a strong shared_ptr in sActiveListeners while
// registered. The callbacks capture a weak_ptr, so they do not keep it alive (no cycle).
// removeListeners() drops the strong ref; the executing callback's local w.lock() keeps
// the object alive until the call returns, and the member callbacks (which own the
// executing Functor) are destroyed only when MultiListener is — after the call returns.
// android: anonymous TransitionListenerAdapter inside onPreDraw that removes the
// transition from the running list when it ends. Named here. Defined before
// MultiListener (MultiListener::onPreDraw news one up).
struct RunningEndListener: public TransitionListenerAdapter{
    ArrayMap<ViewGroup*, std::vector<Transition*>>* running;
    ViewGroup* sceneRoot;

    void onTransitionEnd(Transition& transition) override{
        std::vector<Transition*>* currentTransitions = running->get(sceneRoot);
        if (currentTransitions != nullptr){
            currentTransitions->erase(
                    std::remove(currentTransitions->begin(), currentTransitions->end(), &transition),
                    currentTransitions->end());
        }
        transition.removeListener(this);
    }
};

class MultiListener: public std::enable_shared_from_this<MultiListener>{
public:
    Transition* mTransition;
    ViewGroup* mSceneRoot;
    ViewTreeObserver* mViewTreeObserver;
    ViewTreeObserver::OnPreDrawListener mPreDraw;
    View::OnAttachStateChangeListener mAttach;

    static std::vector<std::shared_ptr<MultiListener>>& activeListeners(){
        static std::vector<std::shared_ptr<MultiListener>> sActiveListeners;
        return sActiveListeners;
    }

    MultiListener(Transition* transition, ViewGroup* sceneRoot)
        : mTransition(transition), mSceneRoot(sceneRoot),
          mViewTreeObserver(sceneRoot->getViewTreeObserver()){
    }

    void start(){
        std::weak_ptr<MultiListener> w = shared_from_this();
        mPreDraw = [w]()->bool{
            if (auto s = w.lock()){
                return s->onPreDraw();
            }
            return true;
        };
        mAttach.onViewAttachedToWindow = [](View&){};
        mAttach.onViewDetachedFromWindow = [w](View& v){
            if (auto s = w.lock()){
                s->onViewDetachedFromWindow(v);
            }
        };
        mSceneRoot->addOnAttachStateChangeListener(mAttach);
        mViewTreeObserver->addOnPreDrawListener(mPreDraw);
    }

    void removeListeners(){
        if (mViewTreeObserver->isAlive()){
            mViewTreeObserver->removeOnPreDrawListener(mPreDraw);
        } else {
            mSceneRoot->getViewTreeObserver()->removeOnPreDrawListener(mPreDraw);
        }
        mSceneRoot->removeOnAttachStateChangeListener(mAttach);
        auto& active = activeListeners();
        std::shared_ptr<MultiListener> self = shared_from_this();
        active.erase(std::remove(active.begin(), active.end(), self), active.end());
    }

    bool onPreDraw(){
        removeListeners();

        // Don't start the transition if it's no longer pending.
        auto& pending = TransitionManager::getPendingTransitions();
        auto pit = std::find(pending.begin(), pending.end(), mSceneRoot);
        if (pit == pending.end()){
            return true;
        }
        pending.erase(pit);

        // Add to running list, handle end to remove it
        ArrayMap<ViewGroup*, std::vector<Transition*>>& runningTransitions =
                TransitionManager::getRunningTransitions();
        std::vector<Transition*>* currentTransitions = runningTransitions.get(mSceneRoot);
        std::vector<Transition*> previousRunningTransitions;
        if (currentTransitions == nullptr){
            runningTransitions.put(mSceneRoot, std::vector<Transition*>());
            currentTransitions = runningTransitions.get(mSceneRoot);
        } else if (!currentTransitions->empty()){
            previousRunningTransitions = *currentTransitions;
        }
        currentTransitions->push_back(mTransition);
        RunningEndListener* endListener = new RunningEndListener();
        endListener->running = &runningTransitions;
        endListener->sceneRoot = mSceneRoot;
        mTransition->addListener(endListener);
        mTransition->captureValues(mSceneRoot, false);
        if (!previousRunningTransitions.empty()){
            for (Transition* runningTransition : previousRunningTransitions){
                runningTransition->resume(mSceneRoot);
            }
        }
        mTransition->playTransition(mSceneRoot);
        return true;
    }

    void onViewDetachedFromWindow(View& /*v*/){
        removeListeners();

        auto& pending = TransitionManager::getPendingTransitions();
        auto pit = std::find(pending.begin(), pending.end(), mSceneRoot);
        if (pit != pending.end()){
            pending.erase(pit);
        }

        std::vector<Transition*>* runningTransitions =
                TransitionManager::getRunningTransitions().get(mSceneRoot);
        if (runningTransitions != nullptr && !runningTransitions->empty()){
            std::vector<Transition*> copy = *runningTransitions;
            for (Transition* runningTransition : copy){
                runningTransition->resume(mSceneRoot);
            }
        }
        mTransition->clearValues(true);
    }
};

} // anonymous namespace

// ---- default transition ----
void TransitionManager::setDefaultTransition(Transition* transition){
    sDefaultTransitionRef() = transition;
}

Transition* TransitionManager::getDefaultTransition(){
    Transition*& t = sDefaultTransitionRef();
    if (t == nullptr){
        // android default: AutoTransition (sequential Fade OUT → ChangeBounds → Fade IN).
        t = new AutoTransition();
    }
    return t;
}

// ---- transition registration ----
void TransitionManager::setTransition(Scene* scene, Transition* transition){
    mSceneTransitions.put(scene, transition);
}

void TransitionManager::setTransition(Scene* fromScene, Scene* toScene, Transition* transition){
    ArrayMap<Scene*, Transition*>* sceneTransitionMap = mScenePairTransitions.get(toScene);
    if (sceneTransitionMap == nullptr){
        mScenePairTransitions.put(toScene, ArrayMap<Scene*, Transition*>());
        sceneTransitionMap = mScenePairTransitions.get(toScene);
    }
    sceneTransitionMap->put(fromScene, transition);
}

Transition* TransitionManager::getTransition(Scene* scene){
    Transition* transition = nullptr;
    ViewGroup* sceneRoot = scene->getSceneRoot();
    if (sceneRoot != nullptr){
        Scene* currScene = Scene::getCurrentScene(sceneRoot);
        if (currScene != nullptr){
            ArrayMap<Scene*, Transition*>* sceneTransitionMap = mScenePairTransitions.get(scene);
            if (sceneTransitionMap != nullptr){
                Transition** tp = sceneTransitionMap->get(currScene);
                if (tp != nullptr && *tp != nullptr){
                    return *tp;
                }
            }
        }
    }
    Transition** tp = mSceneTransitions.get(scene);
    transition = (tp != nullptr) ? *tp : nullptr;
    return (transition != nullptr) ? transition : getDefaultTransition();
}

// ---- scene change machinery ----
void TransitionManager::changeScene(Scene* scene, Transition* transition){
    ViewGroup* sceneRoot = scene->getSceneRoot();
    auto& pending = getPendingTransitions();
    if (std::find(pending.begin(), pending.end(), sceneRoot) != pending.end()){
        return;
    }
    Scene* oldScene = Scene::getCurrentScene(sceneRoot);
    if (transition == nullptr){
        if (oldScene != nullptr){
            oldScene->exit();
        }
        scene->enter();
    } else {
        pending.push_back(sceneRoot);
        Transition* transitionClone = transition->clone();
        transitionClone->setSceneRoot(sceneRoot);
        if (oldScene != nullptr && oldScene->isCreatedFromLayoutResource()){
            transitionClone->setCanRemoveViews(true);
        }
        sceneChangeSetup(sceneRoot, transitionClone);
        scene->enter();
        sceneChangeRunTransition(sceneRoot, transitionClone);
    }
}

void TransitionManager::sceneChangeSetup(ViewGroup* sceneRoot, Transition* transition){
    // Capture current values
    std::vector<Transition*>* runningTransitions = getRunningTransitions().get(sceneRoot);
    if (runningTransitions != nullptr && !runningTransitions->empty()){
        for (Transition* runningTransition : *runningTransitions){
            runningTransition->pause(sceneRoot);
        }
    }
    if (transition != nullptr){
        transition->captureValues(sceneRoot, true);
    }
    // Notify previous scene that it is being exited
    Scene* previousScene = Scene::getCurrentScene(sceneRoot);
    if (previousScene != nullptr){
        previousScene->exit();
    }
}

void TransitionManager::sceneChangeRunTransition(ViewGroup* sceneRoot, Transition* transition){
    if (transition != nullptr && sceneRoot != nullptr){
        std::shared_ptr<MultiListener> listener = std::make_shared<MultiListener>(transition, sceneRoot);
        MultiListener::activeListeners().push_back(listener);
        listener->start();
    }
}

void TransitionManager::transitionTo(Scene* scene){
    changeScene(scene, getTransition(scene));
}

void TransitionManager::go(Scene* scene){
    changeScene(scene, getDefaultTransition());
}

void TransitionManager::go(Scene* scene, Transition* transition){
    changeScene(scene, transition);
}

void TransitionManager::beginDelayedTransition(ViewGroup* sceneRoot){
    beginDelayedTransition(sceneRoot, nullptr);
}

void TransitionManager::beginDelayedTransition(ViewGroup* sceneRoot, Transition* transition){
    auto& pending = getPendingTransitions();
    if (std::find(pending.begin(), pending.end(), sceneRoot) == pending.end() && sceneRoot->isLaidOut()){
        if (Transition::DBG){
            LOGD("beginDelayedTransition: root=%p transition=%p", (void*)sceneRoot, (void*)transition);
        }
        pending.push_back(sceneRoot);
        if (transition == nullptr){
            transition = getDefaultTransition();
        }
        Transition* transitionClone = transition->clone();
        sceneChangeSetup(sceneRoot, transitionClone);
        Scene::setCurrentScene(sceneRoot, nullptr);
        sceneChangeRunTransition(sceneRoot, transitionClone);
    }
}

void TransitionManager::endTransitions(ViewGroup* sceneRoot){
    auto& pending = getPendingTransitions();
    pending.erase(std::remove(pending.begin(), pending.end(), sceneRoot), pending.end());

    std::vector<Transition*>* runningTransitions = getRunningTransitions().get(sceneRoot);
    if (runningTransitions != nullptr && !runningTransitions->empty()){
        // Copy in case this is called by an onTransitionEnd listener
        std::vector<Transition*> copy = *runningTransitions;
        for (int i = (int)copy.size() - 1; i >= 0; i--){
            copy[i]->forceToEnd(sceneRoot);
        }
    }
}

ArrayMap<ViewGroup*, std::vector<Transition*>>& TransitionManager::getRunningTransitions(){
    return sRunningTransitionsRef();
}

std::vector<ViewGroup*>& TransitionManager::getPendingTransitions(){
    return sPendingTransitionsRef();
}

} // namespace cdroid
