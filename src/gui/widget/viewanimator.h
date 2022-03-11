#ifndef __VIEW_ANIMATOR_H__
#define __VIEW_ANIMATOR_H__
#include <widget/framelayout.h>
namespace cdroid{

class ViewAnimator:public FrameLayout{
protected:
    int mWhichChild = 0;
    bool mFirstTime = true;
    bool mAnimateFirstTime = true;
    Animation  *mInAnimation;
    Animation  *mOutAnimation;
private:
    void initViewAnimator(Context* context,const AttributeSet& attrs);
protected:
    void showOnly(int childIndex);
    void showOnly(int childIndex, bool animate);
public:
    ViewAnimator(int w,int h);
    ViewAnimator(Context* context,const AttributeSet& attrs);
    ~ViewAnimator();
    void setDisplayedChild(int whichChild);
    int getDisplayedChild()const;
    void showNext();
    void showPrevious();
    View& addView(View* child, int index,ViewGroup::LayoutParams* params)override;
    void removeAllViews()override;
    void removeView(View* view)override;
    void removeViewAt(int index)override;
    void removeViewInLayout(View* view) override;
    void removeViews(int start, int count) override;
    void removeViewsInLayout(int start, int count) override;
    View* getCurrentView();
    Animation* getInAnimation();
    void setInAnimation(Animation* inAnimation);
    Animation* getOutAnimation();
    void setOutAnimation(Animation* outAnimation);
    bool getAnimateFirstView()const;
    void setAnimateFirstView(bool animate);
    int getBaseline()override;
};
}//namespace
#endif
