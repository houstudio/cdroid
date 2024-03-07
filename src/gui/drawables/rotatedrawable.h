#ifndef __ROTATE_DRAWABLE_H__
#define __ROTATE_DRAWABLE_H__
#include <drawables/drawablewrapper.h>
namespace cdroid{

class RotateDrawable:public DrawableWrapper{
private:
    class RotateState:public DrawableWrapperState{
    public:
       float mFromDegrees;
       float mToDegrees;
       float mCurrentDegrees;
       float mPivotX;
       float mPivotY;
       bool mPivotXRel;
       bool mPivotYRel;
       RotateState();
       RotateState(const RotateState& orig);
       RotateDrawable*newDrawable()override;
    };
    std::shared_ptr<RotateState>mState;
    RotateDrawable(std::shared_ptr<RotateState>state);
protected:
    bool onLevelChange(int level)override;
    void onBoundsChange(const Rect& bounds)override;
    std::shared_ptr<DrawableWrapperState> mutateConstantState();
public:
    RotateDrawable(Drawable*d=nullptr);
    float getFromDegrees()const;
    float getToDegrees()const;
    void setFromDegrees(float fromDegrees);
    void setToDegrees(float toDegrees);

    float getPivotX()const;
    float getPivotY()const;
    void setPivotX(float pivotX);
    void setPivotY(float pivotX);

    bool isPivotXRelative()const;
    void setPivotXRelative(bool relative);
    bool isPivotYRelative()const;
    void setPivotYRelative(bool relative);
    std::shared_ptr<ConstantState>getConstantState()override;
    void draw(Canvas& canvas)override;
    static Drawable*inflate(Context*ctx,const AttributeSet&atts);
};

}
#endif

