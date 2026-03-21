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
    case PorterDuff::CLEAR   :/*= 0*/ return CAIRO_OPERATOR_CLEAR;    /*0*/
    case PorterDuff::SRC     :/*= 1*/ return CAIRO_OPERATOR_SOURCE;   /*1*/
    case PorterDuff::DST     :/*= 2*/ return CAIRO_OPERATOR_DEST;     /*6*/
    case PorterDuff::SRC_OVER:/*= 3*/ return CAIRO_OPERATOR_OVER;     /*2*/
    case PorterDuff::DST_OVER:/*= 4*/ return CAIRO_OPERATOR_DEST_OVER;/*7*/
    case PorterDuff::SRC_IN  :/*= 5*/ return CAIRO_OPERATOR_IN;       /*3*/
    case PorterDuff::DST_IN  :/*= 6*/ return CAIRO_OPERATOR_DEST_IN;  /*8*/
    case PorterDuff::SRC_OUT :/*= 7*/ return CAIRO_OPERATOR_OUT;      /*4*/
    case PorterDuff::DST_OUT :/*= 8*/ return CAIRO_OPERATOR_DEST_OUT; /*9*/
    case PorterDuff::SRC_ATOP:/*= 9*/ return CAIRO_OPERATOR_ATOP;     /*5*/
    case PorterDuff::DST_ATOP:/*=10*/ return CAIRO_OPERATOR_DEST_ATOP;/*10*/
    case PorterDuff::XOR     :/*=11*/ return CAIRO_OPERATOR_XOR;      /*11*/
    case PorterDuff::ADD     :/*=12*/ return CAIRO_OPERATOR_ADD;      /*12*/
    case PorterDuff::MULTIPLY:/*=13*/ return CAIRO_OPERATOR_MULTIPLY; /*14*/
    case PorterDuff::SCREEN  :/*=14*/ return CAIRO_OPERATOR_SCREEN;   /*15*/
    case PorterDuff::OVERLAY :/*=15*/ return CAIRO_OPERATOR_OVERLAY;  /*16*/
    case PorterDuff::DARKEN  :/*=16*/ return CAIRO_OPERATOR_DARKEN;   /*17*/
    case PorterDuff::LIGHTEN :/*=17*/ return CAIRO_OPERATOR_LIGHTEN;  /*18*/
    }
    LOGD("TintMode %d is not support",tintMode);
    return CAIRO_OPERATOR_SOURCE;
}
}/*endof namespace*/
