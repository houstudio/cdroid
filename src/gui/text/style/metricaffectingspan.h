#ifndef __METRIC_AFFECTING_SPAN_H__
#define __METRIC_AFFECTING_SPAN_H__
#include <core/typeface.h>
#include <text/textpaint.h>
#include <text/parcelablespan.h>
namespace cdroid{

class MetricAffectingSpan:public CharacterStyle{
public:
    virtual void updateMeasureState(TextPaint& textPaint)const{};
};

class AbsoluteSizeSpan : public MetricAffectingSpan {
public:
    explicit AbsoluteSizeSpan(int size) : mSize(size), mDip(false) {}
    AbsoluteSizeSpan(int size, bool dip) : mSize(size), mDip(dip) {}

    int getSize() const { return mSize; }
    bool getDip() const { return mDip; }

    void updateDrawState(TextPaint& paint) const override;
    void updateMeasureState(TextPaint& paint) const override;
private:
    int mSize;
    bool mDip;
};

class RelativeSizeSpan : public MetricAffectingSpan {
public:
    explicit RelativeSizeSpan(float proportion) : mProportion(proportion) {}

    float getSizeChange() const { return mProportion; }

    void updateDrawState(TextPaint& paint) const override;
    void updateMeasureState(TextPaint& paint) const override;
private:
    float mProportion;
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
    void updateDrawState(TextPaint& textPaint)const override;
    void updateMeasureState(TextPaint& textPaint)const override;
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
    void updateDrawState(TextPaint& textPaint)const override;
    void updateMeasureState(TextPaint& textPaint)const override;
};

class StyleSpan:public MetricAffectingSpan{
private:
    int mStyle;
    int mFontWeightAdjustment;
    static void apply(Paint& paint, int style, int fontWeightAdjustment);
public:
    StyleSpan(int style):StyleSpan(style,0){}
    StyleSpan(int style, int fontWeightAdjustment){}
    int getStyle() const{
        return mStyle;
    }
    int getFontWeightAdjustment() const{
        return mFontWeightAdjustment;
    }
    void updateDrawState(TextPaint& paint)const override;
    void updateMeasureState(TextPaint& paint) const override;
};

class TypefaceSpan : public MetricAffectingSpan {
public:
    explicit TypefaceSpan(const std::string& family) : mFamily(family) {}
    const std::string& getFamily() const { return mFamily; }

    void updateDrawState(TextPaint& paint) const override;
    void updateMeasureState(TextPaint& paint) const override;
private:
    std::string mFamily;
};

}/*endof namespace*/
#endif/*__METRIC_AFFECTING_SPAN_H__*/
