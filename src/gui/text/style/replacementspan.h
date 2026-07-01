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
    std::string mContentUri;
public:
    ImageSpan(Drawable* drawable) :DynamicDrawableSpan(ALIGN_BOTTOM) {
        mDrawable = drawable;
        mContext=nullptr;
    }
    ImageSpan(Drawable* drawable,int verticalAlignment):DynamicDrawableSpan(verticalAlignment){
        mDrawable = drawable;
    }
    ImageSpan(Context* context, const std::string& resourceId);
    ImageSpan(Context* context, const std::string&resourceId,int verticalAlignment);
    Drawable* getDrawable()const override;
    std::string getSource()const{
        return mContentUri;
    }
};
}/*endof namespace*/
#endif/*__REPLACEMENT_SPAN_H__*/
