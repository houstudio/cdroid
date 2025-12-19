#ifndef __HIDE_BOTTOMVIEW_ONSCROLL_DELEGATE_H__
#define __HIDE_BOTTOMVIEW_ONSCROLL_DELEGATE_H__

#include <widgetEx/coordinatorlayout/hideviewonscrolldelegate.h>

namespace cdroid{
class HideBottomViewOnScrollDelegate:public HideViewOnScrollDelegate {
public:
    HideBottomViewOnScrollDelegate() {}

    int getViewEdge() override{
        return HideViewOnScrollBehavior::EDGE_BOTTOM;
    }

    int getSize(View& child, MarginLayoutParams* params) override{
        return child.getMeasuredHeight() + params->bottomMargin;
    }

    void setAdditionalHiddenOffset(View& child, int size, int additionalHiddenOffset) override{
        child.setTranslationY(size + additionalHiddenOffset);
    }

    int getTargetTranslation() override{
        return 0;
    }

    void setViewTranslation(View& child, int targetTranslation) override{
        child.setTranslationY(targetTranslation);
    }

    ViewPropertyAnimator* getViewTranslationAnimator(View& child, int targetTranslation) override{
        return &child.animate().translationY(targetTranslation);
    }
};
}/*endorf namespace*/
#endif/*__HIDE_BOTTOMVIEW_ONSCROLL_DELEGATE_H__*/
