#ifndef __TOAST_WINDOW_H__
#define __TOAST_WINDOW_H__
#include <widget/cdwindow.h>

namespace cdroid{

class ToastWindow:public Window{
enum{
    LENGTH_SHORT=2000,
    LENGTH_LONG=4000
};
DECLARE_UIEVENT(ToastWindow*,OnCreateContentListener);
public:
    ToastWindow(int w,int h);
    ~ToastWindow();
    virtual bool onKeyUp(int,KeyEvent&event)override;
    static ToastWindow*makeWindow(OnCreateContentListener oncreate,UINT timeout=LENGTH_SHORT);
    static ToastWindow*makeText(const std::string&txt,UINT timeout=LENGTH_SHORT);
private:
    const static int TIMER_ID=0x1000;
    static std::vector<Window*>toasts_;
    DWORD timeout_;
    DWORD time_elapsed;
    DISALLOW_COPY_AND_ASSIGN(ToastWindow);
};
typedef ToastWindow  Toast;

}//namespace

#endif
