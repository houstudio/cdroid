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
#ifndef __GRAPH_DEVICE_H__
#define __GRAPH_DEVICE_H__
#include <core/rect.h>
#include <cairomm/region.h>   // Cairo::Region (computeVisibleRegion)
#include <cairomm/context.h>
#include <mutex>
#include <condition_variable>
#include <map>

#ifndef COMPOSE_ASYNC
#define COMPOSE_ASYNC 0
#endif

namespace cdroid{
class Animator;  // drives a ghost's exit animation (freed by removeGhost, off-dispatch)

class GraphDevice{
public:
    // A compositor-owned "ghost": the snapshot surface of a window that has
    // already been REMOVED, still playing its exit animation. The analog of
    // AOSP WindowStateAnimator's surface outliving the view tree during window
    // exit animations — the view teardown is synchronous (ViewRootImpl.die ->
    // dispatchDetachedFromWindow); only the visual lingers, owned by the
    // compositor alone. No view tree, no input, no focus behind it.
    class GhostLayer {
    public:
        Cairo::RefPtr<Cairo::ImageSurface> snapshot; // window content at removal
        Rect bounds;         // resting bounds at removal (screen space)
        float alpha = 1.f;   // whole-surface fade (FADE exit)
        int dx = 0, dy = 0;  // compose-time translation (SLIDE exit)
        Rect lastRect;       // bounds at the previous compose (swept damage)
        Animator* animator = nullptr; // drives alpha/dx/dy; freed by removeGhost
    };
private:
    int mScreenWidth;
    int mScreenHeight;
    int mFormat;
    int mComposing;
    int mPendingCompose;
    int mRotation;
    bool mQuitFlag;
    bool mShowFPS;
    uint64_t mLastComposeTime;
    uint64_t mFpsStartTime;
    uint64_t mFpsPrevTime;
    uint64_t mFpsNumFrames;
    Rect mRectBanner;
    std::mutex mMutex;
    std::condition_variable mCV;
    std::string mFPSText;
    std::string mLogo;
    void* mPrimarySurface;
    class Canvas*mPrimaryContext;
    std::vector<GhostLayer*> mGhosts;
    GraphDevice();
    void trackFPS(Canvas&);
    void doCompose();
    void computeVisibleRegion(std::vector<class Window*>&windows,std::vector<Cairo::RefPtr<Cairo::Region>>&regions);
    void rotateRectInWindow(const Rect&rcw,const Rect&rs,Rect&rd,int&dx,int&dy,int rotation);
    void showLogo(Cairo::Context*,Cairo::RefPtr<Cairo::ImageSurface>);
public:
    static GraphDevice&getInstance();
    ~GraphDevice();
    GraphDevice& setFormat(int format);
    GraphDevice& setLogo(const std::string&);
    GraphDevice& setRotation(int rotation);
    GraphDevice& showFPS(bool);
    int init();
    void getScreenSize(int &w,int&h)const;
    int getScreenWidth()const;
    int getScreenHeight()const;
    void flip();
    void requestCompose();
    void lock();
    void unlock();
    void composeSurfaces();
    bool needCompose()const;
    GhostLayer* addGhost(const Cairo::RefPtr<Cairo::ImageSurface>& snap, const Rect& bounds);
    void removeGhost(GhostLayer* ghost);
    void clearGhosts();
    void composeGhosts();
    Canvas*getPrimaryContext();
    void* getPrimarySurface()const;
};
}
#endif

