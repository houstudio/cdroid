#ifndef __DROPDOWN_LISTVIEW_H__
#define __DROPDOWN_LISTVIEW_H__
#include <widget/listview.h>
#include <widget/autoscrollhelper.h>
namespace cdroid{

class DropDownListView:public ListView{
private:
    class ResolveHoverRunnable;
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
    DropDownListView(Context*,bool hijackfocus);
    ~DropDownListView()override;
    bool shouldShowSelector()override;
    bool onTouchEvent(MotionEvent& ev)override;
    bool onHoverEvent(MotionEvent& ev)override;
    bool onForwardedEvent(MotionEvent& event, int activePointerId);
    void setListSelectionHidden(bool hideListSelection);
    bool isInTouchMode()const override;
    bool hasWindowFocus()const override;
    bool isFocused()const override;
    bool hasFocus()const override;
};

class DropDownListView::ResolveHoverRunnable{
private:
    DropDownListView*mDLV;
    Runnable mRunnable;
public:
    ResolveHoverRunnable(DropDownListView*v);
    void run();
    void cancel();
    void post();
};
}//endof namespace
#endif
