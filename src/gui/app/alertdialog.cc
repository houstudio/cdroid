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
#include <app/alertdialog.h>
#include <widget/internal_R.h>
namespace cdroid{

AlertDialog::AlertDialog(Context*ctx):AlertDialog(ctx,false,nullptr){
}

AlertDialog::AlertDialog(Context*ctx,int themeResId):Dialog(ctx,themeResId){
    mAlert = AlertController::create(getContext(), this, getWindow());
    P = nullptr;
}

AlertDialog::AlertDialog(Context*ctx,bool cancelable,DialogInterface::OnCancelListener listener)
   :AlertDialog(ctx, cdroid::internal::R::layout::alert_dialog){
    setCancelable(cancelable);
    setOnCancelListener(listener);
}

AlertDialog::~AlertDialog(){
    delete mAlert;
    delete P;
}

void AlertDialog::setTitle(const std::string& title){
    Dialog::setTitle(title);
    mAlert->setTitle(title);
}

void AlertDialog::setCustomTitle(View*customTitleView){
    mAlert->setCustomTitle(customTitleView);
}

void AlertDialog::setMessage(const std::string& message){
    mAlert->setMessage(message);
}

void AlertDialog::setView(View* view){
    mAlert->setView(view);
}

void AlertDialog::setView(View* view, int viewSpacingLeft, int viewSpacingTop, int viewSpacingRight,
            int viewSpacingBottom){
    mAlert->setView(view,viewSpacingLeft,viewSpacingTop,viewSpacingRight,viewSpacingBottom);
}

void AlertDialog::setButton(int whichButton,const std::string&text, OnClickListener listener) {
    mAlert->setButton(whichButton, text, listener);
}

Button* AlertDialog::getButton(int whichButton) {
    return mAlert->getButton(whichButton);
}

ListView* AlertDialog::getListView() {
    return mAlert->getListView();
}

void AlertDialog::setIcon(int iconId){
    mAlert->setIcon(iconId);
}

void AlertDialog::setIcon(Drawable*icon){
    mAlert->setIcon(icon);
}

void AlertDialog::setInverseBackgroundForced(bool forceInverseBackground){
    mAlert->setInverseBackgroundForced(forceInverseBackground);
}

void AlertDialog::onCreate(){
    Dialog::onCreate();
    mAlert->installContent();
}

bool AlertDialog::onKeyDown(int keyCode, KeyEvent& event){
    if (mAlert->onKeyUp(keyCode, event)) return true;
    return Dialog::onKeyUp(keyCode, event);
}

bool AlertDialog::onKeyUp(int keyCode, KeyEvent& event){
    if (mAlert->onKeyUp(keyCode, event)) return true;
    return Dialog::onKeyUp(keyCode, event);
}

////////////////////////////////////////////////////////////////////////////////////////////////

AlertDialog::Builder::Builder(Context* context){
    P = new  AlertController::AlertParams(context);
}

AlertDialog::Builder::~Builder(){
    //delete P;
}

Context* AlertDialog::Builder::getContext(){
    return P->mContext;
}

AlertDialog::Builder& AlertDialog::Builder::setTitle(const std::string& title){
    // AOSP CharSequence form — the literal text, no resource resolution.
    P->mTitle = title;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setTitle(int titleId){
    P->mTitle = P->mContext->getString(titleId);
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setCustomTitle(View* customTitleView){
    P->mCustomTitleView = customTitleView;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setMessage(const std::string& message){
    // AOSP CharSequence form.
    P->mMessage = message;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setMessage(int messageId){
    P->mMessage = P->mContext->getString(messageId);
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setIcon(int iconId){
    P->mIconId = iconId;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setIcon(Drawable*icon){
    P->mIcon =icon;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setPositiveButton(const std::string& text, DialogInterface::OnClickListener listener){
    // AOSP CharSequence form.
    P->mPositiveButtonText = text;
    P->mPositiveButtonListener = listener;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setPositiveButton(int textId, DialogInterface::OnClickListener listener){
    P->mPositiveButtonText = P->mContext->getString(textId);
    P->mPositiveButtonListener = listener;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setNegativeButton(const std::string& text, DialogInterface::OnClickListener listener){
    // AOSP CharSequence form.
    P->mNegativeButtonText = text;
    P->mNegativeButtonListener = listener;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setNegativeButton(int textId, DialogInterface::OnClickListener listener){
    P->mNegativeButtonText = P->mContext->getString(textId);
    P->mNegativeButtonListener = listener;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setNeutralButton(const std::string& text, DialogInterface::OnClickListener listener){
    // AOSP CharSequence form.
    P->mNeutralButtonText = text;
    P->mNeutralButtonListener = listener;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setNeutralButton(int textId, DialogInterface::OnClickListener listener){
    P->mNeutralButtonText = P->mContext->getString(textId);
    P->mNeutralButtonListener = listener;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setCancelable(bool cancelable){
    P->mCancelable = cancelable;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setOnCancelListener(DialogInterface::OnCancelListener onCancelListener){
    P->mOnCancelListener = onCancelListener;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setOnDismissListener(DialogInterface::OnDismissListener onDismissListener){
    P->mOnDismissListener = onDismissListener;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setOnKeyListener(DialogInterface::OnKeyListener onKeyListener){
    P->mOnKeyListener = onKeyListener;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setItems(int itemsId,OnClickListener listener){
    P->mItems = P->mContext->getResources().getStringArray(itemsId);
    P->mOnClickListener = listener;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setItems(const std::vector<std::string>&items, DialogInterface::OnClickListener listener){
    P->mItems = items;
    P->mOnClickListener = listener;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setAdapter(ListAdapter* adapter,DialogInterface::OnClickListener listener){
    P->mAdapter = adapter;
    P->mOnClickListener = listener;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setMultiChoiceItems(int itemsId,
      const std::vector<bool>& checkedItems,DialogInterface::OnMultiChoiceClickListener listener){
    P->mItems = P->mContext->getResources().getStringArray(itemsId);
    P->mOnCheckboxClickListener = listener;
    P->mCheckedItems = checkedItems;
    P->mIsMultiChoice = true;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setMultiChoiceItems(const std::vector<std::string>&items, 
      const std::vector<bool>&checkedItems, DialogInterface::OnMultiChoiceClickListener listener){
    P->mItems = items;
    P->mOnCheckboxClickListener = listener;
    P->mCheckedItems = checkedItems;
    P->mIsMultiChoice = true;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setSingleChoiceItems(int itemsId,
      int checkedItem, DialogInterface::OnClickListener listener){
    P->mItems = P->mContext->getResources().getStringArray(itemsId);
    P->mOnClickListener = listener;
    P->mCheckedItem = checkedItem;
    P->mIsSingleChoice = true;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setSingleChoiceItems(const std::vector<std::string>&items, int checkedItem,DialogInterface::OnClickListener listener){
    P->mItems = items;
    P->mOnClickListener = listener;
    P->mCheckedItem = checkedItem;
    P->mIsSingleChoice = true;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setSingleChoiceItems(ListAdapter* adapter, int checkedItem,DialogInterface::OnClickListener listener){
    P->mAdapter = adapter;
    P->mOnClickListener = listener;
    P->mCheckedItem = checkedItem;
    P->mIsSingleChoice = true;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setOnItemSelectedListener(AdapterView::OnItemSelectedListener listener){
    P->mOnItemSelectedListener = listener;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setView(int themeResId){
    P->mView = nullptr;
    P->mViewLayoutResId = themeResId;
    P->mViewSpacingSpecified = false;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setView(View* view){
    P->mView = view;
    P->mViewLayoutResId = 0;
    P->mViewSpacingSpecified = false;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setRecycleOnMeasureEnabled(bool enabled){
    P->mRecycleOnMeasure = enabled;
    return *this;
}

AlertDialog* AlertDialog::Builder::create(){
    AlertDialog* dialog = new AlertDialog(P->mContext, cdroid::internal::R::layout::alert_dialog);
    P->apply(dialog->mAlert);
    dialog->setCancelable(P->mCancelable);
    if (P->mCancelable) {
        dialog->setCanceledOnTouchOutside(true);
    }
    dialog->setOnCancelListener(P->mOnCancelListener);
    dialog->setOnDismissListener(P->mOnDismissListener);
    if (P->mOnKeyListener != nullptr) {
        dialog->setOnKeyListener(P->mOnKeyListener);
    }
    dialog->P=P;
    return dialog;
}

AlertDialog* AlertDialog::Builder::show(){
    AlertDialog* dialog = create();
    dialog->show();
    return dialog;
}

}//endof namespace

