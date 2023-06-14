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
    static constexpr int LENGTH_SHORT= 2000;
    static constexpr int LENGTH_LONG = 4000;
public:
    Toast(Context*context);
    void show();
    void cancel();
    Toast& setView(View*);
    View*getView()const;
    Toast& setDuration(int duration);
    int getDuration()const;
    Toast& setMargin(int horizontalMargin,int verticalMargin);
    int getHorizontalMargin()const;
    int getVerticalMargin()const;
    Toast& setGravity(int gravity,int xoffset,int yoffset);
    int getGravity()const;
    int getXOffset()const;
    int getYOffset()const;
    Toast& setText(const std::string&);
    static Toast*makeText(Context*,const std::string&text,int duration= LENGTH_SHORT);
};
}//endof namespace

#endif
