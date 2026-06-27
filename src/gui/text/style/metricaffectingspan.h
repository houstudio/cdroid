#include <text/textpaint.h>
#include <text/parcelablespan.h>
#ifndef __METRIC_AFFECTING_SPAN_H__
#define __METRIC_AFFECTING_SPAN_H__
namespace cdroid{

class MetricAffectingSpan:public CharacterStyle{
public:
    virtual void updateMeasureState(TextPaint& textPaint)const{};
};

class ReplacementSpan : public MetricAffectingSpan {
public:
    virtual int  getSize(const Paint& paint,const CharSequence* text,int start, int end, Paint::FontMetricsInt* fm)const{return 0;}
    virtual void draw(Canvas& canvas,const CharSequence* text, int start, int end, float x, int top, int y, int bottom,const Paint& paint)const=0;
};

class SubscriptSpan :public MetricAffectingSpan {
public:
    SubscriptSpan() { }
    /*int getSpanTypeId() {
        return getSpanTypeIdInternal();
    }
    int getSpanTypeIdInternal() {
        return TextUtils.SUBSCRIPT_SPAN;
    }*/
    void updateDrawState(TextPaint& textPaint)const override {
        textPaint.baselineShift -= (int) (textPaint.ascent() / 2);
    }
    void updateMeasureState(TextPaint& textPaint)const override {
        textPaint.baselineShift -= (int) (textPaint.ascent() / 2);
    }
};

class SuperscriptSpan :public MetricAffectingSpan {
public:
    SuperscriptSpan() { }
    /*int getSpanTypeId() {
        return getSpanTypeIdInternal();
    }
    int getSpanTypeIdInternal() {
        return TextUtils.SUPERSCRIPT_SPAN;
    }*/
    void updateDrawState(TextPaint& textPaint)const override {
        textPaint.baselineShift += (int) (textPaint.ascent() / 2);
    }
    void updateMeasureState(TextPaint& textPaint)const override {
        textPaint.baselineShift += (int) (textPaint.ascent() / 2);
    }
};
}/*endof namespace*/
#endif/*__METRIC_AFFECTING_SPAN_H__*/
