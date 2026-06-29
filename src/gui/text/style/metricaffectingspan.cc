#include <core/typeface.h>
#include <text/textpaint.h>
#include <text/parcelablespan.h>
#include <text/style/metricaffectingspan.h>
namespace cdroid{

void AbsoluteSizeSpan::updateDrawState(TextPaint& paint) const {
    updateMeasureState(paint);
}

void AbsoluteSizeSpan::updateMeasureState(TextPaint& paint) const {
    float size = static_cast<float>(mSize);
    if (mDip) {
        size *= paint.density;
    }
    paint.setTextSize(size);
}

void RelativeSizeSpan::updateDrawState(TextPaint& paint) const {
    updateMeasureState(paint);
}

void RelativeSizeSpan::updateMeasureState(TextPaint& paint) const {
    paint.setTextSize(paint.getTextSize() * mProportion);
}


/*class ReplacementSpan : public MetricAffectingSpan {
public:
    virtual int  getSize(const Paint& paint,const CharSequence* text,int start, int end, Paint::FontMetricsInt* fm)const{return 0;}
    virtual void draw(Canvas& canvas,const CharSequence* text, int start, int end, float x, int top, int y, int bottom,const Paint& paint)const=0;
};*/

//class SubscriptSpan :public MetricAffectingSpan {
//SubscriptSpan() { }
/*int getSpanTypeId() {
    return getSpanTypeIdInternal();
}
int getSpanTypeIdInternal() {
    return TextUtils.SUBSCRIPT_SPAN;
}*/
void SubscriptSpan::updateDrawState(TextPaint& textPaint)const {
    textPaint.baselineShift -= (int) (textPaint.ascent() / 2);
}
void SubscriptSpan::updateMeasureState(TextPaint& textPaint)const {
    textPaint.baselineShift -= (int) (textPaint.ascent() / 2);
}


//class SuperscriptSpan :public MetricAffectingSpan {
/*int getSpanTypeId() {
    return getSpanTypeIdInternal();
}
int getSpanTypeIdInternal() {
    return TextUtils.SUPERSCRIPT_SPAN;
}*/
void SuperscriptSpan::updateDrawState(TextPaint& textPaint)const {
    textPaint.baselineShift += (int) (textPaint.ascent() / 2);
}
void SuperscriptSpan::updateMeasureState(TextPaint& textPaint)const {
    textPaint.baselineShift += (int) (textPaint.ascent() / 2);
}

//class StyleSpan:public MetricAffectingSpan{
void StyleSpan::apply(Paint& paint, int style, int fontWeightAdjustment){
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

void StyleSpan::updateDrawState(TextPaint& paint)const{
    apply(paint, mStyle, mFontWeightAdjustment);
}

void StyleSpan::updateMeasureState(TextPaint& paint) const {
    apply(paint, mStyle, mFontWeightAdjustment);
}

//class TypefaceSpan : public MetricAffectingSpan {
void TypefaceSpan::updateDrawState(TextPaint& paint) const {
    updateMeasureState(paint);
}

void TypefaceSpan::updateMeasureState(TextPaint& paint) const {
    if (!mFamily.empty()) {
        Typeface* old = paint.getTypeface();
        int style = old ? old->getStyle() : Typeface::NORMAL;
        Typeface* tf = Typeface::create(mFamily, style);
        if (tf != nullptr) {
            paint.setTypeface(tf);
        }
    }
}

}/*endof namespace*/
