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
#include <view/viewgroup.h>
#include <view/viewoverlay.h>
#include <view/viewgroupoverlay.h>
#include <cdlog.h>
namespace cdroid{
class ViewOverlay::OverlayViewGroup:public ViewGroup{
protected:
    View* mHostView;
    std::vector<Drawable*>mDrawables;
protected:
    bool verifyDrawable(Drawable* who)const override;
    void dispatchDraw(Canvas& canvas)override;
    void onLayout(bool changed, int l, int t, int w, int h)override;
    void invalidateViewProperty(bool invalidateParent, bool forceRedraw)override;
    void invalidateParentCaches()override;
    void invalidateParentIfNeeded()override;
public:
    OverlayViewGroup(Context*context,View* hostView);
    ~OverlayViewGroup()override;
    void add(Drawable* drawable);
    void remove(Drawable* drawable);
    void add(View* child);
    void remove(View* view);
    void clear();
    bool isEmpty()const;
    void invalidateDrawable(Drawable& drawable)override;
    void invalidate(const Rect& dirty)override;
    void invalidate(int l, int t, int w, int h)override;
    void invalidate(bool invalidateCache)override;
    void onDescendantInvalidated(View* child,View* target)override;
    ViewGroup* invalidateChildInParent(int* location, Rect& dirty)override;
};

ViewOverlay::ViewOverlay(Context* context, View* hostView){
    mOverlayViewGroup = new OverlayViewGroup(context, hostView);
}

ViewOverlay::~ViewOverlay(){
    delete mOverlayViewGroup;
}

ViewGroup* ViewOverlay::getOverlayView()const{
    return mOverlayViewGroup;
}

void ViewOverlay::add(Drawable* drawable) {
    mOverlayViewGroup->add(drawable);
}

void ViewOverlay::remove(Drawable* drawable) {
    mOverlayViewGroup->remove(drawable);
}

void ViewOverlay::clear() {
    mOverlayViewGroup->clear();
}

bool ViewOverlay::isEmpty()const{
    return mOverlayViewGroup->isEmpty();
}

///////////////////////////////////////////////////////////////////////////////////

ViewGroupOverlay::ViewGroupOverlay(Context* context, View* hostView)
    :ViewOverlay(context,hostView){
}

void ViewGroupOverlay::add(View*view){
    mOverlayViewGroup->add(view);
}

void ViewGroupOverlay::remove(View*view){
    mOverlayViewGroup->remove(view);
}

///////////////////////////////////////////////////////////////////////////////////

ViewOverlay::OverlayViewGroup::OverlayViewGroup(Context*context,View* hostView):ViewGroup(0,0){
    mHostView = hostView;
}

ViewOverlay::OverlayViewGroup::~OverlayViewGroup(){
    for(Drawable*dr:mDrawables){
        delete dr;
    }
    mDrawables.clear();
}

void ViewOverlay::OverlayViewGroup::add(Drawable* drawable){
    if (drawable == nullptr) {
        throw std::runtime_error("drawable must be non-null");
    }
    if (std::find(mDrawables.begin(),mDrawables.end(),drawable)==mDrawables.end()) {
        // Make each drawable unique in the overlay; can't add it more than once
        mDrawables.push_back(drawable);
        invalidate(drawable->getBounds());
        drawable->setCallback(this);
    }
}

void ViewOverlay::OverlayViewGroup::remove(Drawable* drawable){
    if (drawable == nullptr) {
        throw std::runtime_error("drawable must be non-null");
    }
    auto it=std::find(mDrawables.begin(),mDrawables.end(),drawable);
    if (it!=mDrawables.end()) {
        invalidate(drawable->getBounds());
        drawable->setCallback(nullptr);
        mDrawables.erase(it);
        delete drawable;
    }
}

bool ViewOverlay::OverlayViewGroup::verifyDrawable(Drawable* who)const{
    auto it = std::find(mDrawables.begin(),mDrawables.end(),who);
    return ViewGroup::verifyDrawable(who) || (it!=mDrawables.end());
}

void ViewOverlay::OverlayViewGroup::add(View* child) {
    if (child == nullptr) {
        throw std::runtime_error("view must be non-null");
    }

    if (child->getParent()) {
        ViewGroup* parent = child->getParent();
        if ((parent != mHostView) && parent->getParent() && parent->isAttachedToWindow()) {
            // Moving to different container; figure out how to position child such that
            // it is in the same location on the screen
            int parentLocation[2];
            int hostViewLocation[2];
            parent->getLocationOnScreen(parentLocation);
            mHostView->getLocationOnScreen(hostViewLocation);
            child->offsetLeftAndRight(parentLocation[0] - hostViewLocation[0]);
            child->offsetTopAndBottom(parentLocation[1] - hostViewLocation[1]);
        }
        parent->removeView(child);
        if (parent->getLayoutTransition() != nullptr) {
            // LayoutTransition will cause the child to delay removal - cancel it
            parent->getLayoutTransition()->cancel(LayoutTransition::DISAPPEARING);
        }
        // fail-safe if view is still attached for any reason
        /*if (child->getParent() != nullptr) {
            //child->assignParent(nullptr);
            //child->mParent has been setted to nullptr in parent->removeView(child)
        }*/
    }
    ViewGroup::addView(child);
}

void ViewOverlay::OverlayViewGroup::onDescendantInvalidated(View* child,View* target){
    if (mHostView) {
        if (dynamic_cast<ViewGroup*>(mHostView)) {
            // Propagate invalidate through the host...
            ((ViewGroup*) mHostView)->onDescendantInvalidated(mHostView, target);

            // ...and also this view, since it will hold the descendant, and must later
            // propagate the calls to update display lists if dirty
            ViewGroup::onDescendantInvalidated(child, target);
        } else {
            // Can't use onDescendantInvalidated because host isn't a ViewGroup - fall back
            // to invalidating.
            invalidate(true);
        }
    }
}

void ViewOverlay::OverlayViewGroup::remove(View*view){
    ViewGroup::removeView(view);
}

void ViewOverlay::OverlayViewGroup::clear() {
    removeAllViews();
    for (Drawable* drawable : mDrawables) {
        drawable->setCallback(nullptr);
        delete drawable;
    }
    mDrawables.clear();
}

bool ViewOverlay::OverlayViewGroup::isEmpty()const{
    return (getChildCount() == 0) && (mDrawables.size() == 0);
}

void ViewOverlay::OverlayViewGroup::invalidateDrawable(Drawable& drawable) {
    invalidate(drawable.getBounds());
}

void ViewOverlay::OverlayViewGroup::dispatchDraw(Canvas& canvas) {
    /*
     * The OverlayViewGroup doesn't draw with a DisplayList, because
     * draw(Canvas, View, long) is never called on it. This is fine, since it doesn't need
     * RenderNode/DisplayList features, and can just draw into the owner's Canvas.
     *
     * This means that we need to insert reorder barriers manually though, so that children
     * of the OverlayViewGroup can cast shadows and Z reorder with each other.
     */
     //canvas.insertReorderBarrier();

    ViewGroup::dispatchDraw(canvas);

    //canvas.insertInorderBarrier();
    for (Drawable*d:mDrawables) {
        d->draw(canvas);
    }
}

void ViewOverlay::OverlayViewGroup::onLayout(bool changed, int l, int t, int w, int h) {
    // Noop: children are positioned absolutely
}

void ViewOverlay::OverlayViewGroup::invalidate(const Rect& dirty) {
    ViewGroup::invalidate(dirty);
    if (mHostView) {
        mHostView->invalidate(dirty);
    }
}

void ViewOverlay::OverlayViewGroup::invalidate(int l, int t, int w, int h) {
    ViewGroup::invalidate(l, t, w, h);
    if (mHostView) {
        mHostView->invalidate(l, t, w, h);
    }
}

void ViewOverlay::OverlayViewGroup::invalidate(bool invalidateCache) {
    ViewGroup::invalidate(invalidateCache);
    if (mHostView) {
        mHostView->invalidate(invalidateCache);
    }
}

void ViewOverlay::OverlayViewGroup::invalidateViewProperty(bool invalidateParent, bool forceRedraw) {
    ViewGroup::invalidateViewProperty(invalidateParent, forceRedraw);
    if (mHostView) {
        mHostView->invalidateViewProperty(invalidateParent, forceRedraw);
    }
}

void ViewOverlay::OverlayViewGroup::invalidateParentCaches() {
    ViewGroup::invalidateParentCaches();
    if (mHostView) {
        mHostView->invalidateParentCaches();
    }
}

void ViewOverlay::OverlayViewGroup::invalidateParentIfNeeded() {
    ViewGroup::invalidateParentIfNeeded();
    if (mHostView) {
        mHostView->invalidateParentIfNeeded();
    }
}

ViewGroup* ViewOverlay::OverlayViewGroup::invalidateChildInParent(int* location,Rect& dirty) {
    if (mHostView) {
        dirty.offset(location[0], location[1]);
        if (dynamic_cast<ViewGroup*>(mHostView)) {
            location[0] = 0;
            location[1] = 0;
            ViewGroup::invalidateChildInParent(location, dirty);
            return ((ViewGroup*) mHostView)->invalidateChildInParent(location, dirty);
        } else {
            invalidate(dirty);
        }
    }
    return nullptr;
}

}
