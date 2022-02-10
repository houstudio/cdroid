#ifndef __RADIO_BUTTON_H__
#define __RADIO_BUTTON_H__
#include <widget/togglebutton.h>
namespace cdroid{
class RadioButton:public ToggleButton{
public:
    RadioButton(int w,int h):ToggleButton(w,h){
    }
    RadioButton(Context*ctx,const AttributeSet& attrs)
	  :ToggleButton(ctx,attrs){
    }
    void toggle()override{
        if (!isChecked()) ToggleButton::toggle();
    }
};

}//namespace

#endif
