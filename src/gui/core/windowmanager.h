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
#ifndef __CDROID_WINDOWMANAGER_H__
#define __CDROID_WINDOWMANAGER_H__

#include <widget/cdwindow.h>
#include <core/display.h>
#include <vector>
#include <stdint.h>
#include <queue>
#include <unordered_set>

namespace cdroid {

class WindowManager {
private:
    int mDisplayRotation;
    Window*mActiveWindow;/*activeWindow*/
    std::vector< Window* > mWindows;
    Window* mHoveredWindow;
    std::vector< Display > mDisplays;
    static WindowManager* mInst;
private:
    friend class GraphDevice;
    WindowManager();
    virtual ~WindowManager();
public:
    DECLARE_UIEVENT(bool,WNDENUMPROC,Window*);
    class LayoutParams:public ViewGroup::LayoutParams{
    public:
        int type;
        int format;
        int x,y;
        int width,height;
        int gravity;
        int flags;
        int privateFlags;
    };
public:
    static WindowManager& getInstance();
    void setDisplayRotation(int display,int rotation);
    int  getDisplayRotation(int display=0)const;
    Display&getDefaultDisplay();
    Display*getDisplay(int display);
    void addWindow(Window*w);
    void removeWindow(Window*w);
    void removeWindows(const std::vector<Window*>&);
    void moveWindow(Window*w,int x,int y);
    void sendToBack(Window*w);
    void bringToFront(Window*w);
    void processEvent(InputEvent&e);
    void clip(Window*win);
    int enumWindows(WNDENUMPROC cbk);
    int getWindows(std::vector<Window*>&);
    int getVisibleWindows(std::vector<Window*>&);
    Window*getActiveWindow()const;
    void shutDown(){delete mInst;}
protected:
    virtual void onKeyEvent(KeyEvent&key);
    virtual void onMotion(MotionEvent&event);
    DISALLOW_COPY_AND_ASSIGN(WindowManager);
};

}  // namespace cdroid

#endif  // __CDROID WINDOWMANAGER_H__
