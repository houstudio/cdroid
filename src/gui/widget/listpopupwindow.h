#ifndef __LISTPOPUP_WINDOW_H__
#define __LISTPOPUP_WINDOW_H__
#include <widget/dropdownlistview.h>
#include <widget/popupwindow.h>
namespace cdroid{

class ListPopupWindow{
public:
    static constexpr int POSITION_PROMPT_ABOVE = 0;
    static constexpr int POSITION_PROMPT_BELOW = 1;
    static constexpr int EXPAND_LIST_TIMEOUT = 250;
private:
    Context*mContext;
    Handler*mHandler;
    ListAdapter* mAdapter;
    DropDownListView*mDropDownList;
    int mDropDownHeight;
    int mDropDownWidth;
    int mDropDownHorizontalOffset;
    int mDropDownVerticalOffset;
    int mDropDownWindowLayoutType;
    int mDropDownGravity;
    int mPromptPosition;
    int mListItemExpandMaximum;
    bool mDropDownAlwaysVisible;
    bool mForceIgnoreOutsideTouch;
    DataSetObserver*mObserver;
    View*mPromptView;
    View*mDropDownAnchorView;
    Drawable*mDropDownListHighlight;
    AdapterView::OnItemClickListener mItemClickListener;
    AdapterView::OnItemSelectedListener mItemSelectedListener;
    ListView::OnScrollListener mScrollListener;
    Rect mEpicenterBounds;
    bool mModal;
    uint8_t mOverlapAnchor;/*0xFF unset 1:true,0:false*/
    PopupWindow*mPopup;
    Runnable mResizePopupRunnable;
    Runnable mShowDropDownRunnable;
    Runnable mHideSelector;
    View::OnTouchListener mTouchInterceptor;
private:
    void initPopupWindow();
    void removePromptView();
    int  buildDropDown();
public:
    ListPopupWindow(Context*context,const AttributeSet&atts);
    virtual ~ListPopupWindow();
    void setAdapter(Adapter*adapter);
    void setPromptPosition(int position);
    int getPromptPosition();
    void setModal(bool modal);
    bool isModal();
    void setForceIgnoreOutsideTouch(bool forceIgnoreOutsideTouch);
    void setDropDownAlwaysVisible(bool dropDownAlwaysVisible);
    bool isDropDownAlwaysVisible();
    void setSoftInputMode(int mode);
    int  getSoftInputMode();
    void setListSelector(Drawable* selector);
    Drawable* getBackground();
    void setBackgroundDrawable(Drawable* d);
    void setAnimationStyle(const std::string&);
    std::string getAnimationStyle();
    View* getAnchorView();
    void setAnchorView(View* anchor);
    int getHorizontalOffset();
    void setHorizontalOffset(int offset);
    int getVerticalOffset();
    void setVerticalOffset(int offset);

    void setEpicenterBounds(Rect bounds);
    void setDropDownGravity(int gravity);
    int getWidth();
    void setWidth(int width);
    void setContentWidth(int width);
    int getHeight();
    void setHeight(int height);
    void setWindowLayoutType(int layoutType);
    void setOnItemClickListener(AdapterView::OnItemClickListener clickListener);
    void setOnItemSelectedListener(AdapterView::OnItemSelectedListener selectedListener);
    void setPromptView( View* prompt);
    void postShow();
    void show();
    void dismiss();
    void setOnDismissListener(PopupWindow::OnDismissListener listener);
    void setInputMethodMode(int mode);
    int getInputMethodMode();
    void setSelection(int position);
    void clearListSelection();
    bool isShowing();
    bool isInputMethodNotNeeded();
    bool performItemClick(int position);
    void* getSelectedItem();
    int getSelectedItemPosition();
    long getSelectedItemId();
    View* getSelectedView();
    ListView* getListView();
    DropDownListView* createDropDownListView(Context* context, bool hijackFocus);
    void setListItemExpandMax(int max);
    bool onKeyDown(int keyCode,KeyEvent& event);
    bool onKeyUp(int keyCode,KeyEvent& event);
    void setOverlapAnchor(bool overlap);
};

}
#endif
