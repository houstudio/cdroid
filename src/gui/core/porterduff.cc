#include <core/canvas.h>
#include <core/porterduff.h>
#include <porting/cdlog.h>
namespace cdroid{

PorterDuff::Mode PorterDuff::intToMode(int val) {
    switch (val) {
    default:
    case  0: return Mode::CLEAR;
    case  1: return Mode::SRC;
    case  2: return Mode::DST;
    case  3: return Mode::SRC_OVER;
    case  4: return Mode::DST_OVER;
    case  5: return Mode::SRC_IN;
    case  6: return Mode::DST_IN;
    case  7: return Mode::SRC_OUT;
    case  8: return Mode::DST_OUT;
    case  9: return Mode::SRC_ATOP;
    case 10: return Mode::DST_ATOP;
    case 11: return Mode::XOR;
    case 16: return Mode::DARKEN;
    case 17: return Mode::LIGHTEN;
    case 13: return Mode::MULTIPLY;
    case 14: return Mode::SCREEN;
    case 12: return Mode::ADD;
    case 15: return Mode::OVERLAY;
    }
}

int PorterDuff::toOperator(int tintMode){
    switch(tintMode){
    case PorterDuff::NOOP    :/*=-1*/ return CAIRO_OPERATOR_CLEAR;
    case PorterDuff::CLEAR   :/*= 0*/ return CAIRO_OPERATOR_CLEAR;
    case PorterDuff::SRC     :/*= 1*/ return CAIRO_OPERATOR_SOURCE;
    case PorterDuff::DST     :/*= 2*/ return CAIRO_OPERATOR_DEST;
    case PorterDuff::SRC_OVER:/*= 3*/ return CAIRO_OPERATOR_OVER;
    case PorterDuff::DST_OVER:/*= 4*/ return CAIRO_OPERATOR_DEST_OVER;
    case PorterDuff::SRC_IN  :/*= 5*/ return CAIRO_OPERATOR_IN;
    case PorterDuff::DST_IN  :/*= 6*/ return CAIRO_OPERATOR_DEST_IN;
    case PorterDuff::SRC_OUT :/*= 7*/ return CAIRO_OPERATOR_OUT;
    case PorterDuff::DST_OUT :/*= 8*/ return CAIRO_OPERATOR_DEST_OUT;
    case PorterDuff::SRC_ATOP:/*= 9*/ return CAIRO_OPERATOR_ATOP;
    case PorterDuff::DST_ATOP:/*=10*/ return CAIRO_OPERATOR_DEST_ATOP;
    case PorterDuff::XOR     :/*=11*/ return CAIRO_OPERATOR_XOR;
    case PorterDuff::ADD     :/*=12*/ return CAIRO_OPERATOR_ADD;
    case PorterDuff::MULTIPLY:/*=13*/ return CAIRO_OPERATOR_MULTIPLY;
    case PorterDuff::SCREEN  :/*=14*/ return CAIRO_OPERATOR_SCREEN;
    case PorterDuff::OVERLAY :/*=15*/ return CAIRO_OPERATOR_OVERLAY;
    case PorterDuff::DARKEN  :/*=16*/ return CAIRO_OPERATOR_DARKEN;
    case PorterDuff::LIGHTEN :/*=17*/ return CAIRO_OPERATOR_LIGHTEN;
    }
    LOGD("TintMode %d is not support",tintMode);
    return CAIRO_OPERATOR_SOURCE;
}
}/*endof namespace*/
