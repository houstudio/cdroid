#include <widget/toast.h>
#include <widget/textview.h>
#include <widget/R.h>

namespace cdroid{

Toast::Toast(Context*context){
    mContext = context;
    mX = mY = 0;
    mGravity = Gravity::NO_GRAVITY;
    mWindow = nullptr;
}

void Toast::show(){
}

void Toast::cancel(){
}

void Toast::setView(View*view){
    mNextView = view;
}

View*Toast::getView()const{
    return mNextView;
}

void Toast::setDuration(int duration){
    mDuration = duration;
}

int  Toast::getDuration()const{
    return mDuration;
}

void Toast::setMargin(int horizontalMargin,int verticalMargin){
}

int  Toast::getHorizontalMargin()const{
    return mHorizontalMargin;
}

int  Toast::getVerticalMargin()const{
    return mVerticalMargin;
}

void Toast::setGravity(int gravity,int xoffset,int yoffset){
    mGravity =gravity;
    mX = xoffset;
    mY = yoffset;
}

int  Toast::getGravity()const{
    return mGravity;
}

int  Toast::getXOffset()const{
    return mX;
}

int  Toast::getYOffset()const{
    return mY;
}

Toast*Toast::makeText(Context*context,const std::string&text,int duration){
    Toast*result = new Toast(context);
    LayoutInflater*inflater=LayoutInflater::from(context);
    View*v = inflater->inflate("cdroid:layout/transient_notification",nullptr);
    TextView*tv= (TextView*)v->findViewById(cdroid::R::id::message);
    tv->setText(text);
    result->mNextView=v;
    result->mDuration =duration;
    return result;
}

void Toast::setText(const std::string&text){
    TextView*tv=nullptr;
    if(mNextView)
        tv=(TextView*)mNextView->findViewById(cdroid::R::id::message);
    if(tv==nullptr)
        throw("This Toast was not created by Toast::makeText");
   tv->setText(text);
}

}

