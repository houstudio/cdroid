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
#include <string>

namespace cdroid {

class Context;
class View;
class ViewGroup;

/**
 * A scene represents the collection of values that various properties in the View
 * hierarchy will have when the scene is applied. Ported from android-36 android.transition.Scene.
 *
 * Deviation: android's getSceneForLayout takes an int R.layout.* id. CDROID's resource system
 * is string-reference based ("@layout/..." / "cdroid:layout/...") with no runtime int→resource
 * table, so the int overload cannot inflate (left as a LOGW no-op for API fidelity). Use the
 * string-resource overload getSceneForLayout(sceneRoot, const std::string&, context) instead —
 * it inflates via LayoutInflater on enter(). The View-based and enter-action constructors — the
 * ones used by TransitionManager.beginDelayedTransition — work fully.
 */
class Scene {
  public:
    explicit Scene(ViewGroup* sceneRoot);
    Scene(ViewGroup* sceneRoot, View* layout);
    Scene(ViewGroup* sceneRoot, ViewGroup* layout); // deprecated alias of (sceneRoot, View*)
    static Scene* getSceneForLayout(ViewGroup* sceneRoot, int layoutId, Context* context);
    /** CDROID-idiomatic: inflate a layout by string resource on enter() (e.g. "cdroid:layout/foo"). */
    static Scene* getSceneForLayout(ViewGroup* sceneRoot, const std::string& layoutResource, Context* context);

    ViewGroup* getSceneRoot();
    void exit();
    void enter();

    static void setCurrentScene(ViewGroup* sceneRoot, Scene* scene);
    static Scene* getCurrentScene(ViewGroup* sceneRoot);

    void setEnterAction(const Runnable& action);
    void setExitAction(const Runnable& action);

    bool isCreatedFromLayoutResource() const {
        return mLayoutId > 0 || !mLayoutResource.empty();
    }

  private:
    // Private; layoutId-based scenes are created by the getSceneForLayout cache factory.
    Scene(ViewGroup* sceneRoot, int layoutId, Context* context);
    Scene(ViewGroup* sceneRoot, const std::string& layoutResource, Context* context);

    Context*   mContext = nullptr;
    int        mLayoutId = -1;
    std::string mLayoutResource;   // string layout resource (CDROID); inflated on enter()
    ViewGroup* mSceneRoot;
    View*      mLayout = nullptr;
    Runnable   mEnterAction;
    Runnable   mExitAction;
};

} // namespace cdroid
#endif // __CDROID_TRANSITION_SCENE_H__
