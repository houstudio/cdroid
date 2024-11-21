#ifndef __TABWIDGET_H__
#define __TABWIDGET_H__
#include <widget/linearlayout.h>

namespace cdroid{

class TabWidget:public LinearLayout{
public:
    DECLARE_UIEVENT(void,OnTabSelectionChanged,int,bool);
private:
    Rect mBounds;
    OnTabSelectionChanged mSelectionChangedListener;
    int mSelectedTab = -1;

    Drawable* mLeftStrip;
    Drawable* mRightStrip;

    bool mDrawBottomStrips = true;
    bool mStripMoved;

    // When positive, the widths and heights of tabs will be imposed so that
    // they fit in parent.
    int mImposedTabsHeight = -1;
    std::vector<int> mImposedTabWidths;
private:
    void initTab();
    void onClickChild(View&v,int idx);
protected:
    void onSizeChanged(int w, int h, int oldw, int oldh)override;
    int getChildDrawingOrder(int childCount, int i)override;
    void measureChildBeforeLayout(View* child, int childIndex, int widthMeasureSpec, int totalWidth,
          int heightMeasureSpec, int totalHeight)override;
    void measureHorizontal(int widthMeasureSpec, int heightMeasureSpec)override;
public:
    TabWidget(int w,int h);
    TabWidget(Context*ctx,const AttributeSet&atts);
    View*getChildTabViewAt(int index);
    int getTabCount()const;
    void setDividerDrawable(Drawable* drawable);
    void setDividerDrawable(const std::string&);
    void setLeftStripDrawable(Drawable* drawable);
    void setLeftStripDrawable(const std::string&);
    Drawable*getLeftStripDrawable();
    void setRightStripDrawable(Drawable* drawable);
    void setRightStripDrawable(const std::string& resId);
    Drawable*getRightStripDrawable();
    void setStripEnabled(bool stripEnabled);
    bool isStripEnabled()const;
    void childDrawableStateChanged(View* child)override;
    void dispatchDraw(Canvas& canvas)override;
    void setCurrentTab(int index);
    void focusCurrentTab(int index);
    View& setEnabled(bool enabled)override;
    View& addView(View* child)override;
    void removeAllViews()override;
    PointerIcon* onResolvePointerIcon(MotionEvent& event, int pointerIndex)override;
    void setTabSelectionListener(OnTabSelectionChanged listener);
    void onFocusChange(View* v, bool hasFocus);
};
    
}//namespace 
#endif
