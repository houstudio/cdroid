#include <app/progressdialog.h>
#include <widget/R.h>

namespace cdroid{

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
                    String format = mProgressNumberFormat;
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
        View* view = inflater->inflate(a.getString("horizontalProgressLayout","cdroid:layout/alert_dialog_progress"),nullptr);
        mProgress = (ProgressBar*) view->findViewById(R::id::progress);
        mProgressNumber = (TextView*) view->findViewById(R::id::progress_number);
        mProgressPercent = (TextView*) view->findViewById(R::id::progress_percent);
        setView(view);
    } else {
        View* view = inflater->inflate(a.getString("progressLayout","cdroid:layout/progress_dialog"), nullptr);
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

    /**
     * Gets the maximum allowed progress value. The default value is 100.
     *
     * @return the maximum value
     */
int ProgressDialog::getMax()const {
    if (mProgress) {
        return mProgress->getMax();
    }
    return mMax;
}

    /**
     * Sets the maximum allowed progress value.
     */
void ProgressDialog::setMax(int max) {
    if (mProgress) {
        mProgress->setMax(max);
        onProgressChanged();
    } else {
        mMax = max;
    }
}

    /**
     * Increments the current progress value.
     *
     * @param diff the amount by which the current progress will be incremented,
     * up to {@link #getMax()}
     */
void ProgressDialog::incrementProgressBy(int diff) {
    if (mProgress) {
        mProgress->incrementProgressBy(diff);
        onProgressChanged();
    } else {
        mIncrementBy += diff;
    }
}

    /**
     * Increments the current secondary progress value.
     *
     * @param diff the amount by which the current secondary progress will be incremented,
     * up to {@link #getMax()}
     */
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
