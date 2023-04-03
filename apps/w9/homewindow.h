#pragma once
#include <widget/cdwindow.h>
class HomeWindow:public Window{
private:
    int mCurScrollX;
    int mOldScrollX;
    int screenWidth;
    ValueAnimator*anim;
protected:
   void onButtonClick(View&);
   void onWashOptionClick(View&);
public:
   HomeWindow();
   ~HomeWindow();
};
