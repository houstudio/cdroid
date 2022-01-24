#ifndef __CDROID_DIALOG_H__
#define __CDROID_DIALOG_H__
#include <app/dialoginterface.h>
#include <widget/cdwindow.h>
namespace cdroid{

class Dialog:public DialogInterface{
private:
    Context*mContext;
    Window*mWindow;
    bool mCreated;
    bool mShowing;
    bool mCanceled;
protected:
    void dispatchOnCreate(void*buddle);
    virtual void onCreate();
    virtual void onStart();
    virtual void onStop();
public:
    Dialog(Context*context);
    Context*getContext()const;
    bool isShowing()const;
    void create();
    void show();
    void hide();
    void dismiss()override;
    void dismissDialog(); 
    Window*getWindow()const;
    View*getCurrentFocus(); 
    View*findViewById(int id);
    void setContentView(const std::string&resid);
    void setContentView(View*view);
    void setTitle(const std::string&);


    void cancel()override;  
};
}//endof namespace
#endif//__CDROID_DIALOG_H__
