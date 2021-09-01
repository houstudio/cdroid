#ifndef __INSET_DRAWABLE_H__
#define __INSET_DRAWABLE_H__
#include <drawables/drawablewrapper.h>

namespace cdroid{

class InsetDrawable:public DrawableWrapper{
private:
    class InsetState:public DrawableWrapper::DrawableWrapperState{
    private:
        void applyDensityScaling(int sourceDensity, int targetDensity);
    public:
 	    Rect mInset;
        InsetState();
        InsetState(const InsetState& orig);
        void onDensityChanged(int sourceDensity, int targetDensity)override;
        Drawable*newDrawable()override;
    };
    std::shared_ptr<InsetState>mState;
    InsetDrawable(std::shared_ptr<InsetState>state);
protected:
	void onBoundsChange(const Rect&)override;
    std::shared_ptr<DrawableWrapperState> mutateConstantState()override;
public:
	InsetDrawable();
	InsetDrawable(Drawable*drawable,int inset);
	InsetDrawable(Drawable* drawable,int insetLeft,int insetTop,int insetRight,int insetBottom);
    std::shared_ptr<ConstantState>getConstantState()override;
	static Drawable*inflate(Context*ctx,const AttributeSet&atts);
};

}//namespace

#endif
