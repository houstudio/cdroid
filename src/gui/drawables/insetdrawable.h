#ifndef __INSET_DRAWABLE_H__
#define __INSET_DRAWABLE_H__
#include <drawables/drawablewrapper.h>

namespace cdroid{

class InsetDrawable:public DrawableWrapper{
private:
    class InsetValue{
    public:
        float mFraction;
        int mDimension;
        int getDimension(int boundSize)const;
        void set(float,int);
        void set(float);
    };
    class InsetState:public DrawableWrapper::DrawableWrapperState{
    private:
        void applyDensityScaling(int sourceDensity, int targetDensity);
    public:
        InsetValue mInsetLeft;
        InsetValue mInsetTop;
        InsetValue mInsetRight;
        InsetValue mInsetBottom;
        Insets mInset;
        InsetState();
        InsetState(const InsetState& orig);
        void onDensityChanged(int sourceDensity, int targetDensity)override;
        InsetDrawable*newDrawable()override;
    };
    std::shared_ptr<InsetState>mState;
    InsetDrawable(std::shared_ptr<InsetState>state);
    void getInsets(Rect& out);
    void updateStateFromTypedArray(const AttributeSet&atts);
protected:
    void onBoundsChange(const Rect&)override;
    std::shared_ptr<DrawableWrapperState> mutateConstantState()override;
public:
    InsetDrawable();
    InsetDrawable(Drawable*drawable,int inset);
    InsetDrawable(Drawable* drawable,int insetLeft,int insetTop,int insetRight,int insetBottom);
    std::shared_ptr<ConstantState>getConstantState()override;
    int getIntrinsicWidth() override;
    int getIntrinsicHeight() override;
    void getOutline(Outline&) override;
    bool getPadding(Rect& padding)override;
    int getOpacity()override;
    Insets getOpticalInsets()override;
    void inflate(XmlPullParser&parser,const AttributeSet&atts)override;
};

}//namespace

#endif
