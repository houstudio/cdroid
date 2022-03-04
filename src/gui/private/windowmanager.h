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
#include <core/graph_device.h>
#include <vector>
#include <stdint.h>
#include <queue>
#include <unordered_set>

namespace cdroid {

class WindowManager {
public:
    DECLARE_UIEVENT(bool,WNDENUMPROC,Window*);
    static WindowManager& getInstance();
    void addWindow(Window*w);
    void removeWindow(Window*w);
    void moveWindow(Window*w,int x,int y);
    void processEvent(InputEvent&e);
    void clip(Window*win);
    void resetVisibleRegion();
    int enumWindows(WNDENUMPROC cbk);
    int getWindows(std::vector<Window*>&);
    int getVisibleWindows(std::vector<Window*>&);
    void shutDown(){delete instance;}
protected:
    virtual void onKeyEvent(KeyEvent&key);
    virtual void onMotion(MotionEvent&event);
private:
    friend class GraphDevice;
    WindowManager();
    virtual ~WindowManager();
    Window*activeWindow;/*activeWindow*/
    std::vector< Window* > windows;
    static WindowManager* instance;
    DISALLOW_COPY_AND_ASSIGN(WindowManager);
};

}  // namespace cdroid

#endif  // __CDROID WINDOWMANAGER_H__
