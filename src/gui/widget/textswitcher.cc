#include <widget/textswitcher.h>
#include <widget/textview.h>

namespace cdroid{

DECLARE_WIDGET(TextSwitcher)

TextSwitcher::TextSwitcher(int w,int h)
    :ViewSwitcher(w,h){
}

TextSwitcher::TextSwitcher(Context*ctx,const AttributeSet&atts)
    :ViewSwitcher(ctx,atts){
}

View& TextSwitcher::addView(View* child, int index, ViewGroup::LayoutParams* params){
    if(dynamic_cast<TextView*>(child)==nullptr)
       throw std::runtime_error("TextSwitcher children must be instances of TextView");
    return ViewSwitcher::addView(child,index,params);
}

void TextSwitcher::setText(const std::string&text){
    TextView* t = (TextView*) getNextView();
    t->setText(text);
    showNext();
}

void TextSwitcher::setCurrentText(const std::string& text){
    ((TextView*)getCurrentView())->setText(text);
}

std::string TextSwitcher::getAccessibilityClassName()const{
    return "TextSwitcher";
}

}

