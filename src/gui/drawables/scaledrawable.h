#ifndef __SCALE_DRAWABLE_H__
#define __SCALE_DRAWABLE_H__
#include <drawables/drawablewrapper.h>
namespace cdroid{

class ScaleDrawable:public DrawableWrapper{
private:
    class ScaleState:public DrawableWrapperState{
    public:
        int mScaleWidth;
        int mScaleHeight;
        int mGravity;
        int mInitialLevel;
        bool mUseIntrinsicSizeAsMin;
        ScaleState();
        ScaleState(const ScaleState& orig);
        Drawable* newDrawable()override;
    };
    std::shared_ptr<ScaleState>mState;
    ScaleDrawable(std::shared_ptr<ScaleState> state);
protected:
    void onBoundsChange(const Rect& bounds)override;
    bool onLevelChange(int level)override;
    std::shared_ptr<DrawableWrapperState> mutateConstantState()override;
public:
    ScaleDrawable(Drawable* drawable, int gravity,int scaleWidth,int scaleHeight);
    std::shared_ptr<ConstantState>getConstantState()override;
    void draw(Canvas& canvas);
    static Drawable*inflate(Context*ctx,const AttributeSet&atts);
};

}
#endif

