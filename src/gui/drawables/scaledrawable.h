#ifndef __SCALE_DRAWABLE_H__
#define __SCALE_DRAWABLE_H__
#include <drawables/drawablewrapper.h>
namespace cdroid{

class ScaleDrawable:public DrawableWrapper{
private:
    class ScaleState:public DrawableWrapperState{
    public:
        float mScaleWidth;
        float mScaleHeight;
        int mGravity;
        int mInitialLevel;
        bool mUseIntrinsicSizeAsMin;
        ScaleState();
        ScaleState(const ScaleState& orig);
        ScaleDrawable* newDrawable()override;
    };
    std::shared_ptr<ScaleState>mState;
    ScaleDrawable(std::shared_ptr<ScaleState> state);
    void updateStateFromTypedArray(const AttributeSet&atts);
protected:
    void onBoundsChange(const Rect& bounds)override;
    bool onLevelChange(int level)override;
    std::shared_ptr<DrawableWrapperState> mutateConstantState()override;
public:
    ScaleDrawable();
    ScaleDrawable(Drawable* drawable, int gravity,float scaleWidth,float scaleHeight);
    std::shared_ptr<ConstantState>getConstantState()override;
    void draw(Canvas& canvas);
    int getOpacity()override;
    int getGravity()const;
    void inflate(XmlPullParser&,const AttributeSet&atts)override;
};

}
#endif

