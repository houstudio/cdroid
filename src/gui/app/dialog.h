#ifndef __CDROID_DIALOG_H__
#define __CDROID_DIALOG_H__
#include <app/dialoginterface.h>
#include <widget/cdwindow.h>
namespace cdroid{

class Dialog:public DialogInterface,KeyEvent::Callback{
private:
    Context*mContext;
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
    Dialog(Context*context);
    Dialog(Context* context,const std::string&resId);
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
    void setContentView(const std::string&resid);
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
