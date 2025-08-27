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
#ifndef __SPINNER_H__
#define __SPINNER_H__
#include <widget/absspinner.h>
#include <widget/listview.h>
#include <widget/listpopupwindow.h>
#include <widget/forwardinglistener.h>
namespace cdroid{
class Dialog;
class DialogInterface;
class Spinner:public AbsSpinner{
private:
    class SpinnerPopup {
    public:
        virtual ~SpinnerPopup()=default;
        virtual void setAdapter(Adapter* adapter)=0;
        /**Show the popup */
        virtual void show(int textDirection, int textAlignment)=0;
        /* Dismiss the popup */
        virtual void dismiss()=0;
        /* @return true if the popup is showing, false otherwise. */
        virtual bool isShowing()=0;
        /**Set hint text to be displayed to the user. This should providea description
         * of the choice being made. @param hintText Hint text to set.*/
        virtual void setPromptText(const std::string& hintText)=0;
        virtual const std::string getHintText()=0;
        virtual int getVerticalOffset()=0;
        virtual void setVerticalOffset(int px)=0;
        virtual int getHorizontalOffset()=0;
        virtual void setHorizontalOffset(int px)=0;
        virtual void setBackgroundDrawable(Drawable* bg)=0;
        virtual Drawable* getBackground()=0;

        virtual void computeContentWidth()=0;
        virtual void setContentWidth(int width)=0;
        //virtual int measureContentWidth(Adapter*,Drawable*);
    };

    class DialogPopup:public SpinnerPopup{
    private:
        class AlertDialog*mPopup;
        Spinner *mSpinner; 
        Adapter *mListAdapter;
        std::string mPrompt;
        void onClick(DialogInterface& dialog, int which);
    public:
        DialogPopup(Spinner*spinner);
        ~DialogPopup()override;
        void setAdapter(Adapter* adapter)override;
        void show(int textDirection, int textAlignment)override;
        void dismiss()override;
        bool isShowing()override;
        void setPromptText(const std::string& hintText)override;
        const std::string getHintText()override;
        int getVerticalOffset()override;
        void setVerticalOffset(int px)override;
        int getHorizontalOffset()override;
        void setHorizontalOffset(int px)override;
        void setBackgroundDrawable(Drawable* bg)override;
        Drawable* getBackground()override;
        void computeContentWidth()override;
        void setContentWidth(int width)override;
    };

    class DropdownPopup:public ListPopupWindow,public SpinnerPopup{
    private:
        std::string mHintText;
        Spinner *mSpinner;
        Adapter *mAdapter;
        ViewTreeObserver::OnGlobalLayoutListener mLayoutListener;
    public:
        DropdownPopup(Context*context,Spinner*spinner,const std::string&defStyleAttr);
        ~DropdownPopup()override;
        void setAdapter(Adapter* adapter)override;
        void show(int textDirection, int textAlignment)override; 
        void dismiss()override;
        bool isShowing()override;
        void setPromptText(const std::string& hintText)override;
        const std::string getHintText()override;
        int getVerticalOffset()override;
        void setVerticalOffset(int px)override;
        int getHorizontalOffset()override;
        void setHorizontalOffset(int px)override;
        void setBackgroundDrawable(Drawable* bg)override;
        Drawable* getBackground()override;
        void computeContentWidth()override;
        void setContentWidth(int width)override;
        ListView*getListView();
    };
    class SpinnerForwardingListener:public ForwardingListener{
    private:
        DropdownPopup*mDropDown;
    public:
        SpinnerForwardingListener(View*view,DropdownPopup*d);
        ShowableListMenu getPopup()override;
        bool onForwardingStarted()override;
    };
public:
    static constexpr int  MODE_DIALOG   = 0;
    static constexpr int  MODE_DROPDOWN = 1;
private:
    int  mGravity;
    Context*mPopupContext;
    ForwardingListener* mForwardingListener;
    SpinnerAdapter* mTempAdapter;
    bool mDisableChildrenWhenDisabled;
    void setUpChild(View* child, bool addChild);
    View* makeView(int position, bool addChild);
protected:
    friend class DropdownPopup;
    int mDropDownWidth;
    SpinnerPopup *mPopup;
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void onLayout(bool changed, int l, int t, int w, int h)override;
    void layout(int delta, bool animate)override;
    void computeContentWidth();
public:
    Spinner(int w,int h,int mode=0);
    Spinner(Context*ctx,const AttributeSet&atts);
    ~Spinner()override;
    Context* getPopupContext()const;
    int measureContentWidth(Adapter* adapter, Drawable* background);
    void setPopupBackgroundDrawable(Drawable* background);
    void setPopupBackgroundResource(const std::string& resId);
    Drawable* getPopupBackground();
    void setDropDownVerticalOffset(int pixels);
    int getDropDownVerticalOffset();
    void setDropDownHorizontalOffset(int pixels);
    int getDropDownHorizontalOffset();
    void setDropDownWidth(int pixels);
    int getDropDownWidth()const;
    void setEnabled(bool enabled)override;
    void setGravity(int gravity);
    int getGravity()const;
    void setAdapter(Adapter*adapter)override;
    int getBaseline()override;
    void show(int textDirection, int textAlignment);
    bool onTouchEvent(MotionEvent& event)override;
    bool performClick()override;
    void onClick(DialogInterface& dialog, int which);
    void onClick(int which);
    PointerIcon* onResolvePointerIcon(MotionEvent& event, int pointerIndex)override;
};

}//namespace
#endif
