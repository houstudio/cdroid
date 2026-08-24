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
#ifndef __CDROID_DIALOG_H__
#define __CDROID_DIALOG_H__
#include <app/dialoginterface.h>
#include <widget/cdwindow.h>
namespace cdroid{

/* Dismiss-only lifetime (the unified transient-popup contract, shared with
   PopupMenu): allocate, configure, show(), and never delete - the destructor
   is protected on purpose. dismiss() is the only teardown: it tears the window
   down through Window::close's posted-free and is idempotent. The dialog
   shell stays allocated after dismiss (small, one allocation per dialog);
   dismiss is FINAL - showing again after it is not supported. */
class Dialog:public DialogInterface,KeyEvent::Callback{
private:
    Context*mContext;
    bool mOwnsContext;   // true when mContext is a ContextThemeWrapper we new'd
    Window*mWindow;
    bool mCreated;
    bool mShowing;
    bool mCanceled;
    OnShowListener   mOnShowListener;
    OnDismissListener mOnDismissListener;
    OnCancelListener mOnCancelListener;
    OnKeyListener mOnKeyListener;
protected:
    bool mCancelable = true;
    void dispatchOnCreate(void*buddle);
    virtual void onCreate();
    virtual void onStart();
    virtual void onStop();
    virtual ~Dialog();
public:
    // AOSP Dialog(Context) / Dialog(Context, int themeResId, boolean
    // createContextThemeWrapper): when createContextThemeWrapper, themeResId 0
    // resolves ?attr/dialogTheme from the caller's theme and the context is
    // wrapped in a ContextThemeWrapper owned (and freed) by this Dialog.
    Dialog(Context*context);
    Dialog(Context* context,int themeResId,bool createContextThemeWrapper=true);
    Context*getContext()const;
    bool isShowing()const;
    void create();
    virtual void show();
    void hide();
    void dismiss()override;
    void dismissDialog(); 
    Window*getWindow()const;
    View*getCurrentFocus(); 
    View*findViewById(int id);
    void setContentView(int layoutResId);
    void setContentView(View*view);
    void addContentView(View* view,ViewGroup::LayoutParams* params);
    void setTitle(const std::string&);
    bool onKeyDown(int keyCode,KeyEvent& event)override;
    bool onKeyLongPress(int keyCode,KeyEvent& event)override;
    bool onKeyUp(int keyCode,KeyEvent& event)override;
    bool onKeyMultiple(int keyCode, int repeatCount,KeyEvent& event)override;
    void onBackPressed();
    void setCancelable(bool flag);
    void setCanceledOnTouchOutside(bool);
    void cancel()override;  
    void setOnCancelListener(OnCancelListener listener);
    void setOnDismissListener(OnDismissListener listener);
    void setOnShowListener(OnShowListener listener);
    void setOnKeyListener(OnKeyListener onKeyListener);
};
}//endof namespace
#endif//__CDROID_DIALOG_H__
