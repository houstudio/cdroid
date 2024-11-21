#include<widget/imagebutton.h>
#include<cdlog.h>
#include<app.h>
namespace cdroid{

DECLARE_WIDGET2(ImageButton,"cdroid:attr/imageButtonStyle")

ImageButton::ImageButton(Context*ctx,const AttributeSet& attrs)
  :ImageView(ctx,attrs){
}

ImageButton::ImageButton(int w,int h):ImageView(w,h){
}

PointerIcon* ImageButton::onResolvePointerIcon(MotionEvent& event, int pointerIndex){
    if ((getPointerIcon() == nullptr) && isClickable() && isEnabled()) {
        return PointerIcon::getSystemIcon(getContext(), PointerIcon::TYPE_HAND);
    }
    return ImageView::onResolvePointerIcon(event, pointerIndex);
}

}
