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
protected:
    friend class WindowManager;
    friend class GraphDevice;
    class InvalidateOnAnimationRunnable:public Runnable{
    private:
        bool mPosted;
        Window*mOwner;
        std::vector<View*>mViews;
        void postIfNeededLocked();
    public:
        void setOwner(Window*w);
        void addView(View* view);
        void removeView(View* view);
        void run();
    };
private:
    Runnable layoutRunner;
    bool mInLayout;
    bool mHandingLayoutInLayoutRequest;
    Rect mRectOfFocusedView;
    void doLayout();
    bool performFocusNavigation(KeyEvent& event);
    static View*inflate(Context*ctx,std::istream&stream);
protected:
    std::vector<View*>mLayoutRequesters;
    RefPtr<Region>mWindowRgn;
    int window_type;/*window type*/
    int mLayer;/*surface layer*/
    std::string mText;
    InvalidateOnAnimationRunnable mInvalidateOnAnimationRunnable;
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
    bool isInLayout()const override;
    bool postDelayed(Runnable& what,uint32_t delay)override;
    bool removeCallbacks(const Runnable& what)override;
    void requestLayout()override;
    bool requestLayoutDuringLayout(View*)override;
    void dispatchInvalidateOnAnimation(View* view)override;
    void cancelInvalidate(View* view)override;
    static void broadcast(DWORD msgid,DWORD wParam,ULONG lParam);
    void close();
protected:
};

}  // namespace cdroid

#endif  // UI_LIBUI_WINDOW_H_
