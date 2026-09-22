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
#include <widget/toast.h>
#include <widget/textview.h>
#include <widget/internal_R.h>
#include <core/app.h>
#include <core/windowmanager.h>

namespace cdroid{
using namespace cdroid::internal;

class ToastWindow:public Window{
private:
    Runnable mTimer;
    Toast* mToast;
public:
    ToastWindow(Toast*t,int x,int y,int w,int h,int duration);
    ~ToastWindow();
};

ToastWindow::ToastWindow(Toast*toast,int x,int y,int w,int h,int duration)
    // Bare-window flavor: AOSP toasts are transparent overlay windows that
    // draw their own backdrop (transient_notification's plate) — no themed
    // windowBackground (the app/theme background would give the toast a solid
    // opaque rectangle). The toast's motion comes from Animation_Toast below.
    :Window(&App::getInstance(), x, y, w, h, Window::TYPE_APPLICATION,
            /*themeWindowAnimations=*/false){
    mToast = toast;
    // AOSP Toast.TN: params.windowAnimations = R.style.Animation_Toast — the
    // toast_enter/toast_exit fades ship in the framework pak. The geometric
    // ctor deliberately loads no theme dressing, so wire the style explicitly:
    // the enter fade rides the compose-time alpha path, the timeout close()
    // plays the themed ghost-exit fade.
    setWindowAnimations((int)internal::R::style::Animation_Toast);
    // AOSP Toast.TN.handleShow: schedule ONE delayed hide for the full
    // duration (postDelayed(mHide, mDuration)). The 100ms-first-hop +
    // 500ms self-reposting poll this replaces woke the looper ~5x/s per
    // toast and delivered close() up to half a second late.
    mTimer = [this](){ close(); };
    postDelayed(mTimer, duration > 0 ? duration : Toast::LENGTH_SHORT);
}

ToastWindow::~ToastWindow(){
    LOGD("Window=%p mToast=%p",this,mToast);
    delete mToast;
}

Toast::Toast(Context*context){
    mContext = context;
    if(context == nullptr)
	    mContext= &App::getInstance();
    mX = mY  = 0;
    mGravity = Gravity::NO_GRAVITY;
    mWindow  = nullptr;
    mVerticalMargin = 0;
    mHorizontalMargin = 0;
}

void Toast::show(){
    // AOSP throws on a toast with no view and re-schedules on a re-show; the
    // no-GC analog: no-op a re-show (a second ToastWindow would double-own
    // mToast — both ~ToastWindow delete it) and refuse a viewless show instead
    // of dereferencing null.
    if (mWindow != nullptr) {
        LOGW("Toast::show: already showing; ignoring re-show");
        return;
    }
    ViewGroup* frame = dynamic_cast<ViewGroup*>(mNextView);
    if (frame == nullptr) {
        LOGE("Toast::show: no view set (call setView/makeText first)");
        return;
    }
    MarginLayoutParams*lp=(MarginLayoutParams*)frame->getLayoutParams();
    const int horzMargin = lp->leftMargin+ lp->rightMargin;
    const int vertMargin = lp->topMargin + lp->bottomMargin;
    Point pt;
    WindowManager::getInstance().getDefaultDisplay().getSize(pt);
    LOGD("size=%dx%d margin=%d,%d",pt.x,pt.y,horzMargin,vertMargin);
    int widthSpec  = MeasureSpec::makeMeasureSpec(pt.x-horzMargin,MeasureSpec::EXACTLY);
    int heightSpec = MeasureSpec::makeMeasureSpec(pt.y-vertMargin,MeasureSpec::AT_MOST);
    LOGD("spec=%x/%x lpsize=%d/%d",widthSpec,heightSpec,lp->width,lp->height);
    widthSpec  = frame->getChildMeasureSpec(widthSpec ,0,lp->width);
    heightSpec = frame->getChildMeasureSpec(heightSpec,0,lp->height);
    frame->measure(widthSpec,heightSpec);
    LOGD("size=%dx%d window=%p duration=%d",frame->getMeasuredWidth(),frame->getMeasuredHeight(),mDuration);
    Rect outRect;
    Rect displayRect = Rect::MakeWH(pt.x,pt.y);
    Gravity::apply(mGravity,frame->getMeasuredWidth(),frame->getMeasuredHeight(),displayRect,outRect);
    ToastWindow*w = new ToastWindow(this,outRect.left+mX,outRect.top+mY,
            frame->getMeasuredWidth(),frame->getMeasuredHeight(),mDuration);
    mWindow = w;
    mWindow->addView(mNextView);
    mWindow->requestLayout();
}

void Toast::cancel(){
    if(mWindow)
        mWindow->close();
    mWindow = nullptr;
}

Toast& Toast::setView(View*view){
    mNextView = view;
    return *this;
}

View*Toast::getView()const{
    return mNextView;
}

Toast& Toast::setDuration(int duration){
    mDuration = duration;
    return *this;
}

int  Toast::getDuration()const{
    return mDuration;
}

Toast& Toast::setMargin(int horizontalMargin,int verticalMargin){
    mHorizontalMargin = horizontalMargin;
    mVerticalMargin = verticalMargin;
    return *this;
}

int  Toast::getHorizontalMargin()const{
    return mHorizontalMargin;
}

int  Toast::getVerticalMargin()const{
    return mVerticalMargin;
}

Toast& Toast::setGravity(int gravity,int xoffset,int yoffset){
    mGravity =gravity;
    mX = xoffset;
    mY = yoffset;
    return *this;
}

int  Toast::getGravity()const{
    return mGravity;
}

int  Toast::getXOffset()const{
    return mX;
}

int  Toast::getYOffset()const{
    return mY;
}

Toast*Toast::makeText(Context*context,const std::string&text,int duration){
    Toast* result = new Toast(context);
    LayoutInflater*inflater=LayoutInflater::from(result->mContext);
    View*v = inflater->inflate(cdroid::internal::R::layout::transient_notification,nullptr);
    TextView*tv = (TextView*)v->findViewById(R::id::message);
    tv->setText(text);
    result->mNextView = v;
    result->mDuration = duration;
    return result;
}

/* Toast.java:529-531 -- setText(@StringRes int) */
Toast& Toast::setText(int resId){
    return setText(mContext->getString(resId));
}

/* Toast.java:520-522 -- makeText(context, resId, duration) */
Toast*Toast::makeText(Context*context,int resId,int duration){
    return makeText(context, context->getString(resId), duration);
}

Toast& Toast::setText(const std::string&text){
    TextView* tv = nullptr;
    if(mNextView){
        tv = (TextView*)mNextView->findViewById(R::id::message);
        if(tv)tv->setText(text);
    }
    LOGE_IF(tv==nullptr,"This Toast was not created by Toast::makeText");
    return *this;
}

}

