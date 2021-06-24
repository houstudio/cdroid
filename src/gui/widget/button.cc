#include <widget/button.h>
#include <gravity.h>
namespace cdroid{

Button::Button(Context*ctx,const AttributeSet& attrs):TextView(ctx,attrs){
    setGravity(Gravity::CENTER_VERTICAL);
    setTextAlignment(TEXT_ALIGNMENT_CENTER);
    setFocusable(true);
    setFocusableInTouchMode(true);
    setClickable(true);
}

Button::Button(int32_t w, int32_t h):Button(std::string(),w,h){
}

Button::Button(const std::string& text, int32_t w, int32_t h)
  : TextView(text, w, h){
    setGravity(Gravity::CENTER);//_VERTICAL);
    setTextAlignment(TEXT_ALIGNMENT_CENTER);
    setFocusable(true);
    setFocusableInTouchMode(true);
    setClickable(true);
    if(w==0)w=text.size() * mLayout->getFontSize();
    if(h==0)h=mLayout->getFontSize()*5/2;
    setFrame(0, 0, w, h); 
}

Button::~Button() {
}

}//endof namespace

