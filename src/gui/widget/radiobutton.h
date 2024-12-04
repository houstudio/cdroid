#ifndef __RADIO_BUTTON_H__
#define __RADIO_BUTTON_H__
#include <widget/compoundbutton.h>
namespace cdroid{
class RadioButton:public CompoundButton{
public:
    RadioButton(const std::string&,int w,int h);
    RadioButton(Context*ctx,const AttributeSet& attrs);
#ifndef FUNCTION_AS_CHECKABLE
    void toggle()override;
#endif
    std::string getAccessibilityClassName()const override;
};
}/*endof namespace*/
#endif
