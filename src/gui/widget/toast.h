#ifndef __TOAST_H__
#define __TOAST_H__
#include <view/view.h>
#include <widget/cdwindow.h>

namespace cdroid{

class Toast{
private:
    int mGravity;
    int mX,mY;
    int mHorizontalMargin;
    int mVerticalMargin;
    Window*mWindow;
protected:
    Context*mContext;
    int mDuration;
    View*mNextView;
public:
    enum{
        LENGTH_SHORT,
        LENGTH_LONG
    };
public:
    Toast(Context*context);
    void show();
    void cancel();
    void setView(View*);
    View*getView()const;
    void setDuration(int duration);
    int getDuration()const;
    void setMargin(int horizontalMargin,int verticalMargin);
    int getHorizontalMargin()const;
    int getVerticalMargin()const;
    void setGravity(int gravity,int xoffset,int yoffset);
    int getGravity()const;
    int getXOffset()const;
    int getYOffset()const;
    static Toast*makeText(Context*,const std::string&text,int duration);
    void setText(const std::string&);
};
}//endof namespace

#endif
