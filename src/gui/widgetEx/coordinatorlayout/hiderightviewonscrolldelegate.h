#ifndef __HIDE_RIGHTVIEW_ONSCROLL_DELEGATE_H__
#define __HIDE_RIGHTVIEW_ONSCROLL_DELEGATE_H__
#include <widgetEx/coordinatorlayout/hideviewonscrolldelegate.h>
namespace cdroid{
class HideRightViewOnScrollDelegate:public HideViewOnScrollDelegate {
public:
    HideRightViewOnScrollDelegate() {}

    int getViewEdge() override{
        return HideViewOnScrollBehavior::EDGE_RIGHT;
    }

    int getSize(View& child, MarginLayoutParams* params) override{
        return child.getMeasuredWidth() + params->rightMargin;
    }

    void setAdditionalHiddenOffset(View& child, int size, int additionalHiddenOffset) override{
        child.setTranslationX(size + additionalHiddenOffset);
    }

    int getTargetTranslation() override{
        return 0;
    }

    void setViewTranslation(View& child, int targetTranslation) override{
        child.setTranslationX(targetTranslation);
    }

    ViewPropertyAnimator* getViewTranslationAnimator(View& child, int targetTranslation) override{
        return &child.animate().translationX(targetTranslation);
    }
};
}/*endof namespace*/
#endif/*__HIDE_RIGHTVIEW_ONSCROLL_DELEGATE_H__*/
