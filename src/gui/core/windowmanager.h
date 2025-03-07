/*
 * Copyright (C) 2015 UI project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
