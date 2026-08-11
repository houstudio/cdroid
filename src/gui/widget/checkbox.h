#ifndef __CHECK_BOX_H__
#define __CHECK_BOX_H__
#include <widget/compoundbutton.h>

namespace cdroid{

class CheckBox:public CompoundButton{
public:
    CheckBox(Context*ctx,const AttributeSet& attrs);
    CheckBox(Context*ctx,const AttributeSet* attrs,int defStyleAttr=0);
    CheckBox(const std::string&txt,int w,int h);
    std::string getAccessibilityClassName()const override;
};

}
#endif
