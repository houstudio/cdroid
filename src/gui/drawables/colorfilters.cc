#include <drawables/colorfilters.h>
#include <cdlog.h>
namespace cdroid{

ColorMatrixColorFilter::ColorMatrixColorFilter(const float(&v)[20]){
    mCM.set(v);
}

void ColorMatrixColorFilter::apply(Canvas&canvas,const Rect&rect){
    Cairo::ImageSurface*img=dynamic_cast<Cairo::ImageSurface*>(canvas.get_target().get());
    uint8_t *data=img->get_data();
    /*RGB和Alpha的终值计算方法如下：
    Red通道终值= a[0] * srcR + a[1] * srcG + a[2] * srcB + a[3] * srcA + a[4]
    Green通道终值= a[5] * srcR + a[6] * srcG + a[7] * srcB + a[8] * srcA + a[9]
    Blue通道终值= a[10] * srcR + a[11] * srcG + a[12] * srcB + a[13] * srcA + a[14]
    Alpha通道终值= a[15]*srcR+a[16]*srcG + a[17] * srcB + a[18] * srcA + a[19]*/
}

PorterDuffColorFilter::PorterDuffColorFilter(int color,int mode){
    mColor=color;
    mMode=mode;
}

void PorterDuffColorFilter::apply(Canvas&canvas,const Rect&rect){
    canvas.set_operator((Cairo::Context::Operator)PorterDuff::toOperator(mMode));//2,5(6,7),8,9
    canvas.set_color(mColor);
    canvas.paint();
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

