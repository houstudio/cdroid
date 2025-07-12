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
#include <app/progressdialog.h>
#include <widget/R.h>

namespace cdroid{

ProgressDialog::ProgressDialog(cdroid::Context*context):AlertDialog(context){
    mProgressStyle = STYLE_SPINNER;
    mIndeterminate = false;
    mProgress = nullptr;
    mProgressNumber = nullptr;
    mProgressPercent= nullptr;
    mProgressDrawable = nullptr;
    mIndeterminateDrawable = nullptr;
}

ProgressDialog::ProgressDialog(cdroid::Context*context,const std::string& resid):AlertDialog(context,resid){
    mProgressStyle = STYLE_SPINNER;
    mIndeterminate = false;
    mProgress = nullptr;
    mProgressNumber = nullptr;
    mProgressPercent= nullptr;
    mProgressDrawable = nullptr;
    mIndeterminateDrawable = nullptr;
}

ProgressDialog* ProgressDialog::show(Context* context,const std::string&title,const std::string&message,bool indeterminate){
    return show(context,title,message,indeterminate,false);
}

ProgressDialog* ProgressDialog::show(Context* context, const std::string& title,const std::string& message, bool indeterminate, bool cancelable){
    return show(context,title,message,indeterminate,cancelable,nullptr);
}

ProgressDialog* ProgressDialog::show(Context* context,const std::string& title,const std::string& message, bool indeterminate, bool cancelable, OnCancelListener cancelListener){
    ProgressDialog* dialog = new ProgressDialog(context);
    dialog->setTitle(title);
    dialog->setMessage(message);
    dialog->setIndeterminate(indeterminate);
    dialog->setCancelable(cancelable);
    dialog->setOnCancelListener(cancelListener);
    dialog->show();
    return dialog;
}

void ProgressDialog::show(){
    AlertDialog::show();
}

void ProgressDialog::onCreate() {
    LayoutInflater* inflater = LayoutInflater::from(getContext());
    AttributeSet a = getContext()->obtainStyledAttributes("cdroid:attr/alertDialogStyle");
                //com.android.internal.R.styleable.AlertDialog,com.android.internal.R.attr.alertDialogStyle, 0);
    if (mProgressStyle == STYLE_HORIZONTAL) {
          
        /* Use a separate handler to update the text views as they
         * must be updated on the same thread that created them.*/
        /*mViewUpdateHandler = new Handler() {
            public void handleMessage(Message msg) {
                super.handleMessage(msg);
                    
                // Update the number and percent 
                int progress = mProgress.getProgress();
                int max = mProgress.getMax();
                if (mProgressNumberFormat != null) {
                    std::string format = mProgressNumberFormat;
                    mProgressNumber.setText(String.format(format, progress, max));
                } else {
                    mProgressNumber.setText("");
                }
                if (mProgressPercentFormat != null) {
                    double percent = (double) progress / (double) max;
                    SpannableString tmp = new SpannableString(mProgressPercentFormat.format(percent));
                    tmp.setSpan(new StyleSpan(android.graphics.Typeface.BOLD),
                            0, tmp.length(), Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
                    mProgressPercent.setText(tmp);
                } else {
                    mProgressPercent.setText("");
                }
            }
        };*/
        View* view = inflater->inflate(a.getString("horizontalProgressLayout","cdroid:layout/alert_dialog_progress"),nullptr,false);
        mProgress = (ProgressBar*) view->findViewById(R::id::progress);
        mProgressNumber = (TextView*) view->findViewById(R::id::progress_number);
        mProgressPercent = (TextView*) view->findViewById(R::id::progress_percent);
        setView(view);
    } else {
        View* view = inflater->inflate(a.getString("progressLayout","cdroid:layout/progress_dialog"),nullptr,false);
        mProgress = (ProgressBar*) view->findViewById(R::id::progress);
        mMessageView = (TextView*) view->findViewById(R::id::message);
        setView(view);
    }

    if (mMax > 0) {
        setMax(mMax);
    }
    if (mProgressVal > 0) {
        setProgress(mProgressVal);
    }
    if (mSecondaryProgressVal > 0) {
        setSecondaryProgress(mSecondaryProgressVal);
    }
    if (mIncrementBy > 0) {
        incrementProgressBy(mIncrementBy);
    }
    if (mIncrementSecondaryBy > 0) {
        incrementSecondaryProgressBy(mIncrementSecondaryBy);
    }
    if (mProgressDrawable) {
        setProgressDrawable(mProgressDrawable);
    }
    if (mIndeterminateDrawable) {
        setIndeterminateDrawable(mIndeterminateDrawable);
    }
    if (!mMessage.empty()) {
        setMessage(mMessage);
    }
    setIndeterminate(mIndeterminate);
    onProgressChanged();
    AlertDialog::onCreate();
}

void ProgressDialog::onStart() {
    AlertDialog::onStart();
    mHasStarted = true;
}
    
void ProgressDialog::onStop() {
    AlertDialog::onStop();
    mHasStarted = false;
}

void ProgressDialog::setProgress(int value) {
    if (mHasStarted) {
        mProgress->setProgress(value);
        onProgressChanged();
    } else {
        mProgressVal = value;
    }
}

void ProgressDialog::setSecondaryProgress(int secondaryProgress) {
    if (mProgress) {
        mProgress->setSecondaryProgress(secondaryProgress);
        onProgressChanged();
    } else {
        mSecondaryProgressVal = secondaryProgress;
    }
}

int ProgressDialog::getProgress()const{
    if (mProgress) {
        return mProgress->getProgress();
    }
    return mProgressVal;
}

int ProgressDialog::getSecondaryProgress()const{
    if (mProgress) {
        return mProgress->getSecondaryProgress();
    }
    return mSecondaryProgressVal;
}

int ProgressDialog::getMax()const {
    if (mProgress) {
        return mProgress->getMax();
    }
    return mMax;
}

void ProgressDialog::setMax(int max) {
    if (mProgress) {
        mProgress->setMax(max);
        onProgressChanged();
    } else {
        mMax = max;
    }
}

void ProgressDialog::incrementProgressBy(int diff) {
    if (mProgress) {
        mProgress->incrementProgressBy(diff);
        onProgressChanged();
    } else {
        mIncrementBy += diff;
    }
}

void ProgressDialog::incrementSecondaryProgressBy(int diff) {
    if (mProgress) {
        mProgress->incrementSecondaryProgressBy(diff);
        onProgressChanged();
    } else {
        mIncrementSecondaryBy += diff;
    }
}

void ProgressDialog::setProgressDrawable(Drawable* d) {
    if (mProgress) {
        mProgress->setProgressDrawable(d);
    } else {
        mProgressDrawable = d;
    }
}

void ProgressDialog::setIndeterminateDrawable(Drawable* d) {
    if (mProgress) {
        mProgress->setIndeterminateDrawable(d);
    } else {
        mIndeterminateDrawable = d;
    }
}

void ProgressDialog::setIndeterminate(bool indeterminate) {
    if (mProgress) {
        mProgress->setIndeterminate(indeterminate);
    } else {
        mIndeterminate = indeterminate;
    }
}

bool ProgressDialog::isIndeterminate()const {
    if (mProgress) {
        return mProgress->isIndeterminate();
    }
    return mIndeterminate;
}
    
void ProgressDialog::setMessage(const std::string& message) {
    if (mProgress) {
        if (mProgressStyle == STYLE_HORIZONTAL) {
            AlertDialog::setMessage(message);
        } else {
            mMessageView->setText(message);
        }
    } else {
        mMessage = message;
    }
}

void ProgressDialog::setProgressStyle(int style) {
    mProgressStyle = style;
}

void ProgressDialog::onProgressChanged() {
    if (mProgressStyle == STYLE_HORIZONTAL) {
        /*if (mViewUpdateHandler != null && !mViewUpdateHandler.hasMessages(0)) {
            mViewUpdateHandler.sendEmptyMessage(0);
        }*/
    }
}

}//endof namespace
