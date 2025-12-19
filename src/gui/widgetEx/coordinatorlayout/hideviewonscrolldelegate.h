#ifndef __HIDEVICE_ONSCROLL_DELEGATE_H__
#define __HIDEVICE_ONSCROLL_DELEGATE_H__
#include <view/view.h>
namespace cdroid{

class HideViewOnScrollDelegate {
public:
    virtual ~HideViewOnScrollDelegate()=default;
    /**
     * Returns the edge of the screen from which the view should slide in and out. Must be a {@link
     * com.google.android.material.behavior.HideViewOnScrollBehavior.ViewEdge} value.
     */
    virtual int getViewEdge()=0;

    /**
     * Returns the size of the View. This is based on the height value for the bottom variation, and
     * the width value for right and left variations.
     */
    virtual int getSize(View& child, MarginLayoutParams* params)=0;

    /**
     * Sets the additional offset to add when hiding the view. The offset will be added on the Y axis
     * for the bottom variation, and on the X axis for the right and left variations.
     */
    virtual void setAdditionalHiddenOffset(View& child, int size, int additionalHiddenOffset)=0;

    /** Returns the amount by which the View should be translated. */
    virtual int getTargetTranslation()=0;

    /**
     * Sets the View's translation along the respective axis by the desired target translation amount.
     */
    virtual void setViewTranslation(View& child, int targetTranslation)=0;

    /**
     * Returns an {@link ViewPropertyAnimator} that translates along the respective axis.
     *
     * @param child the View to animate
     * @param targetTranslation the amount by which to translate the View
     */
    virtual ViewPropertyAnimator* getViewTranslationAnimator(View& child, int targetTranslation)=0;
};
}/*endof namespace*/
#endif/*__HIDEVICE_ONSCROLL_DELEGATE_H__*/
