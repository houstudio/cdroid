#include <widget/button.h>

namespace cdroid{

DECLARE_WIDGET2(Button,"cdroid:attr/buttonStyle")

Button::Button(Context*ctx,const AttributeSet& attrs):TextView(ctx,attrs){
}

Button::Button(int32_t w, int32_t h):Button(std::string(),w,h){
}

Button::Button(const std::string& text, int32_t w, int32_t h)
  :TextView(text, w, h){
    setGravity(Gravity::CENTER);
}

Button::~Button() {
}

PointerIcon* Button::onResolvePointerIcon(MotionEvent& event, int pointerIndex) {
    if ((getPointerIcon() == nullptr) && isClickable() && isEnabled()) {
        return PointerIcon::getSystemIcon(getContext(), PointerIcon::TYPE_HAND);
    }
    return TextView::onResolvePointerIcon(event, pointerIndex);
}
}//endof namespace

