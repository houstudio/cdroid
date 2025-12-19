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
#include <widget/R.h>
#include <core/app.h>
#include <core/windowmanager.h>

namespace cdroid{

class ToastWindow:public Window{
private:
    int mDuration;
    int mTimeElapsed;
    Runnable mTimer;
    Toast* mToast;
public:
    ToastWindow(Toast*t,int,int,int ,int);
    ~ToastWindow();
    void timeElapsed();
    void setDuration(int dur);
};

ToastWindow::ToastWindow(Toast*toast,int x,int y,int w,int h):Window(x,y,w,h){
    mDuration = INT_MAX;
    mTimeElapsed = 100;
    mToast = toast;
    mTimer = [this](){timeElapsed();};
    postDelayed(mTimer,100);
}

ToastWindow::~ToastWindow(){
    LOGD("Window=%p mToast=%p",this,mToast);
    delete mToast;
}

void ToastWindow::timeElapsed(){
    if(mTimeElapsed <mDuration){
        postDelayed(mTimer,500);
	    mTimeElapsed += 500;
	    return;
    }
    close();
}

void ToastWindow::setDuration(int dur){
    mDuration = dur;
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
    ViewGroup* frame = dynamic_cast<ViewGroup*>(mNextView);
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
    ToastWindow*w = new ToastWindow(this,outRect.left+mX,outRect.top+mY,frame->getMeasuredWidth(),frame->getMeasuredHeight());
    mWindow = w;
    mWindow->addView(mNextView);
    mWindow->requestLayout();
    w->setDuration(mDuration);
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
    View*v = inflater->inflate("cdroid:layout/transient_notification",nullptr);
    TextView*tv = (TextView*)v->findViewById(cdroid::R::id::message);
    tv->setText(text);
    result->mNextView = v;
    result->mDuration = duration;
    return result;
}

Toast& Toast::setText(const std::string&text){
    TextView* tv = nullptr;
    if(mNextView){
        tv = (TextView*)mNextView->findViewById(cdroid::R::id::message);
        if(tv)tv->setText(text);
    }
    LOGE_IF(tv==nullptr,"This Toast was not created by Toast::makeText");
    return *this;
}

}

