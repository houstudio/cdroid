/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
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
    Runnable mResizePopupRunnable;
    Runnable mShowDropDownRunnable;
    Runnable mHideSelector;
    View::OnTouchListener mTouchInterceptor;
protected:
    PopupWindow*mPopup;
private:
    void initPopupWindow();
    void removePromptView();
    int  buildDropDown();
public:
    ListPopupWindow(Context*context,const AttributeSet&atts);
    ListPopupWindow(Context* context,const AttributeSet& attrs, const std::string&defStyleAttr);
    ListPopupWindow(Context* context,const AttributeSet& attrs, const std::string&defStyleAttr, const std::string&defStyleRes);
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
    void setOnItemClickListener(const AdapterView::OnItemClickListener& clickListener);
    void setOnItemSelectedListener(const AdapterView::OnItemSelectedListener& selectedListener);
    void setPromptView( View* prompt);
    void postShow();
    void show();
    void dismiss();
    void setOnDismissListener(const PopupWindow::OnDismissListener& listener);
    void setInputMethodMode(int mode);
    int getInputMethodMode();
    void setSelection(int position);
    void clearListSelection();
    bool isShowing();
    bool isInputMethodNotNeeded()const;
    bool performItemClick(int position);
    void* getSelectedItem();
    int getSelectedItemPosition();
    long getSelectedItemId();
    View* getSelectedView();
    ListView* getListView()const;
    DropDownListView* createDropDownListView(Context* context, bool hijackFocus);
    void setListItemExpandMax(int max);
    bool onKeyDown(int keyCode,KeyEvent& event);
    bool onKeyUp(int keyCode,KeyEvent& event);
    void setOverlapAnchor(bool overlap);
};

}
#endif
