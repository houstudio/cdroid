#ifndef __RIPPLE_DRAWABLE_H__
#define __RIPPLE_DRAWABLE_H__
#include <drawables/layerdrawable.h>
namespace cdroid{

class RippleDrawable:public LayerDrawable{
private:
    class RippleState:public LayerDrawable::LayerState{
    public:
        std::vector<int>mTouchThemeAttrs;
        int mMaxRadius;
        ColorStateList*mColor;
        RippleState(LayerState* orig, RippleDrawable* owner);
        void onDensityChanged(int sourceDensity, int targetDensity)override;
        void applyDensityScaling(int sourceDensity, int targetDensity);
        Drawable*newDrawable()override;
        int getChangingConfigurations()const override;
    };
	std::shared_ptr<RippleState>mState;
    RippleDrawable(std::shared_ptr<RippleState> state);
protected:
	int mDensity;
	void ensurePadding();
	void refreshPadding();
	void updateLocalState();
public:

};

}
#endif
