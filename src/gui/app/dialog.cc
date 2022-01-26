#include <app/dialog.h>
namespace cdroid{

Dialog::Dialog(Context*context){
    mContext= context;
    mWindow = nullptr;
}

Dialog::Dialog(Context* context,const std::string&resId):Dialog(context){
}

Context*Dialog::getContext()const{
    return mContext;
}

bool Dialog::isShowing()const{
    return mShowing;
}

void Dialog::create(){
    if(!mCreated)dispatchOnCreate(nullptr);
}

void Dialog::show(){
    if(mShowing){
        return;
    }
    mCanceled = false;
    if(!mCreated) dispatchOnCreate(nullptr);
    onStart();
    mShowing = true;
}

void Dialog::hide(){
}

void Dialog::dismiss(){
   
}

void Dialog::dismissDialog(){
}

void Dialog::dispatchOnCreate(void*buddle){
}

void Dialog::onCreate(){
}
void Dialog::onStart(){
}
void Dialog::onStop(){
}

void Dialog::setCancelable(bool flag){
    mCancelable = flag;
    //updateWindowForCancelable();
}

void Dialog::setCanceledOnTouchOutside(bool cancel) {
    if (cancel && !mCancelable) {
        mCancelable = true;
        //updateWindowForCancelable();
    }
        
    //mWindow.setCloseOnTouchOutside(cancel);
}

void Dialog::cancel(){
}

void Dialog::setOnCancelListener(OnCancelListener listener){
    mOnCancelListener = listener;
}

void Dialog::setOnDismissListener(OnDismissListener listener){
    mOnDismissListener = listener;
}

void Dialog::setOnShowListener(OnShowListener listener){
    mOnShowListener = listener;
}

void Dialog::setOnKeyListener(OnKeyListener onKeyListener){

}

Window*Dialog::getWindow()const{
    return mWindow;
}

View* Dialog::getCurrentFocus(){
    return nullptr;
}

View*Dialog::findViewById(int id){
    return mWindow->findViewById(id);
}

void Dialog::setContentView(const std::string&resid){
}

void Dialog::setContentView(View*view){
    mWindow->addView(view);
}

}//endof namespace
