#include <drawables/colorfilters.h>

namespace cdroid{

ColorMatrixColorFilter::ColorMatrixColorFilter(const float(&v)[20]){
    mCM.set(v);
}

void ColorMatrixColorFilter::apply(Canvas&canvas,const Rect&rect){
}

PorterDuffColorFilter::PorterDuffColorFilter(int color,int mode){
    mColor=color;
    mMode=mode;
}

void PorterDuffColorFilter::apply(Canvas&canvas,const Rect&rect){
    canvas.set_operator((Cairo::Context::Operator)mMode);//2,5(6,7),8,9
    canvas.set_color(mColor);
    canvas.rectangle(rect.left,rect.top,rect.width,rect.height);
    canvas.fill();
}
void PorterDuffColorFilter::setColor(int c){
    mColor=c;
}

int PorterDuffColorFilter::getColor()const{
    return mColor;
}

void PorterDuffColorFilter::setMode(int m){
    mMode=m;
}

int PorterDuffColorFilter::getMode()const{
    return mMode;
}

void LightingColorFilter::apply(Canvas&canvas,const Rect&rect){

}

}

