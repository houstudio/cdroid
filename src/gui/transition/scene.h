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
#ifndef __CDROID_TRANSITION_SCENE_H__
#define __CDROID_TRANSITION_SCENE_H__

#include <core/callbackbase.h> // Runnable = CallbackBase<void>
#include <core/sparsearray.h>  // SparseArray (SparseArrayImpl<int,T>)

namespace cdroid{

class Context;
class View;
class ViewGroup;

/**
 * A scene represents the collection of values that various properties in the View
 * hierarchy will have when the scene is applied. Ported from android-36 android.transition.Scene.
 *
 * Deviation: android's layoutId-based scenes inflate by int resource id. CDROID's resource
 * system is string-reference based ("@layout/...") with no full int→resource table, so the
 * layoutId inflation path is deferred (TODO until a resource-id table exists). The View-based
 * and enter-action constructors — the ones used by TransitionManager.beginDelayedTransition —
 * work fully.
 */
class Scene{
public:
    explicit Scene(ViewGroup* sceneRoot);
    Scene(ViewGroup* sceneRoot, View* layout);
    Scene(ViewGroup* sceneRoot, ViewGroup* layout); // deprecated alias of (sceneRoot, View*)
    static Scene* getSceneForLayout(ViewGroup* sceneRoot, int layoutId, Context* context);

    ViewGroup* getSceneRoot();
    void exit();
    void enter();

    static void setCurrentScene(ViewGroup* sceneRoot, Scene* scene);
    static Scene* getCurrentScene(ViewGroup* sceneRoot);

    void setEnterAction(const Runnable& action);
    void setExitAction(const Runnable& action);

    bool isCreatedFromLayoutResource() const{ return mLayoutId > 0; }

private:
    // Private; layoutId-based scenes are created by the getSceneForLayout cache factory.
    Scene(ViewGroup* sceneRoot, int layoutId, Context* context);

    Context*   mContext = nullptr;
    int        mLayoutId = -1;
    ViewGroup* mSceneRoot;
    View*      mLayout = nullptr;
    Runnable   mEnterAction;
    Runnable   mExitAction;
};

} // namespace cdroid
#endif // __CDROID_TRANSITION_SCENE_H__
