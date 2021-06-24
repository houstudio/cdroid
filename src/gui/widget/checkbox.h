#ifndef __CHECK_BOX_H__
#define __CHECK_BOX_H__
#include <widget/compoundbutton.h>

namespace cdroid{
class CheckBox:public CompoundButton{
public:
    CheckBox(Context*ctx,const AttributeSet& attrs):CompoundButton(ctx,attrs){}
    CheckBox(const std::string&txt,int w,int h):CompoundButton(txt,w,h){
        setButtonDrawable("cdroid:drawable/btn_check.xml");
    }
};

}
#endif
