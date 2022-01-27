#include <app/dialog.h>
namespace cdroid{

Dialog::Dialog(Context*context){
    mContext = context;
    mCreated = false;
    mShowing = false;
    mWindow  = nullptr;
    mCancelable = true;
}

Dialog::Dialog(Context* context,const std::string&resId):Dialog(context){
    mWindow=new Window(0,0,640,320);
    View*v=LayoutInflater::from(mWindow->getContext())->inflate(resId,nullptr,false);
    mWindow->addView(v); 
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
    dismissDialog();   
}

void Dialog::dismissDialog(){
    onStop();
    mShowing = false;
    if(mOnDismissListener)
        mOnDismissListener(*this);
}

void Dialog::dispatchOnCreate(void*buddle){
    if (!mCreated) {
        onCreate();
        mCreated = true;
    }
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
    if (!mCanceled && mOnCancelListener) {
        mCanceled = true;
        // Obtain a new message so this dialog can be re-used
        //Message.obtain(mCancelMessage).sendToTarget();
        mOnCancelListener(*this);
    }
    dismiss();
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

void Dialog::setOnKeyListener(OnKeyListener listener){
    mOnKeyListener = listener;
}

Window* Dialog::getWindow()const{
    return mWindow;
}

View* Dialog::getCurrentFocus(){
    return nullptr;
}

View* Dialog::findViewById(int id){
    return mWindow->findViewById(id);
}

void Dialog::setContentView(const std::string&resid){
    View*v=LayoutInflater::from(mWindow->getContext())->inflate(resid,nullptr,false);
    mWindow->addView(v);
}

void Dialog::setContentView(View*view){
    mWindow->addView(view);
}

void Dialog::addContentView(View* view,ViewGroup::LayoutParams* params){
    mWindow->addView(view);
}

void Dialog::setTitle(const std::string&title){
    mWindow->setText(title);
}

bool Dialog::onKeyDown(int keyCode,KeyEvent& event){
    return false;
}

bool Dialog::onKeyLongPress(int keyCode,KeyEvent& event){
    return false;
}

bool Dialog::onKeyUp(int keyCode,KeyEvent& event){
    if (keyCode == KEY_BACK && event.isTracking() && !event.isCanceled()) {
        onBackPressed();
        return true;
    }
    return false;
}

bool Dialog::onKeyMultiple(int keyCode, int repeatCount,KeyEvent& event){
    return false;
}

void Dialog::onBackPressed(){
    if (mCancelable) {
        cancel();
    }
}

}//endof namespace
