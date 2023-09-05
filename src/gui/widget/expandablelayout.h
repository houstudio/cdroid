#ifndef __EXPANDABLE_LAYOUT_H__
#define __EXPANDABLE_LAYOUT_H__
#include <widget/framelayout.h>
namespace cdroid{

class ExpandableLayout:public FrameLayout{
private:
    static constexpr int DEFAULT_DURATION = 300;
public:
    static constexpr int COLLAPSED = 0;
    static constexpr int COLLAPSING = 1;
    static constexpr int EXPANDING = 2;
    static constexpr int EXPANDED = 3;

    static constexpr int HORIZONTAL = 0;
    static constexpr int VERTICAL = 1;
    DECLARE_UIEVENT(void,OnExpansionUpdateListener,float expansionFraction, int state);
private:
    int mDuration = DEFAULT_DURATION;
    float mParallax;
    float mExpansion;
    int mOrientation;
    int mState;
    float mTargetExpansion;
    bool mCanceled;

    Interpolator* mInterpolator;// = new FastOutSlowInInterpolator();
    ValueAnimator* mAnimator;
    OnExpansionUpdateListener mListener;
private:
    void initView();
    void animateSize(int targetExpansion);
protected:
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
public:
    ExpandableLayout(int w,int h);
    ExpandableLayout(Context* context,const AttributeSet& attrs);
    ~ExpandableLayout();
    int getState()const;
    bool isExpanded()const;
    void toggle();
    void toggle(bool animate);
    void expand();
    void expand(bool animate);
    void collapse();
    void collapse(bool animate);
    void setExpanded(bool expand);
    void setExpanded(bool expand, bool animate);
    int getDuration()const;
    void setDuration(int duration);
    void setInterpolator(Interpolator* interpolator);
    float getExpansion()const;
    void setExpansion(float expansion);
    float getParallax()const;
    void setParallax(float parallax);
    int getOrientation()const;
    void setOrientation(int orientation);
    void setOnExpansionUpdateListener(OnExpansionUpdateListener listener);
};
}/*endof namespace*/
#endif
