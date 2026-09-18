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
#ifndef __ALERT_DIALOG_H__
#define __ALERT_DIALOG_H__
#include <widget/button.h>
#include <widget/listview.h>
#include <app/dialog.h>
#include <app/alertcontroller.h>

namespace cdroid{
	
class AlertDialog :public Dialog{
public:
    static constexpr int LAYOUT_HINT_NONE = 0;
    static constexpr int LAYOUT_HINT_SIDE = 1;
    // @Deprecated AOSP special theme selectors for resolveDialogTheme().
    static constexpr int THEME_TRADITIONAL          = 1;
    static constexpr int THEME_HOLO_DARK            = 2;
    static constexpr int THEME_HOLO_LIGHT           = 3;
    static constexpr int THEME_DEVICE_DEFAULT_DARK  = 4;
    static constexpr int THEME_DEVICE_DEFAULT_LIGHT = 5;
    class Builder{
    private:
        AlertController::AlertParams* P;
    public:
        Builder(Context* context);
        Builder(Context* context,int themeResId);
        ~Builder();
        Context* getContext();
        Builder& setTitle(const std::string& title);
        Builder& setTitle(int titleId);
        Builder& setCustomTitle(View* customTitleView);
        Builder& setMessage(const std::string& message);
        Builder& setMessage(int messageId);
        Builder& setIcon(int iconId);
        Builder& setIcon(Drawable*icon);
        Builder& setIconAttribute(int attrId);
        Builder& setPositiveButton(const std::string& text,OnClickListener listener);
        Builder& setPositiveButton(int textId,OnClickListener listener);
        Builder& setNegativeButton(const std::string& text, OnClickListener listener);
        Builder& setNegativeButton(int textId, OnClickListener listener);
        Builder& setNeutralButton(const std::string& text, OnClickListener listener);
        Builder& setNeutralButton(int textId, OnClickListener listener);
        Builder& setCancelable(bool cancelable);
        Builder& setOnCancelListener(OnCancelListener onCancelListener);
        Builder& setOnDismissListener(OnDismissListener onDismissListener);
        Builder& setOnKeyListener(OnKeyListener onKeyListener);
        Builder& setItems(int itemsId,OnClickListener listener);
        Builder& setItems(const std::vector<std::string>&items, OnClickListener listener);
        Builder& setAdapter(ListAdapter* adapter,OnClickListener listener);
        Builder& setMultiChoiceItems(int itemsId,const std::vector<bool>& checkedItems,
                OnMultiChoiceClickListener listener);
        Builder& setMultiChoiceItems(const std::vector<std::string>&items, const std::vector<bool>& checkedItems,
                OnMultiChoiceClickListener listener);
        Builder& setSingleChoiceItems(int itemsId, int checkedItem, OnClickListener listener);
        Builder& setSingleChoiceItems(const std::vector<std::string>&items, int checkedItem,OnClickListener listener);
        Builder& setSingleChoiceItems(ListAdapter* adapter, int checkedItem,OnClickListener listener);
        Builder& setOnItemSelectedListener(AdapterView::OnItemSelectedListener listener);
        Builder& setView(int themeResId);
        Builder& setView(View* view);
        Builder& setRecycleOnMeasureEnabled(bool enabled);
        AlertDialog* create();
        AlertDialog* show();
    };
protected:
    friend Builder;
    friend class AlertController::AlertParams;
    class AlertController* mAlert;
    AlertController::AlertParams*P;
protected:
    AlertDialog(Context*ctx);
    AlertDialog(Context*ctx,int themeResId,bool createContextThemeWrapper=true);
    AlertDialog(Context*ctx,bool cancelable,OnCancelListener listener);
    // AOSP AlertDialog.resolveDialogTheme: THEME_* selectors map to the
    // framework alert-dialog styles, real ids pass through, 0 resolves
    // ?attr/alertDialogTheme from the context theme.
    static int resolveDialogTheme(Context* context,int themeResId);
    void onCreate()override;
    // Dialog.dismissDialog() calls onStop() synchronously BEFORE posting the
    // window close — the one point where the view tree is unquestionably alive
    // and the controller's list adapter can be unbound, so the later posted
    // detach dispatch finds ListView.mAdapter null even if the dialog object
    // is destroyed before the posted teardown runs (AOSP relies on GC there).
    void onStop()override;
public:
    /* Dialog documents the owner-managed contract (dismiss() then delete,
       public ~Dialog); this override used to sit in the protected section,
       making every owner's delete a compile error while leaks remained. */
    ~AlertDialog() override;
    Button* getButton(int whichButton);
    ListView* getListView();
    void setTitle(const std::string& title);
    void setCustomTitle(View*customTitleView);
    virtual void setMessage(const std::string& message);
    virtual void setView(View* view);
    void setView(View* view, int viewSpacingLeft, int viewSpacingTop, int viewSpacingRight,
            int viewSpacingBottom);
    void setButton(int whichButton,const std::string&text, OnClickListener listener);
    void setIcon(int iconId);
    void setIcon(Drawable*);
    void setIconAttribute(int attrId);
    void setInverseBackgroundForced(bool forceInverseBackground);
    bool onKeyDown(int keyCode, KeyEvent& event)override;
    bool onKeyUp(int keyCode, KeyEvent& event)override;
};

}//namespace 
#endif 
