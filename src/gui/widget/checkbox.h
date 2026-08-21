#ifndef __CHECK_BOX_H__
#define __CHECK_BOX_H__
#include <widget/compoundbutton.h>

namespace cdroid{

class CheckBox:public CompoundButton{
public:
    CheckBox(Context*ctx);
    CheckBox(Context*ctx,const AttributeSet* attrs);
    CheckBox(Context*ctx,const AttributeSet* attrs,int defStyleAttr);
    std::string getAccessibilityClassName()const override;
};

}
#endif
