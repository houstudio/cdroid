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

    void updateDrawState(TextPaint& paint) const override {
        updateMeasureState(paint);
    }

    void updateMeasureState(TextPaint& paint) const override {
        float size = static_cast<float>(mSize);
        if (mDip) {
            size *= paint.density;
        }
        paint.setTextSize(size);
    }

private:
    int mSize;
    bool mDip;
};

class RelativeSizeSpan : public MetricAffectingSpan {
public:
    explicit RelativeSizeSpan(float proportion) : mProportion(proportion) {}

    float getSizeChange() const { return mProportion; }

    void updateDrawState(TextPaint& paint) const override {
        updateMeasureState(paint);
    }

    void updateMeasureState(TextPaint& paint) const override {
        paint.setTextSize(paint.getTextSize() * mProportion);
    }

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

class StyleSpan:public MetricAffectingSpan{
private:
    int mStyle;
    int mFontWeightAdjustment;
    static void apply(Paint& paint, int style, int fontWeightAdjustment){
        int oldStyle;
        Typeface* old = paint.getTypeface();
        if (old == nullptr) {
            oldStyle = 0;
        } else {
            oldStyle = old->getStyle();
        }
        int want = oldStyle | style;
        Typeface* tf;
        if (old == nullptr) {
            tf = Typeface::defaultFromStyle(want);
        } else {
            tf = Typeface::create(old, want);
        }
        int fake = want & ~tf->getStyle();
        if ((fake & Typeface::BOLD) != 0) {
            paint.setFakeBoldText(true);
        }
        if ((fake & Typeface::ITALIC) != 0) {
            paint.setTextSkewX(-0.25f);
        }
        paint.setTypeface(tf);
    }
public:
    StyleSpan(int style):StyleSpan(style,0){}
    StyleSpan(int style, int fontWeightAdjustment){}
    int getStyle() const{
        return mStyle;
    }
    int getFontWeightAdjustment() const{
        return mFontWeightAdjustment;
    }
    void updateDrawState(TextPaint& paint)const override{
        apply(paint, mStyle, mFontWeightAdjustment);
    }
    void updateMeasureState(TextPaint& paint) const override {
        apply(paint, mStyle, mFontWeightAdjustment);
    }
};

class TypefaceSpan : public MetricAffectingSpan {
public:
    explicit TypefaceSpan(const std::string& family) : mFamily(family) {}
    const std::string& getFamily() const { return mFamily; }

    void updateDrawState(TextPaint& paint) const override {
        updateMeasureState(paint);
    }

    void updateMeasureState(TextPaint& paint) const override {
        if (!mFamily.empty()) {
            Typeface* old = paint.getTypeface();
            int style = old ? old->getStyle() : Typeface::NORMAL;
            Typeface* tf = Typeface::create(mFamily, style);
            if (tf != nullptr) {
                paint.setTypeface(tf);
            }
        }
    }

private:
    std::string mFamily;
};

}/*endof namespace*/
#endif/*__METRIC_AFFECTING_SPAN_H__*/
