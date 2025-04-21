#ifndef __CLIP_DRAWABLE_H__
#define __CLIP_DRAWABLE_H__
#include <drawables/drawablewrapper.h>
namespace cdroid{

class ClipDrawable:public DrawableWrapper{
public:
    static constexpr int HORIZONTAL = 1;
    static constexpr int VERTICAL = 2;
private:
    class ClipState:public DrawableWrapperState{
    public:
        int mGravity;
        int mOrientation;
        ClipState();
        ClipState(const ClipState& state);
        ClipDrawable*newDrawable()override;
    };
    std::shared_ptr<ClipState>mState;
    ClipDrawable(std::shared_ptr<ClipState>state);
    void updateStateFromTypedArray(const AttributeSet&atts);
protected:
    bool onLevelChange(int level)override;
    std::shared_ptr<DrawableWrapperState> mutateConstantState()override;
public:
    ClipDrawable();
    ClipDrawable(Drawable* drawable, int gravity,int orientation);
    int getOpacity()override;
    int getGravity()const;
    int getOrientation()const;
    void draw(Canvas& canvas)override;
    void inflate(XmlPullParser&,const AttributeSet&)override;
};

}
#endif
