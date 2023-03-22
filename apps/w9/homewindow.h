#pragma once
#include <widget/cdwindow.h>
class HomeWindow:public Window{
private:
    int mCurScrollX;
    int mOldScrollX;
    int screenWidth;
protected:
   void onButtonClick(View&);
   void onWashOptionClick(View&);
public:
   HomeWindow(int mode);
};
