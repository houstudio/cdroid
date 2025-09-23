#ifndef __WEARABLE_RECYCLERVIEW_H__
#define __WEARABLE_RECYCLERVIEW_H__
#include <widgetEx/wear/scrollmanager.h>
#include <widgetEx/recyclerview/recyclerview.h>
namespace cdroid{
class WearableRecyclerView:public RecyclerView {
private:
    static constexpr int NO_VALUE = INT_MIN;

    ScrollManager* mScrollManager = new ScrollManager();
    bool mCircularScrollingEnabled;
    bool mEdgeItemsCenteringEnabled;
    bool mCenterEdgeItemsWhenThereAreChildren;

    int mOriginalPaddingTop = NO_VALUE;
    int mOriginalPaddingBottom = NO_VALUE;

    ViewTreeObserver::OnPreDrawListener mPaddingPreDrawListener;
private:
    void setupOriginalPadding();
protected:
    void onAttachedToWindow() override;
    void onDetachedFromWindow() override;
public:
    WearableRecyclerView(Context* context, const AttributeSet& attrs);

    void setupCenteredPadding();

    bool onTouchEvent(MotionEvent& event) override;
    void setCircularScrollingGestureEnabled(bool circularScrollingGestureEnabled);

    bool isCircularScrollingGestureEnabled()const;

    void setScrollDegreesPerScreen(float degreesPerScreen);

    float getScrollDegreesPerScreen()const;

    void setBezelFraction(float fraction);

    float getBezelFraction()const;

    void setEdgeItemsCenteringEnabled(bool isEnabled);

    bool isEdgeItemsCenteringEnabled()const;
};
}/*endof namespace*/
#endif/*__WEARABLE_RECYCLERVIEW_H__*/
