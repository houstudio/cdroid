#include <app/alertdialog.h>
namespace cdroid{

AlertDialog::AlertDialog(Context*ctx):AlertDialog(ctx,false,nullptr){
}

AlertDialog::AlertDialog(Context*ctx,const std::string&resid):Dialog(ctx,resid){
    mAlert = AlertController::create(getContext(), this, getWindow());
}

AlertDialog::AlertDialog(Context*ctx,bool cancelable,DialogInterface::OnCancelListener listener)
   :AlertDialog(ctx,""){
    setCancelable(cancelable);
    setOnCancelListener(listener);
}

////////////////////////////////////////////////////////////////////////////////////////////////

AlertDialog::Builder::Builder(Context* context){
    P = new  AlertController::AlertParams(context);
}

AlertDialog::Builder::~Builder(){
    delete P;
}

Context* AlertDialog::Builder::getContext(){
    return P->mContext;
}

AlertDialog::Builder& AlertDialog::Builder::setTitle(const std::string& title){
    P->mTitle = title;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setCustomTitle(View* customTitleView){
    P->mCustomTitleView = customTitleView;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setMessage(const std::string&messageId){
    P->mMessage = messageId;//P->mContext->getText(messageId);
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setIcon(const std::string&iconId){
    P->mIconId = iconId;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setIcon(Drawable*icon){
    P->mIcon =icon;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setPositiveButton(const std::string& text, DialogInterface::OnClickListener listener){
    P->mPositiveButtonText = text;//P->mContext->getText(textId);
    P->mPositiveButtonListener = listener;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setNegativeButton(const std::string& text, DialogInterface::OnClickListener listener){
    P->mNegativeButtonText = text;
    P->mNegativeButtonListener = listener;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setNeutralButton(const std::string& text, DialogInterface::OnClickListener listener){
    P->mNeutralButtonText = text;
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

AlertDialog::Builder& AlertDialog::Builder::setItems(const std::string& itemsId,OnClickListener listener){
    P->mContext->getArray(itemsId,P->mItems);
    //P->mItems=P->Context.getResources().getTextArray(itemsId);
    P->mOnClickListener = listener;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setItems(const std::vector<std::string>&items, OnClickListener listener){
    P->mItems =items;
    P->mOnClickListener = listener;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setAdapter(ListAdapter* adapter,OnClickListener listener){
    P->mAdapter = adapter;
    P->mOnClickListener = listener;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setMultiChoiceItems(const std::string&itemsId,const std::vector<bool>& checkedItems,
         DialogInterface::OnMultiChoiceClickListener listener){
    //P.mItems = P.mContext.getResources().getTextArray(itemsId);
    P->mContext->getArray(itemsId,P->mItems);
    P->mOnCheckboxClickListener = listener;
    P->mCheckedItems = checkedItems;
    P->mIsMultiChoice = true;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setMultiChoiceItems(const std::vector<std::string>&items, const std::vector<bool>& checkedItems,
         DialogInterface::OnMultiChoiceClickListener listener){
    P->mItems=items;
    P->mOnCheckboxClickListener = listener;
    P->mCheckedItems = checkedItems;
    P->mIsMultiChoice = true;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setSingleChoiceItems(const std::string&itemsId, int checkedItem, DialogInterface::OnClickListener listener){
    //P.mItems = P.mContext.getResources().getTextArray(itemsId);
    P->mContext->getArray(itemsId,P->mItems);
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

AlertDialog::Builder& AlertDialog::Builder::setView(const std::string&layoutResId){
    P->mView = nullptr;
    P->mViewLayoutResId = layoutResId;
    P->mViewSpacingSpecified = false;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setView(View* view){
    P->mView = view;
    P->mViewLayoutResId ="";
    P->mViewSpacingSpecified = false;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setRecycleOnMeasureEnabled(bool enabled){
    P->mRecycleOnMeasure = enabled;
    return *this;
}

AlertDialog* AlertDialog::Builder::create(){
    AlertDialog* dialog = new AlertDialog(P->mContext, P->mViewLayoutResId);//, false);
    P->apply(dialog->mAlert);
    dialog->getWindow()->requestLayout();
    dialog->setCancelable(P->mCancelable);
    if (P->mCancelable) {
        dialog->setCanceledOnTouchOutside(true);
    }
    dialog->setOnCancelListener(P->mOnCancelListener);
    dialog->setOnDismissListener(P->mOnDismissListener);
    if (P->mOnKeyListener != nullptr) {
        dialog->setOnKeyListener(P->mOnKeyListener);
    }
    return dialog;
}

AlertDialog* AlertDialog::Builder::show(){
    AlertDialog* dialog = create();
    dialog->show();
    return dialog;
}

}//endof namespace

