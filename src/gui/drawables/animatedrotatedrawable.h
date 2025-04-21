#ifndef __ANIMATED_ROTATE_DRAWABLE_H__
#define __ANIMATED_ROTATE_DRAWABLE_H__
#include <drawables/drawablewrapper.h>

namespace cdroid{

class AnimatedRotateDrawable:public DrawableWrapper,public Animatable{
private:
    class AnimatedRotateState:public DrawableWrapperState{
    public:
        std::vector<int> mThemeAttrs;
        bool mPivotXRel;
        float mPivotX;
        bool mPivotYRel;
        float mPivotY;
        int mFrameDuration;
        int mFramesCount;
        AnimatedRotateState();
        AnimatedRotateState(const AnimatedRotateState& orig);
        AnimatedRotateDrawable* newDrawable()override;
        int getChangingConfigurations()const override;
    };
    Runnable mNextFrame;
    float mCurrentDegrees;
    float mIncrement;
    bool mRunning;
    void updateLocalState();
    std::shared_ptr<AnimatedRotateState>mState;
    AnimatedRotateDrawable(std::shared_ptr<AnimatedRotateState> state);
    void updateStateFromTypedArray(const AttributeSet&atts);
protected:
    std::shared_ptr<DrawableWrapperState> mutateConstantState()override;
public:
    AnimatedRotateDrawable();
    ~AnimatedRotateDrawable()override;
    float getPivotX()const;
    float getPivotY()const;
    void setPivotX(float pivotX);
    void setPivotY(float pivotX);

    bool isPivotXRelative()const;
    void setPivotXRelative(bool relative);
    bool isPivotYRelative()const;
    void setPivotYRelative(bool relative);

    void setFramesCount(int framesCount);
    void setFramesDuration(int framesDuration);
    bool setVisible(bool visible, bool restart)override;
    void start()override;
    void stop()override;
    bool isRunning()override;
    void nextFrame();
    void draw(Canvas& canvas)override;
    void inflate(XmlPullParser&,const AttributeSet&atts)override;
};

}
#endif
