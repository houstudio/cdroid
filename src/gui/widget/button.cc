#include <widget/button.h>
#include <gravity.h>
namespace cdroid{

Button::Button(Context*ctx,const AttributeSet& attrs):TextView(ctx,attrs){
    setGravity(Gravity::CENTER_VERTICAL);
    setTextAlignment(TEXT_ALIGNMENT_CENTER);
    setFocusable(true);
}

Button::Button(int32_t w, int32_t h):Button(std::string(),w,h){
}

Button::Button(const std::string& text, int32_t w, int32_t h)
  : TextView(text, w, h){
    setGravity(Gravity::CENTER);//_VERTICAL);
    setTextAlignment(TEXT_ALIGNMENT_CENTER);
    setFocusable(true);
}

Button::~Button() {
}

}//endof namespace

