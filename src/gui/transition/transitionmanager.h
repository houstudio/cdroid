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
#ifndef __CDROID_TRANSITION_TRANSITIONMANAGER_H__
#define __CDROID_TRANSITION_TRANSITIONMANAGER_H__

#include <vector>

#include <transition/arraymap.h>
#include <transition/scene.h>
#include <transition/transition.h>

namespace cdroid{

/**
 * Manages the set of transitions that fire on a change of Scene. Ported from
 * android-36 android.transition.TransitionManager.
 *
 * Entry points: beginDelayedTransition(sceneRoot[, transition]) captures start values,
 * then on the next frame (ViewTreeObserver.OnPreDraw) captures end values and plays the
 * transition. go(Scene[, transition]) and transitionTo(Scene) drive scene-based changes.
 *
 * Deviation: the default transition is android's AutoTransition (a TransitionSet of
 * Fade+ChangeBounds). TransitionSet is not yet ported, so the default is currently Fade;
 * it will switch to AutoTransition once TransitionSet lands.
 */
class TransitionManager{
public:
    void setDefaultTransition(Transition* transition);
    static Transition* getDefaultTransition();

    void setTransition(Scene* scene, Transition* transition);
    void setTransition(Scene* fromScene, Scene* toScene, Transition* transition);
    Transition* getTransition(Scene* scene);

    void transitionTo(Scene* scene);

    static void go(Scene* scene);
    static void go(Scene* scene, Transition* transition);

    static void beginDelayedTransition(ViewGroup* sceneRoot);
    static void beginDelayedTransition(ViewGroup* sceneRoot, Transition* transition);

    static void endTransitions(ViewGroup* sceneRoot);

    // android: package-private static state (ThreadLocal<WeakReference<...>> + static list).
    // CDROID runs transitions on the UI thread, so these are process-local statics. Public
    // so the file-local MultiListener (not a member/friend) can access them, matching
    // android's same-package access.
    static ArrayMap<ViewGroup*, std::vector<Transition*>>& getRunningTransitions();
    static std::vector<ViewGroup*>& getPendingTransitions();

private:
    static void changeScene(Scene* scene, Transition* transition);
    static void sceneChangeSetup(ViewGroup* sceneRoot, Transition* transition);
    static void sceneChangeRunTransition(ViewGroup* sceneRoot, Transition* transition);

    ArrayMap<Scene*, Transition*> mSceneTransitions;
    ArrayMap<Scene*, ArrayMap<Scene*, Transition*>> mScenePairTransitions;
};

} // namespace cdroid
#endif // __CDROID_TRANSITION_TRANSITIONMANAGER_H__
