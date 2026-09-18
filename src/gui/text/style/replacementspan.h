#ifndef __REPLACEMENT_SPAN_H__
#define __REPLACEMENT_SPAN_H__
#include <text/style/metricaffectingspan.h>
namespace cdroid{
class Drawable;
class ReplacementSpan : public MetricAffectingSpan {
public:
    virtual int  getSize(const Paint& paint,const CharSequence* text,int start, int end, Paint::FontMetricsInt* fm)const{return 0;}
    virtual void draw(Canvas& canvas,const CharSequence* text, int start, int end, float x, int top, int y, int bottom,const Paint& paint)const=0;
};

class DynamicDrawableSpan:public ReplacementSpan{
protected:
    int mVerticalAlignment;
    mutable Drawable* mDrawable = nullptr;
public:
    enum AlignmentType{
        ALIGN_BOTTOM = 0,
        ALIGN_BASELINE = 1,
        ALIGN_CENTER = 2
    };
    DynamicDrawableSpan() {
        mVerticalAlignment = ALIGN_BOTTOM;
    }
    DynamicDrawableSpan(int verticalAlignment) {
        mVerticalAlignment = verticalAlignment;
    }
    int getVerticalAlignment()const{
        return mVerticalAlignment;
    }
    virtual Drawable* getDrawable()const=0;
    int  getSize(const Paint& paint, const CharSequence* text, int start, int end, Paint::FontMetricsInt* fm)const override;
    void draw(Canvas& canvas, const CharSequence* text, int start, int end, float x, int top, int y, int bottom, const Paint& paint)const override;
};

class ImageSpan:public DynamicDrawableSpan{
protected:
    Context*mContext = nullptr;
    int mContentUri=0;
    /*AOSP mSource (String): the src/uri the span was built from; empty when
      the span came from a bare Drawable or a resource id (Java null). Carried
      for the Html round-trip — toHtml emits <img src="getSource()">.*/
    std::string mSource;
public:
    ImageSpan(Drawable* drawable) :DynamicDrawableSpan(ALIGN_BOTTOM) {
        mContext = nullptr;
        mDrawable = drawable;
    }
    ImageSpan(Drawable* drawable,int verticalAlignment):DynamicDrawableSpan(verticalAlignment){
        mDrawable = drawable;
    }
    // AOSP ImageSpan(drawable, source[, verticalAlignment]).
    ImageSpan(Drawable* drawable, const std::string& source)
        :ImageSpan(drawable, source, ALIGN_BOTTOM) {}
    ImageSpan(Drawable* drawable, const std::string& source, int verticalAlignment)
        :DynamicDrawableSpan(verticalAlignment), mSource(source) {
        mDrawable = drawable;
    }
    ImageSpan(Context* context, int resourceId);
    ImageSpan(Context* context, int resourceId,int verticalAlignment);
    Drawable* getDrawable()const override;
    const std::string& getSource()const{
        return mSource;
    }
    // mContext/mDrawable are BORROWED (lifetime managed elsewhere; ImageSpan
    // never deletes them), so the implicit copy ctor's shallow pointer copy is
    // correct here — matches Android's drawable-shared-across-copies behavior.
    ImageSpan* clone() const override { return new ImageSpan(*this); }
};
}/*endof namespace*/
#endif/*__REPLACEMENT_SPAN_H__*/
