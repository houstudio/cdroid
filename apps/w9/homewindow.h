#pragma once
#include <widget/cdwindow.h>
class HomeWindow:public Window{
private:
   void onButtonClick(View&);
   void onWashOptionClick(View&);
public:
   HomeWindow(int mode);
};
