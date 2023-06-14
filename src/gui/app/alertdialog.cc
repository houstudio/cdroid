#include <app/alertdialog.h>
namespace cdroid{

AlertDialog::AlertDialog(Context*ctx):AlertDialog(ctx,false,nullptr){
}

AlertDialog::AlertDialog(Context*ctx,const std::string&resid):Dialog(ctx,resid){
    mAlert = AlertController::create(getContext(), this, getWindow());
    P = nullptr;
}

AlertDialog::AlertDialog(Context*ctx,bool cancelable,DialogInterface::OnCancelListener listener)
   :AlertDialog(ctx,"@cdroid:layout/alert_dialog"){
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

void AlertDialog::setIcon(const std::string&iconId){
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

const std::string AlertDialog::Builder::getString(const std::string&resid)const{
    const std::string text = P->mContext->getString(resid);
    if(text.size())return text;
    return resid;
}

AlertDialog::Builder& AlertDialog::Builder::setTitle(const std::string& title){
    P->mTitle = getString(title);
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setCustomTitle(View* customTitleView){
    P->mCustomTitleView = customTitleView;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setMessage(const std::string&messageId){
    P->mMessage = getString(messageId);
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
    P->mPositiveButtonText = getString(text);
    P->mPositiveButtonListener = listener;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setNegativeButton(const std::string& text, DialogInterface::OnClickListener listener){
    P->mNegativeButtonText = getString(text);
    P->mNegativeButtonListener = listener;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setNeutralButton(const std::string& text, DialogInterface::OnClickListener listener){
    P->mNeutralButtonText = getString(text);
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
    P->mContext->getArray(itemsId,P->mItems);
    P->mOnClickListener = listener;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setItems(const std::vector<std::string>&items, DialogInterface::OnClickListener listener){
    P->mItems =items;
    P->mOnClickListener = listener;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setAdapter(ListAdapter* adapter,DialogInterface::OnClickListener listener){
    P->mAdapter = adapter;
    P->mOnClickListener = listener;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setMultiChoiceItems(const std::string&itemsId,
      const std::vector<bool>& checkedItems,DialogInterface::OnMultiChoiceClickListener listener){
    P->mContext->getArray(itemsId,P->mItems);
    P->mOnCheckboxClickListener = listener;
    P->mCheckedItems = checkedItems;
    P->mIsMultiChoice = true;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setMultiChoiceItems(const std::vector<std::string>&items, 
      const std::vector<bool>&checkedItems, DialogInterface::OnMultiChoiceClickListener listener){
    P->mItems=items;
    P->mOnCheckboxClickListener = listener;
    P->mCheckedItems = checkedItems;
    P->mIsMultiChoice = true;
    return *this;
}

AlertDialog::Builder& AlertDialog::Builder::setSingleChoiceItems(const std::string&itemsId, 
      int checkedItem, DialogInterface::OnClickListener listener){
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
    AlertDialog* dialog = new AlertDialog(P->mContext,"@cdroid:layout/alert_dialog");
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

