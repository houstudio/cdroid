#pragma once
#include <widget/cdwindow.h>

class WashOptionsWindow:public Window{
private:
   void onOptionClick(View&v);
public:	
   enum WashOptions{
	TEMPRATURE=0,

   };
   WashOptionsWindow(int options);
};

