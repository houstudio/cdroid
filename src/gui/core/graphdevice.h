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
#include <cairomm/context.h>
#include <mutex>
#include <condition_variable>
#include <map>

#ifndef COMPOSE_ASYNC
#define COMPOSE_ASYNC 0
#endif

namespace cdroid{
class GraphDevice{
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
    /* Mark a screen-space rectangle as dirty on every window whose bounds
     * intersect it (translated to that window's local coords). The next
     * composeSurfaces() blits those dirty regions, so an area uncovered by a
     * window that hid / moved / was removed is correctly redrawn from the
     * windows below. This is required by dirty-rect backends (xlib); full-flush
     * backends (DRM/FB) repaint everything anyway, so it's a harmless extra. */
    void invalidate(const Rect& screenRect);
    Canvas*getPrimaryContext();
    void* getPrimarySurface()const;
};
}
#endif

