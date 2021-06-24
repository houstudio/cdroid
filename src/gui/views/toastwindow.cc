#include <toastwindow.h>
#include <windows.h>
#include <cdlog.h>


namespace cdroid{

std::vector<Window*>ToastWindow::toasts_;
 
ToastWindow::ToastWindow(int w,int h):Window(0,0,w,h,TYPE_TOAST){
    timeout_=-1;
    time_elapsed=0;
    setFocusable(false);
    sendMessage((DWORD)WM_TIMER,(DWORD)TIMER_ID,0,500);
    toasts_.push_back(this);
}

ToastWindow::~ToastWindow(){
    toasts_.erase(std::find(toasts_.begin(),toasts_.end(),this));
}

bool ToastWindow::onMessage(DWORD msg,DWORD wp,ULONG lp){
    if(msg==WM_TIMER && wp==TIMER_ID){
         time_elapsed+=500;
         if(time_elapsed>=timeout_){
             sendMessage(WM_DESTROY,0,0);
             time_elapsed=0;
         }
         sendMessage(msg,wp,lp,500);
         return true;
    }
    return Window::onMessage(msg,wp,lp);
}

bool ToastWindow::onKeyUp(int keyCode,KeyEvent&evt){
    time_elapsed=0;
    return Window::onKeyUp(keyCode,evt);
}

ToastWindow*ToastWindow::makeWindow(OnCreateContentListener oncreate,UINT timeout){
    ToastWindow*w=nullptr;
    if(oncreate){
        w=oncreate();
        w->timeout_=timeout;
    }
    return w;
}

ToastWindow*ToastWindow::makeText(const std::string&txt,UINT timeout){
    int sw,sh,tw,th;
    GraphDevice::getInstance().getScreenSize(sw,sh);
    GraphDevice::getInstance().getPrimaryContext()->set_font_size(20);
    GraphDevice::getInstance().getPrimaryContext()->get_text_size(txt,&tw,&th);
    tw+=th*4;th+=th;
    return makeWindow([&]()->ToastWindow*{
        ToastWindow*w=new ToastWindow(tw,th);
        TextView*tv=new TextView(txt,tw,th);
        tv->setTextSize(20);
        tv->setSingleLine(false);
        w->addView(tv);
        w->setPos((sw-tw)/2,(sh-th)/2);
        return w;
    },timeout);
}
}//namespace

