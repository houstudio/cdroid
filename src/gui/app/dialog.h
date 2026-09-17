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

/* Owner-managed lifetime: allocate, configure, show(), then dismiss() and
   delete. dismiss() tears the window down through Window::close's posted-free
   and is idempotent; ~Dialog also removes a still-live window itself, so
   deleting an undismissed dialog is safe. dismiss is FINAL - showing again
   after it is not supported (build a new dialog instead). */
class Dialog:public DialogInterface,public WindowCallback,KeyEvent::Callback{
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
    // Window whose dismissal already started (dismissDialog nulls mWindow while
    // close()'s exit animation + posted deletes still own the teardown). Only
    // consulted while App::isQuitting() — at quit the posts are dropped so the
    // object is guaranteed alive; at runtime the post may have freed it and the
    // address reused by an unrelated window.
    Window* mDismissedWindow = nullptr;
    bool mCancelable = true;
    void dispatchOnCreate(void*buddle);
    virtual void onCreate();
    virtual void onStart();
    virtual void onStop();
public:
    // AOSP keeps ~Dialog protected ("use dismiss()") and relies on GC. CDROID
    // has no GC and no delete-this path, so with a protected dtor every dialog
    // leaked by construction. Public dtor: owners dismiss() then delete
    // (subclasses like AlertDialog stay protected-deletable via this base,
    // virtual dispatch still runs the full destructor chain).
    virtual ~Dialog();
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
    // --- WindowCallback graft (AOSP Dialog implements Window.Callback and
    // installs itself via mWindow.setCallback(this); Dialog.java:832-917) ---
    bool dispatchKeyEvent(KeyEvent& event)override;
    bool dispatchKeyShortcutEvent(KeyEvent& event)override;
    bool dispatchTouchEvent(MotionEvent& event)override;
    bool dispatchTrackballEvent(MotionEvent& event)override;
    bool dispatchGenericMotionEvent(MotionEvent& event)override;
    bool dispatchPopulateAccessibilityEvent(AccessibilityEvent& event)override;
    View* onCreatePanelView(int featureId)override;
    bool onCreatePanelMenu(int featureId, Menu& menu)override;
    bool onPreparePanel(int featureId, View* view, Menu& menu)override;
    bool onMenuOpened(int featureId, Menu& menu)override;
    bool onMenuItemSelected(int featureId, MenuItem& item)override;
    void onPanelClosed(int featureId, Menu& menu)override;
    void onWindowDismissed(bool finishTask, bool suppressWindowTransition)override;
    // AOSP Dialog's own (non-Callback) key/motion tails reached from the
    // dispatch chain above.
    bool onKeyShortcut(int keyCode, KeyEvent& event);
    bool onTrackballEvent(MotionEvent& event);
    bool onGenericMotionEvent(MotionEvent& event);
    // AOSP Dialog panel helpers behind onPreparePanel/onCreatePanelMenu.
    bool onCreateOptionsMenu(Menu& menu);
    bool onPrepareOptionsMenu(Menu& menu);
    void onBackPressed();
    void setCancelable(bool flag);
    void setCanceledOnTouchOutside(bool);
    // AOSP Dialog.onTouchEvent (Dialog.java:802-807): outside-touch close
    // consumption, reached through the Window.Callback graft
    // (dispatchTouchEvent -> superDispatchTouchEvent miss -> here).
    virtual bool onTouchEvent(MotionEvent& event);
    void cancel()override;
    void setOnCancelListener(OnCancelListener listener);
    void setOnDismissListener(OnDismissListener listener);
    void setOnShowListener(OnShowListener listener);
    void setOnKeyListener(OnKeyListener onKeyListener);
};
}//endof namespace
#endif//__CDROID_DIALOG_H__
