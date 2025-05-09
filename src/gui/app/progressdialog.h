#ifndef __PROGRESS_DIALOG_H__
#define __PROGRESS_DIALOG_H__
#include <app/alertdialog.h>
namespace cdroid{

class ProgressDialog:public AlertDialog{
public:
    static constexpr int STYLE_SPINNER = 0;
    
    /**
     * Creates a ProgressDialog with a horizontal progress bar.
     */
    static constexpr int STYLE_HORIZONTAL = 1;
private:
    
    ProgressBar* mProgress;
    TextView* mMessageView;
    
    int mProgressStyle = STYLE_SPINNER;
    TextView* mProgressNumber;
    std::string mProgressNumberFormat;
    TextView* mProgressPercent;
    //NumberFormat mProgressPercentFormat;
    
    int mMax;
    int mProgressVal;
    int mSecondaryProgressVal;
    int mIncrementBy;
    int mIncrementSecondaryBy;
    Drawable* mProgressDrawable;
    Drawable* mIndeterminateDrawable;
    std::string mMessage;
    bool mIndeterminate;
    
    bool mHasStarted;
private:
    void onProgressChanged();
protected:
    void onCreate()override;
    void onStop()override;
public:
    ProgressDialog(Context* context);
    ProgressDialog(Context* context,const std::string& theme);
    static ProgressDialog* show(Context* context,const std::string&title,const std::string&message,bool indeterminate=false);
    static ProgressDialog* show(Context* context,const std::string&title,const std::string&message,bool indeterminate,bool cancelable);
    static ProgressDialog* show(Context* context,const std::string&title,const std::string&message,bool indeterminate,bool cancelable, OnCancelListener cancelListener);
    void show()override;
    void onStart()override;
    void setProgress(int value);
    void setSecondaryProgress(int secondaryProgress);
    int getProgress()const;
    int getSecondaryProgress()const;
    int getMax()const;
    void setMax(int max);
    void incrementProgressBy(int diff);
    void incrementSecondaryProgressBy(int diff);
    void setProgressDrawable(Drawable* d);
    void setIndeterminateDrawable(Drawable* d);
    void setIndeterminate(bool indeterminate);
    bool isIndeterminate()const;
    void setMessage(const std::string& message)override;
    void setProgressStyle(int style);
};
}
#endif
