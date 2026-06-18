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
}/*endof namespace*/
#endif/*__METRIC_AFFECTING_SPAN_H__*/