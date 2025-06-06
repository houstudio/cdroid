#ifndef __TEXTSWITCHER_H__
#define __TEXTSWITCHER_H__
#include <widget/viewswitcher.h>

namespace cdroid{

class TextSwitcher:public ViewSwitcher{
public:
    TextSwitcher(int w,int h);
    TextSwitcher(Context*ctx,const AttributeSet&atts);
    View& addView(View* child, int index, ViewGroup::LayoutParams* params)override;
    void setText(const std::string&);
    void setCurrentText(const std::string& text);
    std::string getAccessibilityClassName()const override;
};

}

#endif
