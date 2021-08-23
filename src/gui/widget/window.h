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

#ifndef __NGLUI_WINDOW_H__
#define __NGLUI_WINDOW_H__
#include <cdtypes.h>
#include <widget/viewgroup.h>
#include <widget/framelayout.h>

namespace cdroid {
class Window : public ViewGroup {
    friend class WindowManager;
    friend class GraphDevice;
private:
    Runnable layoutRunner;
    RECT mRectOfFocusedView;
    void doLayout();
    bool performFocusNavigation(KeyEvent& event);
    static View*inflate(Context*ctx,std::istream&stream);
protected:
    Canvas*canvas;
    RefPtr<Region>mWindowRgn;	
    int window_type;/*window type*/
    int mLayer;/*surface layer*/
    std::string mText;
    class UIEventSource*source;
    void onFinishInflate()override;
    int processInputEvent(InputEvent&event);
    int processKeyEvent(KeyEvent&event);
    int processPointerEvent(MotionEvent&event);
    Canvas*getCanvas();
public:
    typedef enum{
        TYPE_WALLPAPER    =1,
        TYPE_APPLICATION  =2,
        TYPE_SYSTEM_WINDOW=2000,
        TYPE_STATUS_BAR   =2001,
        TYPE_SEARCH_BAR   =2002,
        TYPE_SYSTEM_ALERT =2003,
        TYPE_KEYGUARD     =2004,
        TYPE_TOAST        =2005,
    }WindowType;
    Window(Context*,const AttributeSet&);
    Window(int x,int y,int w,int h,int type=TYPE_APPLICATION);
    void setRegion(const RefPtr<Region>&region);
    void draw();
    int inflate(const std::string&res);
    static View*inflate(Context*ctx,const std::string&res);
    virtual ~Window();
    virtual void show();
    virtual void hide();
    virtual void setText(const std::string&);
    const std::string getText()const;
    virtual View& setPos(int x,int y)override;
    virtual View& setSize(int cx,int cy)override;
    virtual bool onKeyUp(int keyCode,KeyEvent& evt) override;
    virtual bool onKeyDown(int keyCode,KeyEvent& evt) override;
    virtual void onBackPressed();
    virtual void onActive();
    virtual void onDeactive();
    bool dispatchKeyEvent(KeyEvent&event)override;

    /*virtual void sendMessage(DWORD msgid,DWORD wParam,ULONG lParam,DWORD delayedtime=0){
        sendMessage(this,msgid,wParam,lParam,delayedtime);
    }
    virtual void sendMessage(View*v,DWORD msgid,DWORD wParam,ULONG lParam,DWORD delayedtime=0)override;*/
    void postDelayed(const Runnable& what,uint32_t delay)override;
    void removeCallbacks(const Runnable& what)override;
    void requestLayout()override;
    static void broadcast(DWORD msgid,DWORD wParam,ULONG lParam);
    void close();
protected:
};

}  // namespace cdroid

#endif  // UI_LIBUI_WINDOW_H_
