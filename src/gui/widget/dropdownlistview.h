#pragma once
#include <widget/listview.h>
#include <widget/autoscrollhelper.h>
namespace cdroid{

class DropDownListView:public ListView{
private:
    bool mListSelectionHidden;
    bool mHijackFocus;
    bool mDrawsInPressedState;
    AbsListViewAutoScroller* mScrollHelper;
    Runnable mResolveHoverRunnable;
    void clearPressedItem();
    void setPressedItem(View* child, int position, float x, float y);
protected:
    void drawableStateChanged()override;
    bool touchModeDrawsInPressedState()override;
    View* obtainView(int position, bool* isScrap)override;
public:
    DropDownListView(Context*,bool hijackfoxus);
    bool shouldShowSelector()override;
    bool onTouchEvent(MotionEvent& ev)override;
    bool onHoverEvent(MotionEvent& ev)override;
    bool onForwardedEvent(MotionEvent& event, int activePointerId);
    void setListSelectionHidden(bool hideListSelection);
    bool isInTouchMode()const override;
    bool hasWindowFocus()const override;
    bool isFocused()const override;
    bool hasFocus()const override;
    virtual void scrollTargetBy(int deltaX, int deltaY)=0;
    virtual bool canTargetScrollHorizontally(int direction)=0;   
    virtual bool canTargetScrollVertically(int direction)=0;
};

}

