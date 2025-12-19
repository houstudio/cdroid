#ifndef __HIDE_LEFTVIEW_ONSCROLL_DELEGATE_H__
#define __HIDE_LEFTVIEW_ONSCROLL_DELEGATE_H__
#include <widgetEx/coordinatorlayout/hideviewonscrolldelegate.h>
namespace cdroid{
class HideLeftViewOnScrollDelegate:public HideViewOnScrollDelegate {
public:
    HideLeftViewOnScrollDelegate() {}

    int getViewEdge() override{
        return HideViewOnScrollBehavior::EDGE_LEFT;
    }

    int getSize(View& child, MarginLayoutParams* params) override{
        return child.getMeasuredWidth() + params->leftMargin;
    }

    void setAdditionalHiddenOffset(View& child, int size, int additionalHiddenOffset) override{
        child.setTranslationX(size - additionalHiddenOffset);
    }

    int getTargetTranslation() override{
        return 0;
    }

    void setViewTranslation(View& child, int targetTranslation) override{
        child.setTranslationX(-targetTranslation);
    }

    ViewPropertyAnimator* getViewTranslationAnimator(View& child, int targetTranslation) override{
        return &child.animate().translationX(-targetTranslation);
    }
};
}/*endof namespace*/
#endif/*__HIDE_LEFTVIEW_ONSCROLL_DELEGATE_H__*/
