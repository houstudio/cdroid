#include <drawables/colorfilters.h>
#include <cdlog.h>
namespace cdroid{

int ColorFilter::tintMode2CairoOperator(int tintMode){
    switch(tintMode){
    case NONOP   :/*=-1*/ return CAIRO_OPERATOR_CLEAR;
    case CLEAR   :/*= 0*/ return CAIRO_OPERATOR_CLEAR;
    case SRC     :/*= 1*/ return CAIRO_OPERATOR_SOURCE;
    case DST     :/*= 2*/ return CAIRO_OPERATOR_DEST;
    case SRC_OVER:/*= 3*/ return CAIRO_OPERATOR_OVER;
    case DST_OVER:/*= 4*/ return CAIRO_OPERATOR_DEST_OVER;
    case SRC_IN  :/*= 5*/ return CAIRO_OPERATOR_IN;
    case DST_IN  :/*= 6*/ return CAIRO_OPERATOR_DEST_IN;
    case SRC_OUT :/*= 7*/ return CAIRO_OPERATOR_OUT;
    case DST_OUT :/*= 8*/ return CAIRO_OPERATOR_DEST_OUT;
    case SRC_ATOP:/*= 9*/ return CAIRO_OPERATOR_ATOP;
    case DST_ATOP:/*=10*/ return CAIRO_OPERATOR_DEST_ATOP;
    case XOR     :/*=11*/ return CAIRO_OPERATOR_XOR;
    case ADD     :/*=12*/ return CAIRO_OPERATOR_ADD;
    case MULTIPLY:/*=13*/ return CAIRO_OPERATOR_MULTIPLY;
    case SCREEN  :/*=14*/ return CAIRO_OPERATOR_SCREEN;
    case OVERLAY :/*=15*/ return CAIRO_OPERATOR_OVERLAY;
    case DARKEN  :/*=16*/ return CAIRO_OPERATOR_DARKEN;
    case LIGHTEN :/*=17*/ return CAIRO_OPERATOR_LIGHTEN;
    }
    LOGD("TintMode %d is not support",tintMode);
    return CAIRO_OPERATOR_SOURCE;
}

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
    canvas.set_operator((Cairo::Context::Operator)tintMode2CairoOperator(mMode));//2,5(6,7),8,9
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

