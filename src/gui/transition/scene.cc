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
#include <transition/scene.h>

#include <core/context.h>
#include <porting/cdlog.h>
#include <view/view.h>
#include <view/viewgroup.h>
#include <view/layoutinflater.h>
#include <widget/R.h>

namespace cdroid{

Scene::Scene(ViewGroup* sceneRoot)
    : mSceneRoot(sceneRoot){
}

Scene::Scene(ViewGroup* sceneRoot, View* layout)
    : mSceneRoot(sceneRoot), mLayout(layout){
}

Scene::Scene(ViewGroup* sceneRoot, ViewGroup* layout) // deprecated
    : mSceneRoot(sceneRoot), mLayout(layout){
}

Scene::Scene(ViewGroup* sceneRoot, int layoutId, Context* context)
    : mContext(context), mLayoutId(layoutId), mSceneRoot(sceneRoot){
}

Scene::Scene(ViewGroup* sceneRoot, const std::string& layoutResource, Context* context)
    : mContext(context), mLayoutResource(layoutResource), mSceneRoot(sceneRoot){
}

Scene* Scene::getSceneForLayout(ViewGroup* sceneRoot, int layoutId, Context* context){
    SparseArray<Scene*>* scenes = static_cast<SparseArray<Scene*>*>(
            sceneRoot->getTag(R::id::scene_layoutid_cache));
    if (scenes == nullptr){
        scenes = new SparseArray<Scene*>();
        sceneRoot->setTag(R::id::scene_layoutid_cache, scenes);
    }
    Scene* scene = scenes->get(layoutId);
    if (scene != nullptr){
        return scene;
    }
    scene = new Scene(sceneRoot, layoutId, context);
    scenes->put(layoutId, scene);
    return scene;
}

Scene* Scene::getSceneForLayout(ViewGroup* sceneRoot, const std::string& layoutResource, Context* context){
    // CDROID resources are string-reference based, so this is the working overload.
    // Android caches getSceneForLayout results (keyed by int id on the sceneRoot); caching a
    // string-keyed map would need an extra tag id, so for now we return a fresh Scene per call
    // (enter() re-inflates). Ownership follows the same GC-equivalent convention as the rest of
    // the transition framework (borrowed, not freed by the caller).
    return new Scene(sceneRoot, layoutResource, context);
}

ViewGroup* Scene::getSceneRoot(){
    return mSceneRoot;
}

void Scene::exit(){
    if (getCurrentScene(mSceneRoot) == this){
        if (mExitAction){
            mExitAction();
        }
    }
}

void Scene::enter(){
    // Apply layout change, if any
    if (mLayoutId > 0 || mLayout != nullptr || !mLayoutResource.empty()){
        // empty out parent container before adding to it
        getSceneRoot()->removeAllViews();
        if (!mLayoutResource.empty()){
            // CDROID-idiomatic path: inflate by string resource ("cdroid:layout/..." / "@layout/...").
            LayoutInflater::from(mContext)->inflate(mLayoutResource, mSceneRoot, true);
        } else if (mLayoutId > 0){
            // Android int layoutId: CDROID has no runtime int→resource table, so this cannot
            // resolve. Use the string-resource overload of getSceneForLayout instead.
            LOGW("Scene::enter: int layoutId inflation not supported (mLayoutId=%d); use the "
                 "string-resource getSceneForLayout overload", mLayoutId);
        } else {
            mSceneRoot->addView(mLayout);
        }
    }
    // Notify next scene that it is entering. Subclasses may override to configure scene.
    if (mEnterAction){
        mEnterAction();
    }
    setCurrentScene(mSceneRoot, this);
}

void Scene::setCurrentScene(ViewGroup* sceneRoot, Scene* scene){
    sceneRoot->setTag(R::id::current_scene, scene);
}

Scene* Scene::getCurrentScene(ViewGroup* sceneRoot){
    return static_cast<Scene*>(sceneRoot->getTag(R::id::current_scene));
}

void Scene::setEnterAction(const Runnable& action){
    mEnterAction = action;
}

void Scene::setExitAction(const Runnable& action){
    mExitAction = action;
}

} // namespace cdroid
