#ifndef __UI_BUTTON_H__
#define __UI_BUTTON_H__
#include<widget/textview.h>

namespace cdroid{
class Button : public TextView{
public:
    Button(int w, int h);
    Button(const std::string& text, int w, int h);
    Button(Context*ctx,const AttributeSet& attrs);
    virtual ~Button();
};
}
#endif
